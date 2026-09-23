# Phase 4 — Grocery baseline results

Implementation verified September 22, 2026; documentation closeout September 23, 2026. Previously agreed target: September 18, 2026. This is a simple still-image baseline, not the finished Live-Vision camera application.

## Verified exit checks

- Separate filtered image records for training, validation, and test lists.
- JPEG loading through OpenCV and shared image-to-tensor preprocessing.
- Three-class registered LibTorch Linear layer, batch forward pass, and training.
- Validation loss/accuracy, majority-class comparison, and mistake reporting.
- Grocery model state saved and reloaded with matching scores.
- Command-line prediction loads a checkpoint and exits before training.
- Missing-image and invalid-argument prediction paths return failure.

## Data and preprocessing

Source: Marcus Klasson, Cheng Zhang, and Hedvig Kjellstrom, [GroceryStoreDataset](https://github.com/marcusklasson/GroceryStoreDataset), associated with *A Hierarchical Grocery Store Image Dataset with Visual and Semantic Labels* (WACV 2019). The local clone is revision `fc80ba90f803d79d0383df52c5a4ac5de99ff6fc`. Follow the upstream repository for dataset use/citation information; this project does not redistribute its images or claim additional redistribution rights.

Each upstream list line contains image path, specific category ID, and broader category ID. Filtering uses the broader ID; all other IDs map to -1 and are skipped. Image paths are relative to `data/GroceryStoreDataset/dataset/`.

| Grocery | Broader dataset ID | Model label | Training | Validation |
| --- | ---: | ---: | ---: | ---: |
| Avocado | 1 | 0 | 41 | 5 |
| Banana | 2 | 1 | 45 | 6 |
| Lemon | 4 | 2 | 42 | 5 |
| Total | | | 128 | 16 |

The separate test list contains 125 selected records. It has only been read and counted, not evaluated or used for parameter updates. These are provisional upstream splits; duplicate checks, capture-session grouping, and leakage auditing have not been completed. Those are Phase 6 work.

The preprocessing contract, shared by training and prediction, is:

1. Decode a three-channel BGR image with OpenCV; reject an empty result.
2. Directly resize to 64 by 64 using INTER_AREA. Aspect ratio is not preserved by padding or cropping.
3. Convert BGR to RGB.
4. Create a uint8 tensor view over the image memory, then clone it to own the pixels.
5. Convert to float32 and divide by 255, giving values in [0, 1]. No learned mean/std normalization or augmentation is used.
6. Permute HWC to CHW and make contiguous: [3, 64, 64].
7. Stack images into [N, 3, 64, 64], then flatten from axis 1 to [N, 12288]. For one prediction, unsqueeze adds the batch axis first.

## Model and training settings

- CPU LibTorch Linear(12288, 3), registered as `classifier`.
- Weight shape [3, 12288], bias shape [3]; 36,867 trainable scalar parameters.
- Raw class scores (logits), with labels ordered avocado, banana, lemon.
- Cross-entropy receives raw scores and int64 target labels.
- SGD learning rate 0.001; default zero momentum and weight decay.
- 20 epochs, batch size 16: 8 updates per epoch, 160 updates per run.
- All 128 processed training images are loaded into memory before training.
- randperm generates a new shuffled ordering each epoch; the same indexes select images and labels.
- Printed epoch loss is the example-weighted mean of batch losses encountered while parameters change, not a post-epoch evaluation of a fixed model.
- No explicit random seed. Initialization and shuffling vary across runs; exact reproduction of reported metrics is not promised.

## Recorded observations

The following are separate runs observed during mentorship, not an average or a controlled comparison:

| Run/check | Last epoch training loss | Validation loss | Validation accuracy |
| --- | ---: | ---: | ---: |
| Initial validation verification | 0.542213 | 0.755819 | 9/16 (56.25%) |
| Baseline and mistake-report verification | 0.546143 | 0.731351 | 9/16 (56.25%) |
| Save/reload verification; retained checkpoint | 0.565349 | 0.748435 | 10/16 (62.5%) |

An earlier training-only run decreased average loss from 1.07723 in its first epoch to 0.531793 in its twentieth. This demonstrates fitting training data, not generalization by itself.

The majority class in training is banana (45/128). Always predicting banana is correct on 6/16 validation images: **37.5%**. The retained checkpoint's observed accuracy exceeds this reference by four images, but the validation set is only 16 images: one prediction changes accuracy by 6.25 percentage points. No test accuracy has been measured.

### Representative mistakes

In the 9/16 mistake-report run:

- Banana_001.jpg, Banana_002.jpg, and Banana_005.jpg in val/Fruit/Banana were predicted as lemon.
- Lemon_001.jpg through Lemon_004.jpg in val/Fruit/Lemon were predicted as banana.

Banana_001.jpg and Lemon_001.jpg were visually inspected. Both contain multiple yellow fruits in store displays, with background and arrangement differences. Similar color and the limitations of a flattened linear model are hypotheses to investigate, not proven causal explanations. The images depict category-level scenes; the program does not locate individual objects.

The retained checkpoint later predicted Banana_001.jpg as banana. That does not contradict the earlier mistake list: it came from another random training run.

## Checkpoint and inference verification

State is stored in ignored `build-msvc/saved-model.pt` using OutputArchive and loaded into the same model architecture using InputArchive. It contains registered model state, not the C++ implementation, preprocessing contract, label names, or optimizer state. The current code relies on those being specified consistently in source.

The September 22 retained checkpoint SHA-256, checked again September 23:

```text
29D1E87946F643E6F07A30C396CE1CA8611DF6F67A60C1B691341AD8DCC89A1E
```

Verified behavior:

- `torch::allclose` reports true between original and reloaded model scores on identical inputs.
- Prediction-only mode on val/Fruit/Banana/Banana_001.jpg prints banana and returns 0, without epoch output.
- An incomplete predict command prints usage and returns 1.
- A nonexistent image prints an error and returns 1.
- The checkpoint hash remains unchanged across prediction checks.
- MSVC Release build succeeds; existing CTest smoke test passes 1/1 (September 23). It validates test plumbing, not classifier correctness.

The build emits existing conversion warnings from LibTorch headers. Configuration previously emitted a nonfatal missing-kineto warning. Neither prevented the verified build/run.

## Known limitations and next phase

There is no CNN, camera input, object detection, unknown-category rejection, calibration, data augmentation, fixed seed, best-checkpoint selection, or full dataset-quality audit. Prediction chooses one of the supported labels even for an unsupported object. Training-mode file/parse exceptions are not caught at the top level; the list parser only has basic missing-field checks. Robust parsing and general input hardening remain future work.

No-argument execution retrains and overwrites the checkpoint. It also runs the old scalar Phase 3 exercise and prints large diagnostic tensors; those outputs must not be confused with grocery metrics. Future cleanup can separate exercises and configuration after the learner understands the changes.

Phase 5 will compare a CNN against this baseline using the same provisional split and preprocessing/evaluation contract, with changes explicitly recorded. Phase 6 will formalize dataset quality and reproducibility. Preserve this baseline and its limitations when making comparisons.
