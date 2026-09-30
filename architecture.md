# Architecture and implementation status

Updated September 30, 2026. One executable and one repository; application code remains in src/main.cpp. The learner writes application code with mentorship. Camera capture is not implemented.

## Data and model

Upstream split lists -> strict selected-category records -> OpenCV preprocessing -> [N,3,64,64] tensors -> GroceryCNN -> logits -> cross-entropy/SGD during training, or argmax labels during prediction.

GroceryCNN: Conv2d(3,8,3), stride 1/padding 1 -> ReLU -> MaxPool2d(2), stride 2 -> flatten(1) -> Linear(8192,3); 24,803 parameters. The retained linear model is Linear(12288,3). Both prediction paths share loadImageTensor. See [dataset contract](docs/dataset.md).

## Commands

| Command | Behavior |
|---|---|
| No arguments / unknown command | Usage and exit 1; no training |
| train | Seeded training, per-epoch validation, best-loss checkpoint saving/restoration, reload check, metadata, then historical scalar exercise |
| preprocessing-check | Known-color/shape/scaling/layout and independent-loaded-tensor checks; writes test PNG |
| dataset-check | Validate selected upstream splits with a shared path map |
| dataset-check-list manifests... | Same checks on supplied manifests, including cross-manifest literal-path overlap |
| cnn-check | Random-input shape/parameter diagnostic; no checkpoint writes |
| predict-cnn image | Load retained Phase 5 saved-cnn.pt and predict |
| evaluate-cnn checkpoint val&#124;test | Load specified CNN and report split metrics without training or checkpoint writes |
| predict image | Load retained Phase 4 saved-model.pt and predict |

Run from repository root. Data image paths inside manifests are relative to data/GroceryStoreDataset/dataset. Training uses seed 42 before construction/shuffling and one CPU computation thread. Each epoch shuffles indices once; batches take slices. Training and validation remain separate; test records are counted by training but not passed through the model.

## Artifacts and limitations

Training currently writes build-msvc/phase7-run-b.pt plus a descriptive .metadata.txt sidecar. Registered model state is in the checkpoint; architecture/labels/preprocessing must still match source. Sidecar is not automatically consumed, and optimizer state is not saved. Historical checkpoints remain preserved; current predict-cnn still loads the Phase 5 file. evaluate-cnn accepts an explicit checkpoint. Phase 8 must explicitly load the selected Phase 7 checkpoint.

Dataset inventory, selected manifests and fingerprints live in docs/manifests. Training still reads upstream lists; snapshots are verified selections of those lists. Audit images are local artifacts, not committed dataset copies. No confirmed cross-split capture match was established by the bounded review, but session independence is not certified.

Known-color checks and 269-image checks pass. Two seeded logs match at printed precision; reload allclose passes. Phase 7 selected Run B epoch 19 by validation loss and scored 85/125 (68%) on held-out images. Standalone validation reproduces the selected metrics. Unknown-object rejection, camera input and localization remain unimplemented. The scalar exercise remains after training; training error handling is basic. General usage currently omits preprocessing-check even though the command works and is documented here.

See [Phase 6 results](phase-6-results.md) for evidence and the accepted bounded-audit scope and unverified session-independence limitation. See [Phase 7 results](phase-7-results.md) for completed evaluation. evaluateCNN reuses preprocessing and label mapping; metric reporting currently duplicates training reporting. The small evaluation split is loaded as one batch.
