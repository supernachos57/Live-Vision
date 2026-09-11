#include <torch/torch.h>
#include <iostream>

int main()
{
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
        loss.backward();
        optimizer.step();

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


}