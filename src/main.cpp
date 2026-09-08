#include <torch/torch.h>
#include <iostream>

int main()
{
    torch::Tensor inputs = torch::tensor({1.0f, 2.0f, 3.0f});
    torch::Tensor labels = torch::tensor({3.0f, 5.0f, 7.0f});
    torch::Tensor predictions = inputs * 2 + 1;
    torch::Tensor errors = predictions - labels;
    torch::Tensor loss = errors.square().mean();

    std::cout <<"Predictions: " << predictions << std::endl;
    std::cout <<"Errors: " << errors << std::endl;
    std::cout <<"Loss: " << loss << std::endl;

}