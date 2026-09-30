# Phase 7 — CNN selection and held-out evaluation
Date: September 30, 2026.

## Selection fixed before testing
Two fresh seeded CNN runs used identical architecture, preprocessing, split, SGD, seed 42, one CPU thread, batch size 16 and 20 epochs. Only learning rate changed. Each run retained its lowest-validation-loss epoch. Run B was selected by lower validation loss before test evaluation.

| Run | Learning rate | Selected epoch | Validation loss | Validation correct | Reload parity |
| --- | ---: | ---: | ---: | ---: | --- |
| A | 0.001 | 20 | 1.00202 | 9/16 | passed |
| B | 0.003 | 19 | 0.792704 | 9/16 | passed |

Run A reproduced the previous result. Run B epoch 20 had loss 0.827813, so epoch 19 was restored. Higher accuracy at other epochs did not change the predefined loss-based rule. Both checkpoints and logs remain separately in build-msvc/phase7-run-a.* and phase7-run-b.*.

Selected checkpoint: build-msvc/phase7-run-b.pt
SHA256: C03EBD305C8A5C531E1588BDF556425F5E2C6E8B9B580483D51FDB4A1876D902

This selects between these two CNN runs. The historical unseeded linear baseline had validation loss 0.748435 and accuracy 10/16; CNN superiority over that baseline is not established.

## Evaluation command
From the repository root, with the documented OpenCV DLL folder on PATH:

    build-msvc/LiveVision.exe evaluate-cnn build-msvc/phase7-run-b.pt test

The command constructs GroceryCNN, loads the specified archive, sets evaluation mode, disables gradient tracking, and uses loadImageTensor and mapCategory from training. Labels remain avocado, banana, lemon. There is no optimizer, training, or checkpoint write in this path. Evaluation batches the small split in memory.

## Held-out results
Selected CNN: 85/125 correct (68%), cross-entropy loss 0.68481.
Always-banana reference: 44/125 correct (35.2%).

Rows are actual classes; columns are predicted classes.

| Actual / predicted | Avocado | Banana | Lemon |
| --- | ---: | ---: | ---: |
| Avocado | 40 | 0 | 0 |
| Banana | 4 | 19 | 21 |
| Lemon | 0 | 15 | 26 |

| Class | Support | Precision | Recall |
| --- | ---: | ---: | ---: |
| Avocado | 40 | 90.91% | 100% |
| Banana | 44 | 55.88% | 43.18% |
| Lemon | 41 | 55.32% | 63.41% |

Forty mistakes: 21 bananas predicted lemon, 15 lemons predicted banana, and four bananas predicted avocado. These are observed label confusions; visual causes such as lighting/background were not established.

## Verification
Release build succeeded. Standalone validation evaluation reproduced Run B loss, accuracy, and confusion matrix. Both training runs passed saved-score reload parity. Test matrix totals 125, diagonal totals 85, and class supports total 125. Checkpoint SHA256 was unchanged by evaluation. Invalid split and missing checkpoint return exit code 1.

Full evaluation output: build-msvc/phase7-test.log. Training metadata's test_evaluated=false describes the time of training; this separate report records the subsequent test evaluation.

## Limits and next steps
Only 16 validation images and one seed were used. Capture-session independence remains unverified under the accepted Phase 6 bounded audit. Test images were previously inspected for data quality, but classifier test results were not used for selection. Test accuracy exceeding validation accuracy does not mean the model improved during testing: the images differ and the weights did not change.

No configuration was changed in response to test results. This is a three-class still-image CNN result, not live-camera validation or unknown-object rejection. Existing predict-cnn still loads the historical saved-cnn.pt; Phase 8 must explicitly load the selected phase7-run-b.pt checkpoint. Source training settings currently retain Run B; rerunning train overwrites Run B's experiment artifacts.

Technical evaluation, reporting and learner review are complete. The user accepted the assisted code changes. Git commit, PR review and merge remain pending.