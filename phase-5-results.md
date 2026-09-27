# Phase 5 — CNN experiment results

Verified and documented September 27, 2026. Original target: September 19, 2026. Phase 5 is complete: a small CNN was implemented, explained, trained, compared with the baseline, saved/reloaded, and used for prediction without training. Improvement was measured, not assumed; this run did not outperform the linear baseline.

## Architecture and learning checks

GroceryCNN inherits from torch::nn::Module and registers convolution, pooling, and a final linear layer. N denotes batch size.

| Operation | Output shape | Trainable parameters |
| --- | --- | ---: |
| Input | [N, 3, 64, 64] | 0 |
| Conv2d(3,8,3), stride 1, padding 1 | [N, 8, 64, 64] | 216 weights + 8 biases |
| ReLU | [N, 8, 64, 64] | 0 |
| MaxPool2d(2), stride 2 | [N, 8, 32, 32] | 0 |
| flatten(1) | [N, 8192] | 0 |
| Linear(8192,3) | [N, 3] | 24576 weights + 3 biases |
| Total | | 24803 |

The constructor creates layers; forward processes images. Eight feature channels are a design choice. The final three scores represent avocado, banana, and lemon. Flattening happens after spatial feature extraction. The cnn-check command verified intermediate shapes, complete [2,3] output, registered parameter shapes, and 24,803 parameters. Random diagnostic scores are not quality measurements.

Mentorship covered filters, channels, shared weights, activations, pooling, receptive fields, tensor shapes, registration, logits versus loss, and parameter inspection. The developer wrote the CNN and pipeline changes, with an assisted diagnostic block and minor syntax corrections.

## Data and training

The same provisional splits and preprocessing as [Phase 4](phase-4-results.md) were used: 128 training, 16 validation, 125 test records; resized 64x64 RGB float32 in [0,1], owned contiguous CHW tensors. Labels are avocado 0, banana 1, lemon 2. Test records remain counted only, not evaluated.

CPU LibTorch training uses SGD at learning rate 0.001, batches of 16, 20 epochs, cross-entropy on raw logits, and shuffled indices each epoch: 160 updates. Both convolution and linear parameters are trained. No seed, augmentation, or best-checkpoint selection is configured. Epoch loss is the example-weighted average during updates, not a final fixed-model training evaluation.

## Measured results

Evidence: local ignored build-msvc/cnn-first-run.txt and verified terminal output. These values are transcribed so results remain available without committing build artifacts.

| Measure | First CNN run | Retained linear baseline |
| --- | ---: | ---: |
| First epoch training loss | 1.09001 | Not recorded for this retained run |
| Last epoch training loss | 0.937465 | 0.565349 |
| Validation loss | 0.973922 | 0.748435 |
| Validation accuracy | 8/16 (50%) | 10/16 (62.5%) |
| Always-banana reference | 6/16 (37.5%) | 6/16 (37.5%) |

The CNN fit training data but scored two fewer validation images correctly than the retained linear model. These are separate unseeded runs, not a repeated controlled study. Each validation image changes accuracy by 6.25 percentage points. This does not prove CNNs are generally worse or identify the cause of errors. More training or changed settings remain hypotheses, not verified improvements.

All eight mistakes were banana/lemon confusions:

- Banana_001.jpg, Banana_002.jpg, Banana_004.jpg, Banana_005.jpg, and Banana_006.jpg in val/Fruit/Banana were predicted lemon.
- Lemon_001.jpg, Lemon_002.jpg, and Lemon_004.jpg in val/Fruit/Lemon were predicted banana.

The subsequent Phase 3 scalar exercise and its tiny losses are separate from grocery metrics.

## Checkpoints and inference verification

- No-argument execution trains a fresh CNN and overwrites build-msvc/saved-cnn.pt, then runs the scalar exercise.
- The Phase 4 build-msvc/saved-model.pt remains preserved for the existing predict command.
- predict-cnn loads GroceryCNN state, uses shared preprocessing with an added batch axis, and exits before training.
- Checkpoints contain registered state, not architecture code, preprocessing, labels, or optimizer-resume state.

SHA-256 checked September 27, 2026:

```text
CNN:    359CA86D198E084D4897C7758F581F6AC7A75FC801ECC77298BBCF94146DD789
Linear: 29D1E87946F643E6F07A30C396CE1CA8611DF6F67A60C1B691341AD8DCC89A1E
```

Original and reloaded CNN scores matched with torch::allclose on identical training inputs. Prediction on val/Fruit/Banana/Banana_001.jpg printed `CNN predicted grocery: lemon`, matching the recorded mistake. This verifies consistent inference, not a correct prediction. Both checkpoint hashes stayed unchanged during prediction checks. Missing-image-argument usage output was checked; exhaustive CNN error-path testing was not performed.

MSVC Release build succeeded and CTest passed 1/1. That smoke test validates test infrastructure, not classifier accuracy. Existing LibTorch conversion warnings did not stop the build. Use an initialized x64 VS 2022 developer terminal; UCRT64 lacked the required MSVC header environment.

## Limitations and next work

Dataset auditing, related-capture grouping, split leakage checks, reproducibility, held-out test evaluation, camera input, and unknown-class rejection remain outstanding. Training error handling is basic. The application still resides in main.cpp. Transfer learning was not attempted.

Phase 6 will audit manifests, labels, unreadable images, duplicate/related captures, preprocessing/provenance, and establish reproducibility. If splits change, repeat the baseline on the finalized split. Phase 7 handles stronger metrics, selection, and held-out testing after tuning.

On September 27 the user targeted Phase 6 completion that day and starting Phase 7 afterward. This is a plan, not completion evidence. The career fair remains October 1, 2026.
