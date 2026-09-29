# Live-Vision

A C++ learning project that classifies grocery images using LibTorch and OpenCV. The developer writes the application and training/inference pipeline; the libraries provide image decoding, tensors, layers, automatic differentiation, and optimizers.

## Current status

Updated September 29, 2026. Phases 1–6 learning/engineering checkpoints are complete. On September 29 the user accepted the bounded dataset audit with unchanged upstream splits and explicitly unverified capture-session independence. Phase 7 reporting is implemented but selection/test evaluation remain outstanding. The application trains a small CNN to classify **avocado, banana, and lemon**, validates it, saves/reloads its weights, and predicts without retraining. Linear-baseline inference remains available. Live camera capture is not implemented.

- [x] Phase 1 — C++ engineering setup and testing
- [x] Phase 2 — ML foundations and library decision
- [x] Phase 3 — Library learning exercise
- [x] Phase 4 — Baseline grocery classifier
- [x] Phase 5 — CNNs and image classification
- [x] Phase 6 — Dataset engineering and reproducibility
- [ ] Phase 7 — Model evaluation and selection
- [ ] Phase 8 — Live camera application
- [ ] Phase 9 — ONNX Runtime and detection comparison
- [ ] Phase 10 — Release and presentation

The saved baseline's observed validation result was **10/16 correct (62.5%), loss 0.748435**, compared with **6/16 (37.5%)** for always predicting banana. These are small validation-set results, not held-out test results or a real-world reliability claim. That historical baseline training was unseeded; current CNN training uses seed 42. See [Phase 4 results and limitations](phase-4-results.md).

The first CNN run achieved **8/16 (50%), validation loss 0.973922**, below the retained linear baseline. Its reload parity and prediction-only path were verified. See [Phase 5 results](phase-5-results.md) for architecture, settings, mistakes, and limitations.

## Dependencies and toolchain

The verified setup is Windows x64, Visual Studio 2022 C++ tools (MSVC 14.42), C++20, CMake/Ninja, CPU LibTorch (installed headers report 2.14.0), OpenCV 5.0.0, and GoogleTest v1.18.0 fetched by CMake.

Use **x64 Native Tools Command Prompt for VS 2022**, not MSYS2 UCRT64 Bash, for this LibTorch/OpenCV build. The earlier UCRT64 setup was the Phase 1 toolchain.

Dependencies are installed separately. The following commands use this machine's paths; change the library paths on another machine. LibTorch and OpenCV must be compatible x64 builds. Do not commit dependency binaries or dataset images.

## Dataset

Clone [GroceryStoreDataset](https://github.com/marcusklasson/GroceryStoreDataset) under `data/GroceryStoreDataset`. The verified local dataset revision is `fc80ba90f803d79d0383df52c5a4ac5de99ff6fc`:

```bat
git clone https://github.com/marcusklasson/GroceryStoreDataset.git data/GroceryStoreDataset
git -C data/GroceryStoreDataset checkout fc80ba90f803d79d0383df52c5a4ac5de99ff6fc
```

The application filters the upstream train/validation/test lists separately using broader category IDs 1, 2, and 4. It selects 128 training, 16 validation, and 125 test records. Test files have been inspected for integrity and possible overlap, but have not been used for model training, tuning, or classifier evaluation. Current manifests, fingerprints, provenance, preprocessing and audit limitations are recorded in [the dataset audit](docs/dataset.md).

## Configure, build, and test

Run from the repository root. For the current checkout:

```bat
cd /d C:\Users\ryanw\source\programming_projects\Live-Vision
set "PATH=C:\Users\ryanw\Libraries\opencv\build\x64\vc16\bin;%PATH%"
cmake -S . -B build-msvc -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Users/ryanw/Libraries/libtorch -DOpenCV_DIR=C:/Users/ryanw/Libraries/opencv/build
cmake --build build-msvc --target LiveVision LiveVisionTests
ctest --test-dir build-msvc --output-on-failure
```

The `PATH` command is needed in each new terminal so Windows can locate the OpenCV DLL. CMake copies LibTorch DLLs beside the executable. The OpenCV distribution uses a `vc16` folder even with the verified VS 2022 build; use the directory actually installed on your machine.

The existing CTest test is a smoke test of the test infrastructure, not a test of classifier quality. The September 27 build and CTest run passed (1/1).

## Dataset and preprocessing checks

```bat
build-msvc\LiveVision.exe preprocessing-check
build-msvc\LiveVision.exe dataset-check
build-msvc\LiveVision.exe dataset-check-list docs/manifests/train.txt docs/manifests/val.txt docs/manifests/test.txt
```

These commands exit before training. The preprocessing command writes a synthetic PNG in build-msvc. Supplied manifests contain paths relative to the dataset root. The dataset check reports 269 unique selected paths. See [Phase 6 evidence](phase-6-results.md).

## Train and save

```bat
build-msvc\LiveVision.exe train
```

Training is explicit; no arguments print usage and return 1. The CNN trains for 20 epochs with seed 42, one CPU computation thread, batches of 16, SGD learning rate 0.001 and cross-entropy. Each run overwrites `build-msvc/saved-cnn-phase6.pt`, reloads it to check score parity, and writes `saved-cnn-phase6.pt.metadata.txt`. Current validation is 9/16 (56.25%), loss 1.00202. Two fresh runs produced identical printed logs on this setup; this is not a cross-platform guarantee.

The Phase 3 scalar exercise still runs afterward with its separate outputs/files. There is no optimizer resume or best-checkpoint selection. Confusion-matrix and per-class reporting are present as initial Phase 7 work. Existing prediction commands still use the older retained checkpoints, not the Phase 6 file.

## Predict without training

Run from the repository root. CNN prediction uses `saved-cnn.pt`; the existing `predict` command uses the retained linear `saved-model.pt`:

```bat
build-msvc\LiveVision.exe predict-cnn "data/GroceryStoreDataset/dataset/val/Fruit/Banana/Banana_001.jpg"
build-msvc\LiveVision.exe predict "data/GroceryStoreDataset/dataset/val/Fruit/Banana/Banana_001.jpg"
```

Use an absolute image path or a path relative to the current working directory, quoting paths containing spaces. The command loads the saved grocery model, uses the shared preprocessing function, prints one grocery name, and exits without training or modifying the checkpoint. The checkpoint location is currently fixed relative to the repository root.

Invalid arguments print usage and return 1. Model/image loading failures in prediction mode print an error and return 1. Successful prediction returns 0. The classifier always chooses one of the three supported categories; it has no unknown-object rejection or localization.

The first CNN checkpoint predicts lemon for this banana image, matching its validation mistake. `build-msvc\LiveVision.exe cnn-check` inspects shapes and parameter counts using random inputs without training or saving.

## Project notes

- [Phase 6 results](phase-6-results.md) — dataset audit, reproducibility, metadata and remaining grouped-split limitation.
- [Phase 5 results](phase-5-results.md) — CNN architecture, measured comparison, checkpoint and inference verification.
- [Phase 4 results](phase-4-results.md) — experiment settings, measured results, verification, and limitations.
- [Phase 3 results](phase-3-results.md) — historical scalar learning exercise.
- [Architecture](architecture.md) — implemented CNN/baseline paths and planned camera pipeline.
- [Roadmap](roadmap.md) and [project plan](project-plan.md) — learning phases and targets.

The previously agreed Phase 4/5/6 targets were September 18/19/20, 2026. Phase 4 implementation was verified September 22 and documented September 23; Phase 5 closed out September 27. The recorded Phase 6 target was September 27 and has passed. Phase 6 was completed September 29 under the accepted audit scope; Phase 7 remains open. See Phase 6 results for evidence and limitations. The career-fair date remains October 1, 2026. Phase 9 is stretch work.

The learner writes the application code with mentorship. Minor assisted corrections and documentation support are part of that workflow. Next phases focus on dataset quality, reproducibility, stronger evaluation, and live camera integration.
