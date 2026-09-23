#include <torch/torch.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
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

            int64_t broaderCategory = std::stoll(broaderCategoryText);
            int64_t label = mapCategory(broaderCategory);

            if (label != -1)
            {
                ImageRecord record{imagePath, label};
                records.push_back(record);
            }
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


int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (argc != 3 || std::string(argv[1]) != "predict")
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

    torch::Tensor flattenedBatch = imageBatch.flatten(1);

    const int64_t inputFeatures = 3 * 64 * 64;
    const int64_t classCount = 3;

    GroceryClassifier model(inputFeatures, classCount);

    torch::Tensor scores = model.forward(flattenedBatch);

    std::cout << "Flattened batch shape: " << flattenedBatch.sizes() << std::endl;
    std::cout << "Output shape: " << scores.sizes() << std::endl;
    std::cout << "Class scores:\n" << scores << std::endl;


    for (const auto & parameter : model.named_parameters())
        std::cout << "Parameter name: " << parameter.key() << ", shape: " << parameter.value().sizes() << std::endl;

    model.train();

    torch::optim::SGD groceryOptimizer(model.parameters(), torch::optim::SGDOptions(0.001));
    const int64_t epochCount = 20;
    const int64_t batchSize = 16;
    const int64_t exampleCount = flattenedBatch.size(0);

    for (int64_t epoch = 0; epoch < epochCount; ++epoch)
    {
        torch::Tensor shuffledIndices = torch::randperm(exampleCount, torch::TensorOptions().dtype(torch::kInt64));

        double totalLoss = 0.0;

        for (int64_t start = 0; start < exampleCount; start += batchSize)
        {
            torch::Tensor batchIndices = shuffledIndices.slice(0, start, start + batchSize);

            torch::Tensor batchInputs = flattenedBatch.index_select(0, batchIndices);
            torch::Tensor batchTargets = labelBatch.index_select(0, batchIndices);

            groceryOptimizer.zero_grad();

            torch::Tensor batchScores = model.forward(batchInputs);

            torch::Tensor batchLoss = torch::nn::functional::cross_entropy(batchScores, batchTargets);

            batchLoss.backward();
            groceryOptimizer.step();

            totalLoss += batchLoss.item<double>() * batchInputs.size(0);
        }
        std::cout << "Epoch: " << epoch << ", Average Loss: " << totalLoss / exampleCount << std::endl;
    }

    model.eval();
    {
        torch::NoGradGuard noGrad;

        std::vector<torch::Tensor> validationImages;
        std::vector<int64_t> validationLabels;

        for(const ImageRecord& record : validationRecords)
        {
            std::string imagePath = "data/GroceryStoreDataset/dataset/" + record.path;

            validationImages.push_back(loadImageTensor(imagePath));
            validationLabels.push_back(record.label);
        }

        torch::Tensor validationInputs = torch::stack(validationImages).flatten(1);

        torch::Tensor validationTargets = torch::tensor(validationLabels, torch::TensorOptions().dtype(torch::kInt64));

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

        for (int64_t index = 0; index < validationCount; ++index)
        {
            int64_t predicted = predictedLabels[index].item<int64_t>();
            int64_t actual = validationTargets[index].item<int64_t>();
            {
                if (predicted != actual)
                {
                    std::cout << "Mistake: " << validationRecords.at(index).path << ", Predicted: "
                    << classNames[predicted] << ", Actual: " << classNames[actual] << std::endl;
                }
            }
        }
    }

    torch::serialize::OutputArchive modelArchive;
    model.save(modelArchive);
    modelArchive.save_to("build-msvc/saved-model.pt");

    GroceryClassifier loadedModel(inputFeatures, classCount);

    torch::serialize::InputArchive loadedArchive;
    loadedArchive.load_from("build-msvc/saved-model.pt");
    loadedModel.load(loadedArchive);
    loadedModel.eval();

    {
        torch::NoGradGuard noGrad;

        torch::Tensor originalScores = model.forward(flattenedBatch);
        torch::Tensor reloadedScores = loadedModel.forward(flattenedBatch);

        bool scoresMatch = torch::allclose(originalScores, reloadedScores);

        std::cout << "Grocery scores match after reload: " << std::boolalpha << scoresMatch << std::endl;

        if (!scoresMatch)
        {
            std::cerr << "Error: Scores do not match after reloading the model." << std::endl;
            return 1;
        }

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
