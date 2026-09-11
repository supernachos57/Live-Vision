# Phase 3 learning checkpoint

Verified September 11, 2026. Scheduled due date: September 10, 2026.

Using CPU LibTorch on Windows with MSVC, the C++ example learns scalar weight and bias from inputs 1, 2, 3 and labels 3, 5, 7. SGD performs 500 updates at learning rate 0.1. Each iteration clears gradients, calculates loss, backpropagates, and updates parameters. Evaluation uses NoGradGuard.

## Verified results

- Final weight approximately 2 and bias 0.999998.
- Training MSE approximately 7.77e-13.
- New inputs 4 and 5 produce predictions approximately 9 and 11; evaluation MSE 5.91e-12.
- Parameters saved separately in build-msvc/learned-weight.pt and build-msvc/learned-bias.pt, then loaded into new tensors.
- Loaded predictions match pre-save predictions using torch::allclose; a mismatch returns exit code 1.
- MSVC Release build succeeded; existing GoogleTest smoke test passed 1/1.

Run from the repository root in an x64 Visual Studio developer terminal:

```bat
cmake --build build-msvc
build-msvc\LiveVision.exe
ctest --test-dir build-msvc --output-on-failure
```

The checkpoint files are generated under the ignored build-msvc directory. LibTorch is installed separately and configured with CMAKE_PREFIX_PATH.

## Limits and next step

This is a synthetic linear learning exercise, not a grocery classifier. Reload verification occurs within the same run; restarting still retrains. The existing CTest smoke test checks test infrastructure, while the application checks reload parity. No claim is made about real-world image accuracy or a standalone inference mode.

Next: Phase 4, a small grocery-classification baseline. Continue reinforcing model APIs and training/evaluation concepts as the image pipeline is introduced.
