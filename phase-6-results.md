# Phase 6 — Dataset engineering and reproducibility

Updated September 29, 2026. Phase 6 learning/engineering checkpoint complete under the scope explicitly accepted by the user on September 29: preserve upstream splits and accept the completed bounded audit with capture-session independence unverified. This revises the original stronger grouped-split completion requirement; it does not certify independent sessions. Git/PR closeout remains separate. Phase 7 reporting code already written is retained; model selection and held-out evaluation are not complete.

## Verified work

- Strict numeric category parsing, fine/coarse consistency checks, selected-class manifests and counts.
- Dataset commands validate images, tensor properties and exact repeated paths; 12 valid/invalid fixture cases behaved as expected.
- Known-color preprocessing check passed on September 29, including RGB/scaling and independent loaded tensors.
- All 269 images audited by file and decoded-RGB hashes; no exact duplicate groups. Thirty-six cross-split similarity candidates and all test thumbnails reviewed for data quality only. See [dataset audit](docs/dataset.md) for bounds and split decision.
- Explicit `train` command; no-argument execution now prints usage and exits 1 without training.
- Seed 42 before model initialization/shuffling, one CPU computation thread, SGD learning rate 0.001, batch 16, 20 epochs.
- Training uses `build-msvc/saved-cnn-phase6.pt`; retained Phase 4 and Phase 5 checkpoints remain unchanged.
- Metadata sidecar generated after successful reload parity: architecture, labels, preprocessing, expected data revision, counts and training settings. It is descriptive, not automatically loaded by inference.

## Reproducibility evidence

`phase6-run1.txt` and `phase6-run2.txt` are byte-identical local logs, SHA-256 `72F9BF2B49DF11CFD3B042BFE4DD83893BF2C6670BA47EA33661810A3589646E`. This establishes repetition at printed precision on this environment, not equality of every parameter or a cross-platform guarantee. Run 2 overwrote the Phase 6 checkpoint; no separate run-1 checkpoint was retained for a tensor-level comparison.

Both runs: training loss 1.11394 to 0.94188; validation loss 1.00202; 9/16 correct (56.25%); always-banana 6/16 (37.5%); reload allclose true. Later metadata and metric runs reproduced these reported values.

Checkpoint fingerprints are recorded in [lock.json](docs/manifests/lock.json). Retained CNN hash: `359CA86D198E084D4897C7758F581F6AC7A75FC801ECC77298BBCF94146DD789`; linear hash: `29D1E87946F643E6F07A30C396CE1CA8611DF6F67A60C1B691341AD8DCC89A1E`; current Phase 6 hash: `0202862964D2D1177EBC4FE4DF2343887B661422993E74163133FB3C624EACC5`.

The build was reported successful by the learner and the current executable ran the checks above. The GoogleTest executable passed its existing one smoke test directly on September 29. CTest was then invoked using its configured absolute path and passed 1/1. All three generated manifests passed dataset-check-list with 269 unique paths. Regenerating the audit reproduced all six manifest/inventory/candidate files byte for byte. This smoke test measures infrastructure, not classifier quality.

## Phase 7 reporting retained

The verified `phase7-metrics-run.txt` confusion matrix has actual rows and predicted columns in avocado, banana, lemon order:

| Actual / Predicted | Avocado | Banana | Lemon |
|---|---:|---:|---:|
| Avocado | 4 | 1 | 0 |
| Banana | 0 | 1 | 5 |
| Lemon | 0 | 1 | 4 |

Total 16, diagonal 9. Precision/recall: avocado 100%/80%, banana 33.3333%/16.6667%, lemon 44.4444%/80%. These are validation results on unchanged upstream membership, not final test results. The retained linear baseline scored 10/16; no improvement claim is made.

## Known limitations and closeout decision

Related validation images reduce independent scene diversity. No supplied session IDs permit a certified grouped split. Current snapshots preserve upstream membership and record the bounded audit. The user explicitly accepted this limitation and authorized Phase 6 completion on September 29, 2026. No classifier test evaluation has occurred.

Training still executes the historical scalar exercise afterward and overwrites its separate scalar files. Training exceptions are not comprehensively caught. `predict-cnn` still loads the retained Phase 5 checkpoint; it does not load the new Phase 6 checkpoint. No optimizer resume, automatic sidecar interpretation, or best-checkpoint selection exists. Shape/range checks alone do not establish semantic labels or full spatial correctness. Metadata's dataset revision is an expectation, not a runtime Git validation.

Original revised Phase 6 target: September 27, 2026; no replacement date is asserted. Career fair: October 1, 2026. No PR, milestone or board item was created or changed by this audit.
