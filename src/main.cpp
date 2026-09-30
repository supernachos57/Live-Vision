#include <torch/torch.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


struct ImageRecord
{
    std::string path;
    int64_t label;
};

int64_t mapCategory(int64_t datasetCategory)
{
    if (datasetCategory == 1)
        return 0;
    else if (datasetCategory == 2)
        return 1;
    else if (datasetCategory == 4)
        return 2;
    return -1; // Default case if no mapping is found
};

struct GroceryClassifier : torch::nn::Module
{
    torch::nn::Linear classifier{nullptr};

    GroceryClassifier(int64_t inputFeatures, int64_t classCount)
    {
        classifier = register_module("classifier", torch::nn::Linear(inputFeatures, classCount));
    }

    torch::Tensor forward(torch::Tensor inputs)
    {
        return classifier->forward(inputs);
    }
};

struct GroceryCNN : torch::nn::Module
{
    torch::nn::Conv2d convolution {nullptr};
    torch::nn::Linear classifier {nullptr};
    torch::nn::MaxPool2d pooling {nullptr};

    GroceryCNN()
    {
        convolution = register_module("convolution", torch::nn::Conv2d(torch::nn::Conv2dOptions(3, 8, 3).stride(1).padding(1)));
        pooling = register_module("pooling", torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)));
        classifier = register_module("classifier", torch::nn::Linear(8 * 32 * 32, 3)); // Assuming 3 classes and input image size 64x64

    }

    torch::Tensor forward(torch::Tensor inputs)
    {
        torch::Tensor features = convolution->forward(inputs);
        torch::Tensor activated = torch::relu(features); // Apply ReLU activation
        torch::Tensor pooled = pooling->forward(activated);
        torch::Tensor flattened = pooled.flatten(1);

        return classifier->forward(flattened);
    }

};

int64_t parseCategoryId(const std::string& text)
{
    std::istringstream input(text);
    int64_t value = 0;

    if (!(input >> value ))
        throw std::runtime_error("Invalid category ID: " + text);

    input >> std::ws;

    if (!input.eof() || value < 0)
        throw std::runtime_error("Invalid category ID: " + text);

    return value;
}

std::vector<ImageRecord> readImageRecords(const std::string& listPath)
{

    std::ifstream imageList(listPath);
    if (!imageList.is_open())
        throw std::runtime_error("Failed to open image list file.");
    std::vector<ImageRecord> records;

    std::string line;
    while (std::getline(imageList, line))
        {

            std::istringstream recordStream(line);

            std::string imagePath;
            std::string specificCategoryText;
            std::string broaderCategoryText;

            if (!std::getline(recordStream, imagePath, ',') || !std::getline(recordStream, specificCategoryText, ',') || !std::getline(recordStream, broaderCategoryText))
            {
                throw std::runtime_error("Failed to parse image record line: " + line);
            }

            if (imagePath.empty())
                throw std::runtime_error("Empty image path in: " + listPath);

                int64_t specificCategory = parseCategoryId(specificCategoryText);
                int64_t broaderCategory = parseCategoryId(broaderCategoryText);
                int64_t label = mapCategory(broaderCategory);

                const bool isSelectedSpecific = specificCategory == 5 || specificCategory == 6 || specificCategory == 8;

                if (label == -1)
                {
                    if (isSelectedSpecific)
                    {
                        throw std::runtime_error("Selected fine category has wrong coarse category: " + line);
                    }

                    continue;
                }

                const std::vector<int64_t> expectedSpecificIds{5, 6, 8};

                if (specificCategory != expectedSpecificIds.at(label))
                {
                    throw std::runtime_error("Fine/coarse category mismatch: " + line);
                }
                ImageRecord record {imagePath, label};
                records.push_back(record);
        }
    return records;
}

torch::Tensor loadImageTensor(const std::string& fullImagePath)
{
    cv::Mat image = cv::imread(fullImagePath, cv::IMREAD_COLOR);
    if(image.empty())
        throw std::runtime_error("Failed to read image: " + fullImagePath);

    //resize and arrange color channels
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(64, 64), 0.0, 0.0, cv::INTER_AREA);

    cv::Mat rgbImage;
    cv::cvtColor(resizedImage, rgbImage, cv::COLOR_BGR2RGB);

    torch::Tensor imageTensor = torch::from_blob(rgbImage.data, {rgbImage.rows, rgbImage.cols, 3}, torch::TensorOptions().dtype(torch::kUInt8)).clone();

    imageTensor = imageTensor.to(torch::kFloat32) / 255.0f;
    imageTensor = imageTensor.permute({2,0,1}).contiguous();
    return imageTensor;
}

void checkPreprocessing()
{
    torch::NoGradGuard noGrad;

    const std::string fixturePath =
        "build-msvc/preprocessing-check.png";

    // OpenCV uses BGR: blue=0, green=128, red=255.
    cv::Mat fixture(7, 11, CV_8UC3, cv::Scalar(0, 128, 255));

    if (!cv::imwrite(fixturePath, fixture))
        throw std::runtime_error("Failed to write preprocessing fixture.");

    torch::Tensor image = loadImageTensor(fixturePath);

    if (image.dim() != 3 ||
        image.size(0) != 3 ||
        image.size(1) != 64 ||
        image.size(2) != 64)
    {
        throw std::runtime_error("Preprocessing shape check failed.");
    }

    if (image.scalar_type() != torch::kFloat32 ||
        !image.is_contiguous())
    {
        throw std::runtime_error("Preprocessing dtype/layout check failed.");
    }

    const std::vector<float> expectedValues{
        1.0f, 128.0f / 255.0f, 0.0f};

    for (int64_t channel = 0; channel < 3; ++channel)
    {
        torch::Tensor expectedChannel = torch::full_like(
            image[channel], expectedValues.at(channel));

        if (!torch::allclose(
                image[channel], expectedChannel, 1e-5, 1e-6))
        {
            throw std::runtime_error(
                "Color/scaling check failed for channel " +
                std::to_string(channel));
        }
    }

    torch::Tensor original = image.clone();

    fixture.setTo(cv::Scalar(0, 0, 0));

    if (!cv::imwrite(fixturePath, fixture))
        throw std::runtime_error("Failed to write black fixture.");

    torch::Tensor secondImage = loadImageTensor(fixturePath);

    if (!torch::equal(secondImage, torch::zeros_like(secondImage)))
        throw std::runtime_error("Black-image check failed.");

    secondImage.fill_(1.0f);

    if (!torch::equal(image, original))
        throw std::runtime_error("Loaded images unexpectedly share pixels.");

    std::cout
        << "Preprocessing checks passed: shape, float32, contiguous, "
        << "RGB order, scaling, and independent loaded tensors.\n";
}

void checkDatasetSplit(const std::string& splitName, std::unordered_map<std::string, std::string>& seenPaths, const std::string& manifestPath = "")
{
    const std::string datasetRoot = "data/GroceryStoreDataset/dataset/";
    const std::vector<std::string> classNames {"avocado", "banana", "lemon"};

    std::string listPath = manifestPath;

    if (listPath.empty())
        listPath = datasetRoot + splitName + ".txt";

    const std::vector<ImageRecord> records = readImageRecords(listPath);

    if (records.empty())
        throw std::runtime_error("No selected records in split: " + splitName);

    std::vector<int64_t> counts(3,0);

    for (const ImageRecord& record : records)
    {
        if (record.label < 0 || record.label >= static_cast<int64_t>(classNames.size()))
        {
            throw std::runtime_error("Invalid model label: " + record.path);
        }

        const auto insertion = seenPaths.emplace(record.path, splitName);

        if(!insertion.second)
        {
            throw std::runtime_error("Repeated image path: " + record.path + " | first split: " + insertion.first->second + " | repeated in: " + splitName);
        }

        torch::Tensor image = loadImageTensor(datasetRoot + record.path);

        if (image.dim() != 3 || image.size(0) != 3 || image.size(1) != 64 || image.size(2) != 64)
        {
            throw std::runtime_error("Unexpected image shape: " + record.path);
        }

        if (image.scalar_type() != torch::kFloat32 || !image.is_contiguous())
        {
            throw std::runtime_error("Unexpected image dtype or layout: " + record.path);
        }

        if (!torch::isfinite(image).all().item<bool>() || image.min().item<float>() < 0.0f || image.max().item<float>() > 1.0f)
        {
            throw std::runtime_error("Invalid pixel values: " + record.path);
        }
        ++counts.at(record.label);
    }

    std::cout << splitName << ": " <<records.size() << " selected images checked\n";

    for (size_t label = 0; label < classNames.size(); ++label)
    {
        std::cout << " " << classNames.at(label) << " (label " << label << "): " << counts.at(label) << std::endl;

    }

}

void evaluateCNN(const std::string& checkpointPath, const std::string& split)
{
    const std::string datasetRoot = "data/GroceryStoreDataset/dataset/";
    const auto evaluationRecords = readImageRecords(datasetRoot + split + ".txt");
    if (evaluationRecords.empty())
        throw std::runtime_error("No selected evaluation images.");

    torch::set_num_threads(1);
    torch::NoGradGuard noGrad;
    GroceryCNN model;
    torch::serialize::InputArchive archive;
    archive.load_from(checkpointPath);
    model.load(archive);
    model.eval();

    std::vector<torch::Tensor> images;
    std::vector<int64_t> labels;
    for (const auto& record : evaluationRecords)
    {
        images.push_back(loadImageTensor(datasetRoot + record.path));
        labels.push_back(record.label);
    }
    torch::Tensor evaluationInputs = torch::stack(images);
    torch::Tensor evaluationTargets =
        torch::tensor(labels, torch::TensorOptions().dtype(torch::kInt64));

    std::cout << "Checkpoint: " << checkpointPath << "\nSplit: " << split << '\n';
        torch::Tensor evaluationScores = model.forward(evaluationInputs);
        torch::Tensor evaluationLoss = torch::nn::functional::cross_entropy(evaluationScores, evaluationTargets);

        torch::Tensor predictedLabels = evaluationScores.argmax(1);
        int64_t correctCount = predictedLabels.eq(evaluationTargets).sum().item<int64_t>();

        const int64_t evaluationCount = evaluationTargets.size(0);
        double accuracy = 100.0 * correctCount / evaluationCount;

        std::cout << "Evaluation loss: " << evaluationLoss.item<float>() << std::endl; std::cout <<"Evaluation accuracy: "
        <<accuracy << "% ( " << correctCount << "/" << evaluationCount << ")\n";

        const std::vector<std::string> classNames { "avocado", "banana", "lemon"};

        const int64_t majorityLabel = 1; // Banana: most common training class
        int64_t baselineCorrect = evaluationTargets.eq(majorityLabel).sum().item<int64_t>();

        std::cout <<"Always-banana accuracy: " << 100.0 * baselineCorrect / evaluationCount << "%\n";

        const size_t classCount = classNames.size();
        std::vector<std::vector<int64_t>> confusion (classCount, std::vector<int64_t>(classCount, 0));

        for (int64_t index = 0; index < evaluationCount; ++index)
        {
            int64_t predicted = predictedLabels[index].item<int64_t>();
            int64_t actual = evaluationTargets[index].item<int64_t>();
            ++confusion.at(actual).at(predicted);
            {
                if (predicted != actual)
                {
                    std::cout << "Mistake: " << evaluationRecords.at(index).path << ", Predicted: "
                    << classNames[predicted] << ", Actual: " << classNames[actual] << std::endl;
                }
            }
        }

        std::cout << "\nEvaluation confusion matrix\n";
        std::cout << "Rows=actual, columns=predicted\n";
        std::cout << "Class order: avocado banana lemon\n";

        for (size_t actual = 0; actual < classCount; ++actual)
        {
            std::cout << classNames.at(actual) << ':';

            for (size_t predicted = 0; predicted < classCount; ++predicted)
                std::cout << ' ' << confusion.at(actual).at(predicted);

            std::cout << '\n';
        }

        std::cout << "\nEvaluation per-class metrics\n";

        for (size_t label = 0; label < classCount; ++label)
        {
            const int64_t truePositives = confusion.at(label).at(label);
            int64_t actualCount = 0;
            int64_t predictedCount = 0;

            for (size_t other = 0; other < classCount; ++other)
            {
                actualCount += confusion.at(label).at(other);
                predictedCount += confusion.at(other).at(label);
            }

            std::cout << classNames.at(label)
                      << " | support=" << actualCount;

            if (predictedCount > 0)
            {
                std::cout << " | precision="
                          << 100.0 * truePositives / predictedCount << '%';
            }
            else
            {
                std::cout << " | precision=N/A (no predictions)";
            }

            if (actualCount > 0)
            {
                std::cout << " | recall="
                          << 100.0 * truePositives / actualCount << '%';
            }
            else
            {
                std::cout << " | recall=N/A (no examples)";
            }

            std::cout << '\n';
        }
}

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "evaluate-cnn")
    {
        if (argc != 4 || (std::string(argv[3]) != "val" && std::string(argv[3]) != "test"))
        {
            std::cerr << "Usage: LiveVision.exe evaluate-cnn <checkpoint-path> <val|test>\n";
            return 1;
        }
        try
        {
            evaluateCNN(argv[2], argv[3]);
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Evaluation failed: " << error.what() << '\n';
            return 1;
        }
    }
    if (argc > 1 && std::string(argv[1]) == "preprocessing-check")
    {
        if (argc != 2)
        {
            std::cerr << "Usage: LiveVision.exe preprocessing-check\n";
            return 1;
        }

        try
        {
            checkPreprocessing();
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Preprocessing check failed: "
                      << error.what() << '\n';
            return 1;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "dataset-check-list")
    {
        if (argc < 3)
        {
            std::cerr <<"Usage: LiveVision.exe dataset-check-list " << "<manifest-path> [more-manifest-paths...]\n";
            return 1;
        }

        try
        {
            std::unordered_map<std::string, std::string> seenPaths;

            for (int argument = 2; argument < argc; ++argument)
            {
                const std::string manifestPath = argv[argument];

                checkDatasetSplit(manifestPath, seenPaths, manifestPath);
            }

            std::cout << "Supplied manifest checks passed.\n";
            std::cout << "Unique listed paths: " << seenPaths.size() << std::endl;
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Manifest check failed: " <<error.what() << std::endl;
            return 1;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "dataset-check")
    {
        if (argc != 2)
        {
            std::cerr << "Usage: LiveVision.exe dataset-check\n";
            return 1;
        }

        try
        {

            std::unordered_map<std::string, std::string> seenPaths;

            checkDatasetSplit("train", seenPaths);
            checkDatasetSplit("val", seenPaths);
            checkDatasetSplit("test", seenPaths);

            std::cout << "Dataset image checks passed." << std::endl;
            std::cout << "Unique listed paths: " << seenPaths.size() << std::endl;
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Dataset check failed: " << error.what() << std::endl;
            return 1;
        }
    }

    if (argc == 2 && std::string(argv[1]) == "cnn-check")
    {
        torch::NoGradGuard noGrad;

        torch::Tensor inputs = torch::rand({2, 3, 64, 64});

        torch::nn::Conv2d convolution(
            torch::nn::Conv2dOptions(3, 8, 3).stride(1).padding(1));

        torch::nn::MaxPool2d pooling(
            torch::nn::MaxPool2dOptions(2).stride(2));

        torch::Tensor features = convolution->forward(inputs);
        torch::Tensor activated = torch::relu(features);
        torch::Tensor pooled = pooling->forward(activated);

        std::cout << "Input: " << inputs.sizes() << std::endl;
        std::cout << "After convolution: " << features.sizes() << std::endl;
        std::cout << "After ReLU: " << activated.sizes() << std::endl;
        std::cout << "After pooling: " << pooled.sizes() << std::endl;

        std::cout << "Convolution weights: "
                  << convolution->weight.sizes() << std::endl;
        std::cout << "Convolution biases: "
                  << convolution->bias.sizes() << std::endl;


        GroceryCNN cnn;
        cnn.eval();
        torch::Tensor scores = cnn.forward(inputs);

        std::cout <<"CNN output shape: " <<scores.sizes() << std::endl;
        std::cout << "CNN class scores:\n" << scores << std::endl;

        int64_t parameterCount = 0;

        for (const auto& parameter : cnn.named_parameters())
        {
            std::cout <<"Parameter: " <<parameter.key() << " | Size: " << parameter.value().sizes() << std::endl;
            parameterCount += parameter.value().numel();
        }
        std::cout << "Total number of CNN parameters: " << parameterCount << std::endl;

        return 0;
    }

    if (argc > 1 && std::string(argv[1]) == "predict-cnn")
    {
        if (argc != 3)
        {
            std::cerr << "Usage: LiveVision.exe predict-cnn <image-path>\n";
            return 1;
        }

        try
        {
            GroceryCNN predictionModel;

            torch::serialize::InputArchive archive;
            archive.load_from("build-msvc/saved-cnn.pt");
            predictionModel.load(archive);
            predictionModel.eval();

            torch::NoGradGuard noGrad;

            torch::Tensor imageTensor = loadImageTensor(argv[2]);
            torch::Tensor predictionInput = imageTensor.unsqueeze(0);
            torch::Tensor predictionScores = predictionModel.forward(predictionInput);

            int64_t predictedLabel = predictionScores.argmax(1).item<int64_t>();

            const std::vector<std::string> classNames{
                "avocado", "banana", "lemon"};

            std::cout << "CNN predicted grocery: " << classNames.at(predictedLabel) << std::endl;
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "CNN prediction failed: " << error.what() << std::endl;
            return 1;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "predict")
    {
        if (argc != 3 )
        {
            std::cerr << "Usage: LiveVision.exe predict <image-path>\n";
            return 1;
        }
        try
        {
            GroceryClassifier predictionModel ( 3 * 64 * 64, 3);

            torch::serialize::InputArchive archive;
            archive.load_from("build-msvc/saved-model.pt");
            predictionModel.load(archive);
            predictionModel.eval();

            torch::NoGradGuard noGrad;

            torch::Tensor imageTensor = loadImageTensor(argv[2]);
            torch::Tensor predictionInput = imageTensor.unsqueeze(0).flatten(1);

            torch::Tensor predictionScores = predictionModel.forward(predictionInput);

            int64_t predictedLabel = predictionScores.argmax(1).item<int64_t>();

            const std::vector<std::string> classNames {"avocado", "banana", "lemon" };

            std::cout << "Predicted grocery: " << classNames.at(predictedLabel) << std::endl;
            return 0;
        }

        catch(const std::exception& error)
        {
            std::cerr << "Prediction failed: " << error.what() << std::endl;
            return 1;
        }
    }

    if (argc != 2 || std::string(argv[1]) != "train")
    {
        std::cerr
            << "Usage:\n"
            << "  LiveVision.exe train\n"
            << "  LiveVision.exe evaluate-cnn <checkpoint-path> <val|test>\n"
            << "  LiveVision.exe dataset-check\n"
            << "  LiveVision.exe dataset-check-list <manifest> [more...]\n"
            << "  LiveVision.exe cnn-check\n"
            << "  LiveVision.exe predict <image-path>\n"
            << "  LiveVision.exe predict-cnn <image-path>\n";
        return 1;
    }

    const int64_t trainingSeed = 42;
    const int cpuThreads = 1;
    const std::string trainingCheckpoint = "build-msvc/phase7-run-b.pt";

    torch::manual_seed(trainingSeed);
    torch::set_num_threads(cpuThreads);

    std::cout << "Training seed: " << trainingSeed << std::endl;
    std::cout << "CPU computation threads: " << cpuThreads << std::endl;

    std::cout << "Training checkpoint: " << trainingCheckpoint << std::endl;

    std::vector<ImageRecord> trainingRecords = readImageRecords("data/GroceryStoreDataset/dataset/train.txt");
    std::vector<ImageRecord> validationRecords = readImageRecords("data/GroceryStoreDataset/dataset/val.txt");
    std::vector<ImageRecord> testRecords = readImageRecords("data/GroceryStoreDataset/dataset/test.txt");

    std::cout << "Selected validation images: " << validationRecords.size() << std::endl;
    std::cout << "Selected test images: " << testRecords.size() << std::endl;
    std::cout << "Selected training images: " << trainingRecords.size() << std::endl;

    const ImageRecord& firstRecord = trainingRecords.at(0);

    std::string fullImagePath = "data/GroceryStoreDataset/dataset/" + firstRecord.path;
    std::cout << "First image: " << fullImagePath << std::endl;

    torch::Tensor imageTensor = loadImageTensor(fullImagePath);

    std::cout << "Image tensor shape: " << imageTensor.sizes() << std::endl;
    std::cout << "pixel range: [" << imageTensor.min().item<float>() << ", " << imageTensor.max().item<float>() << "]" << std::endl;


    std::vector<torch::Tensor> batchImages;
    std::vector<int64_t> batchLabels;

    for(size_t index = 0; index <  trainingRecords.size(); ++index)
    {
        const ImageRecord& record = trainingRecords.at(index);

        std::string imagePath = "data/GroceryStoreDataset/dataset/" + record.path;

        batchImages.push_back(loadImageTensor(imagePath));
        batchLabels.push_back(record.label);
    }
    torch::Tensor imageBatch = torch::stack(batchImages);

    torch::Tensor labelBatch = torch::tensor(batchLabels, torch::TensorOptions().dtype(torch::kInt64));

    std::cout << "Image batch shape: " << imageBatch.sizes() << std::endl;
    std::cout << "Label batch shape: " << labelBatch.sizes() << std::endl;
    std:: cout <<"Batch labels: " <<labelBatch << std::endl;

   GroceryCNN model;

   torch::Tensor scores = model.forward(imageBatch);

    std::cout << "Output shape: " << scores.sizes() << std::endl;
    std::cout << "Class scores:\n" << scores << std::endl;


    for (const auto & parameter : model.named_parameters())
        std::cout << "Parameter name: " << parameter.key() << ", shape: " << parameter.value().sizes() << std::endl;

    model.train();

    const double learningRate = 0.003;
    torch::optim::SGD groceryOptimizer(model.parameters(), torch::optim::SGDOptions(learningRate));
    const int64_t epochCount = 20;

    const int64_t batchSize = 16;
    const int64_t exampleCount = imageBatch.size(0);

    std::vector<torch::Tensor> validationImages;
        std::vector<int64_t> validationLabels;

        for(const ImageRecord& record : validationRecords)
        {
            std::string imagePath = "data/GroceryStoreDataset/dataset/" + record.path;

            validationImages.push_back(loadImageTensor(imagePath));
            validationLabels.push_back(record.label);
        }

        torch::Tensor validationInputs = torch::stack(validationImages);

        torch::Tensor validationTargets = torch::tensor(validationLabels, torch::TensorOptions().dtype(torch::kInt64));

    double bestValidationLoss = 0.0;
    int64_t bestEpoch = -1;
    torch::Tensor bestValidationScores;

    for (int64_t epoch = 0; epoch < epochCount; ++epoch)
    {

        model.train();

        torch::Tensor shuffledIndices = torch::randperm(exampleCount, torch::TensorOptions().dtype(torch::kInt64));

        double totalLoss = 0.0;

        for (int64_t start = 0; start < exampleCount; start += batchSize)
        {
            torch::Tensor batchIndices = shuffledIndices.slice(0, start, start + batchSize);

            torch::Tensor batchInputs = imageBatch.index_select(0, batchIndices);
            torch::Tensor batchTargets = labelBatch.index_select(0, batchIndices);

            groceryOptimizer.zero_grad();

            torch::Tensor batchScores = model.forward(batchInputs);

            torch::Tensor batchLoss = torch::nn::functional::cross_entropy(batchScores, batchTargets);

            batchLoss.backward();
            groceryOptimizer.step();

            totalLoss += batchLoss.item<double>() * batchInputs.size(0);
        }

        model.eval();
        {
            torch::NoGradGuard noGrad;

            torch::Tensor validationScores = model.forward(validationInputs);
            torch::Tensor validationLoss =
                torch::nn::functional::cross_entropy(
                    validationScores, validationTargets);

            torch::Tensor predictedLabels = validationScores.argmax(1);
            int64_t correctCount =
                predictedLabels.eq(validationTargets).sum().item<int64_t>();

            double validationAccuracy =
                100.0 * correctCount / validationTargets.size(0);

            double currentValidationLoss = validationLoss.item<double>();

           if (bestEpoch == -1 || currentValidationLoss < bestValidationLoss)
            {
                torch::serialize::OutputArchive bestArchive;
                model.save(bestArchive);
                bestArchive.save_to(trainingCheckpoint);

                bestValidationLoss = currentValidationLoss;
                bestEpoch = epoch + 1;
                bestValidationScores = validationScores.clone();

                std::cout << "Saved best epoch: " << bestEpoch << " | Validation loss: " << bestValidationLoss << '\n';
            }
            std::cout
                << "Epoch: " << (epoch + 1)
                << " | Training loss: " << totalLoss / exampleCount
                << " | Validation loss: " << validationLoss.item<double>()
                << " | Validation accuracy: " << validationAccuracy << "%\n";
        }
    }

    if (bestEpoch == -1)
    {
        std::cerr << "No best checkpoint was saved.\n";
        return 1;
    }

    torch::serialize::InputArchive bestArchive;
    bestArchive.load_from(trainingCheckpoint);
    model.load(bestArchive);

    std::cout << "Selected epoch: " << bestEpoch << " | Best validation loss: " << bestValidationLoss << '\n';

    model.eval();
    {
        torch::NoGradGuard noGrad;

        torch::Tensor validationScores = model.forward(validationInputs);
        torch::Tensor validationLoss = torch::nn::functional::cross_entropy(validationScores, validationTargets);

        torch::Tensor predictedLabels = validationScores.argmax(1);
        int64_t correctCount = predictedLabels.eq(validationTargets).sum().item<int64_t>();

        const int64_t validationCount = validationTargets.size(0);
        double accuracy = 100.0 * correctCount / validationCount;

        std::cout << "Validation loss: " << validationLoss.item<float>() << std::endl; std::cout <<"Validation accuracy: "
        <<accuracy << "% ( " << correctCount << "/" << validationCount << ")\n";

        const std::vector<std::string> classNames { "avocado", "banana", "lemon"};

        const int64_t majorityLabel = 1; // Banana: most common training class
        int64_t baselineCorrect = validationTargets.eq(majorityLabel).sum().item<int64_t>();

        std::cout <<"Always-banana accuracy: " << 100.0 * baselineCorrect / validationCount << "%\n";

        const size_t classCount = classNames.size();
        std::vector<std::vector<int64_t>> confusion (classCount, std::vector<int64_t>(classCount, 0));

        for (int64_t index = 0; index < validationCount; ++index)
        {
            int64_t predicted = predictedLabels[index].item<int64_t>();
            int64_t actual = validationTargets[index].item<int64_t>();
            ++confusion.at(actual).at(predicted);
            {
                if (predicted != actual)
                {
                    std::cout << "Mistake: " << validationRecords.at(index).path << ", Predicted: "
                    << classNames[predicted] << ", Actual: " << classNames[actual] << std::endl;
                }
            }
        }

        std::cout << "\nValidation confusion matrix\n";
        std::cout << "Rows=actual, columns=predicted\n";
        std::cout << "Class order: avocado banana lemon\n";

        for (size_t actual = 0; actual < classCount; ++actual)
        {
            std::cout << classNames.at(actual) << ':';

            for (size_t predicted = 0; predicted < classCount; ++predicted)
                std::cout << ' ' << confusion.at(actual).at(predicted);

            std::cout << '\n';
        }

        std::cout << "\nValidation per-class metrics\n";

        for (size_t label = 0; label < classCount; ++label)
        {
            const int64_t truePositives = confusion.at(label).at(label);
            int64_t actualCount = 0;
            int64_t predictedCount = 0;

            for (size_t other = 0; other < classCount; ++other)
            {
                actualCount += confusion.at(label).at(other);
                predictedCount += confusion.at(other).at(label);
            }

            std::cout << classNames.at(label)
                      << " | support=" << actualCount;

            if (predictedCount > 0)
            {
                std::cout << " | precision="
                          << 100.0 * truePositives / predictedCount << '%';
            }
            else
            {
                std::cout << " | precision=N/A (no predictions)";
            }

            if (actualCount > 0)
            {
                std::cout << " | recall="
                          << 100.0 * truePositives / actualCount << '%';
            }
            else
            {
                std::cout << " | recall=N/A (no examples)";
            }

            std::cout << '\n';
        }

    }


        GroceryCNN loadedModel;

        torch::serialize::InputArchive loadedArchive;
        loadedArchive.load_from(trainingCheckpoint);
        loadedModel.load(loadedArchive);
        loadedModel.eval();


    {
        torch::NoGradGuard noGrad;

        torch::Tensor reloadedScores = loadedModel.forward(validationInputs);

        bool scoresMatch =
        torch::allclose(bestValidationScores, reloadedScores);

        std::cout << "Grocery scores match after reload: " << std::boolalpha << scoresMatch << std::endl;

        if (!scoresMatch)
        {
            std::cerr << "Error: Scores do not match after reloading the model." << std::endl;
            return 1;
        }

    }

        {
        const std::string metadataPath = trainingCheckpoint + ".metadata.txt";
        std::ofstream metadata(metadataPath);

        if (!metadata.is_open())
        {
            std::cerr << "Failed to open metadata file: "
                      << metadataPath << '\n';
            return 1;
        }

        metadata
            << "metadata_version=1\n"
            << "checkpoint=" << trainingCheckpoint << '\n'
            << "model=GroceryCNN\n"
            << "architecture=Conv2d(3,8,3,stride=1,padding=1)"
            << " -> ReLU -> MaxPool2d(2,stride=2)"
            << " -> flatten(1) -> Linear(8192,3)\n"
            << "labels=0:avocado,1:banana,2:lemon\n"
            << "dataset_coarse_ids=1,2,4\n"
            << "dataset_fine_ids=5,6,8\n"
            << "dataset_source=https://github.com/marcusklasson/GroceryStoreDataset\n"
            << "expected_dataset_revision=fc80ba90f803d79d0383df52c5a4ac5de99ff6fc\n"
            << "split_policy=upstream; grouping audit incomplete\n"
            << "train_manifest=data/GroceryStoreDataset/dataset/train.txt\n"
            << "validation_manifest=data/GroceryStoreDataset/dataset/val.txt\n"
            << "test_manifest=data/GroceryStoreDataset/dataset/test.txt\n"
            << "training_count=" << trainingRecords.size() << '\n'
            << "validation_count=" << validationRecords.size() << '\n'
            << "test_count=" << testRecords.size() << '\n'
            << "test_evaluated=false\n"
            << "resize=64x64; INTER_AREA; direct resize\n"
            << "color=RGB\n"
            << "tensor=float32; CHW; contiguous; owned\n"
            << "scaling=uint8 / 255; range=[0,1]\n"
            << "augmentation=none\n"
            << "device=CPU\n"
            << "seed=" << trainingSeed << '\n'
            << "cpu_threads=" << cpuThreads << '\n'
            << "optimizer=SGD\n"
            << "learning_rate=" << learningRate << '\n'
            << "momentum=0\n"
            << "weight_decay=0\n"
            << "epochs=" << epochCount << '\n'
            << "batch_size=" << batchSize << '\n'
            << "loss=cross_entropy\n"
            << "checkpoint_selection=lowest_validation_loss\n"
            << "selected_epoch=" << bestEpoch << '\n'
            << "best_validation_loss=" << bestValidationLoss << '\n'
            << "reload_allclose=true\n";

        metadata.close();

        if (!metadata)
        {
            std::cerr << "Failed to finish writing metadata: "
                      << metadataPath << '\n';
            return 1;
        }

        std::cout << "Saved metadata: " << metadataPath << '\n';
    }

    torch::Tensor inputs = torch::tensor({1.0f, 2.0f, 3.0f});
    torch::Tensor labels = torch::tensor({3.0f, 5.0f, 7.0f});

    torch::Tensor bias = torch::tensor(0.0f, torch::TensorOptions().requires_grad(true));
    torch::Tensor weight = torch::tensor(0.0f, torch::TensorOptions().requires_grad(true));

    torch::optim::SGD optimizer({weight, bias}, torch::optim::SGDOptions(0.1));

    for(int step = 0; step < 500; ++step)
    {
        optimizer.zero_grad();
        torch::Tensor predictions = inputs * weight + bias;
        torch::Tensor loss = (predictions - labels).square().mean();
        loss.backward(); //compute gradients
        optimizer.step(); //updates parameters

        {

            torch::NoGradGuard noGrad;
            torch::Tensor updatedPredictions = inputs * weight + bias;
            torch::Tensor updatedLoss = (updatedPredictions - labels).square().mean();
            if ((step + 1) % 50 == 0)
            {
                std::cout << "Step: " << step << ", Updated Weight: " << weight.item<float>() << ", Updated Bias: " << bias.item<float>() << ", Updated Loss: " << updatedLoss.item<float>() << std::endl;
            }
        }
    }

    {
        torch::NoGradGuard noGrad;

        torch::Tensor newInputs = torch::tensor({4.0f, 5.0f});
        torch::Tensor expected = torch::tensor({9.0f, 11.0f});

        torch::Tensor newPredictions = newInputs * weight + bias;
        torch::Tensor evaluationLoss = (newPredictions - expected).square().mean();

        std:: cout << "New predictions " << newPredictions << std::endl;
        std:: cout << "Evaluation Loss " << evaluationLoss.item<float>() << std::endl;

        torch::save(weight, "build-msvc/learned-weight.pt");
        torch::save(bias, "build-msvc/learned-bias.pt");

        std::cout << "Saved weight and bias.\n";

        torch::Tensor loadedWeight;
        torch::Tensor loadedBias;

        torch::load(loadedWeight, "build-msvc/learned-weight.pt");
        torch::load(loadedBias, "build-msvc/learned-bias.pt");

        torch::Tensor loadedPredictions = newInputs * loadedWeight + loadedBias;

        bool predictionsMatch = torch::allclose(loadedPredictions, newPredictions);

        std::cout << "Loaded predictions: " << loadedPredictions << std::endl;
        std::cout << "Predictions match before saving: " << std::boolalpha << predictionsMatch << std::endl;

        if (!predictionsMatch)
        {
            std::cerr << "Error: Loaded predictions do not match predictions before saving." << std::endl;
            return 1;
        }
    }
    for (int category : {1, 2, 4, 0})
        std::cout << mapCategory(category) << std::endl;


}
