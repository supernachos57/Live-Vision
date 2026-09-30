# Dataset contract and bounded audit

Audit date: September 29, 2026. Dataset membership is unchanged. This documents a completed bounded inventory/content audit, not certification of capture-session-independent splits.

## Provenance and usage

Source: Marcus Klasson, Cheng Zhang, and Hedvig Kjellstrom, GroceryStoreDataset, associated with *A Hierarchical Grocery Store Image Dataset with Visual and Semantic Labels*, WACV 2019. Repository: https://github.com/marcusklasson/GroceryStoreDataset . Verified local revision: `fc80ba90f803d79d0383df52c5a4ac5de99ff6fc`; local dataset Git status was clean.

The upstream README describes smartphone images taken in grocery stores and asks researchers to cite the paper. The local repository contains an MIT LICENSE, copyright 2019 Marcus Klasson, whose text refers to Software and associated documentation. This note does not extend that wording into a separate image-rights guarantee. Obtain images from upstream; this project commits paths, labels, hashes and audit notes, not dataset photos. Preserve upstream attribution and consult upstream terms for any redistribution.

## Selected classes

| Class | Fine ID | Coarse ID | Model label | Train | Validation | Test |
|---|---:|---:|---:|---:|---:|---:|
| Avocado | 5 | 1 | 0 | 41 | 5 | 40 |
| Banana | 6 | 2 | 1 | 45 | 6 | 44 |
| Lemon | 8 | 4 | 2 | 42 | 5 | 41 |
| Total | | | | 128 | 16 | 125 |

Images depict class-level scenes, frequently entire displays, rather than annotated individual object boxes. Generic fruit identity was reviewed on contact sheets; this is not an expert relabeling exercise. Store backgrounds and correlated captures can influence results. Webcam single-item scenes may differ substantially.

## Manifests and identity

`manifests/train.txt`, `val.txt`, and `test.txt` preserve the selected upstream rows, their order and their original split. Format: `relative_image_path, fine_id, coarse_id`. Paths are relative to `data/GroceryStoreDataset/dataset/`, not to the manifest directory. These are snapshots of the current selection; the application still reads and filters the upstream lists during training.

`inventory.json` records all 269 selected files with labels, original dimensions, SHA-256 of file bytes and SHA-256 of decoded RGB dimensions plus bytes. `summary.json` records upstream manifest hashes and exact-overlap results. `lock.json` fingerprints selected manifests, inventory, source and retained checkpoints. `capture-observations.json` records the observed validation group candidates without inventing session IDs for every image.

Upstream files and generated selected manifests can have different byte hashes because they contain different rows/formatting. Compare logical rows to check selection identity. A hash detects change; it does not prove correct labels or scene independence.

## Verified checks

- Current C++ `dataset-check`: all 269 selected images decoded and passed [3,64,64], float32, contiguous, finite [0,1] checks; 269 unique literal paths.
- Full selected subset: zero repeated paths, zero duplicate file SHA-256 groups, zero identical decoded-RGB groups, both within and across splits.
- Twelve manifest fixtures checked successful input, invalid numeric suffix/negative IDs, fine/coarse mismatch in both directions, empty path, missing field, extra field, repeated path within/across manifests, missing image and empty selection. Valid case exited 0; invalid cases exited 1 with expected messages.
- Known-color C++ preprocessing check passed: synthetic 7x11 BGR (0,128,255) PNG becomes [3,64,64] float32 RGB (1,128/255,0); black reload and independent loaded-tensor checks passed. Uniform input does not independently establish every spatial interpolation property.

## Related-capture review and split decision

All selected training/validation contact sheets were reviewed with the learner, followed by all 125 test thumbnails for integrity/grouping only. During the Phase 6 audit, no classifier was run on test images and no hyperparameters were chosen from test predictions.

For each class and each split pairing (train/val, train/test, val/test), a support script ranked all image pairs using resized 32x32 RGB mean squared difference and a 64-bit grayscale difference hash. The top two pairs per heuristic were selected, deduplicated, yielding 36 candidate pairs for visual review. These heuristics nominate candidates only; there is no validated threshold and rotations/crops/re-encodings can be missed. Exact decoded-RGB comparison used Pillow, distinct from the application's OpenCV decoder.

No reviewed candidate established a convincing shared physical capture across splits. Similar fruit colors and common retail fixtures were not accepted as proof. This does NOT establish that no related cross-split images exist. The supplied lists/classes table contains no store/session identifiers, and the visual review is bounded.

Validation has strong within-split related-display candidates: avocado 001–005, banana 001–006 (particularly 001/004/005), and lemon 001–005. These are visual inferences, not authoritative capture-session labels. Candidate groups already remain within validation. Repeated scenes also occur within training and test. Sixteen validation images should not be described as sixteen independent capture conditions.

Decision for the current experiment: retain upstream membership and ordering, keep identified validation candidates together, and record the uncertainty. No reassignment or image deletion is justified by the reviewed evidence. Because splits have not changed, a baseline rerun is not required solely for this audit; controlled comparisons were subsequently completed in Phase 7.

On September 29, 2026, the user explicitly accepted this bounded audit and unchanged upstream splits as the Phase 6 completion scope. This supersedes the stronger original requirement for verified capture-group-independent splits. Independence remains unverified and must be stated alongside evaluation results. No photos were moved and no model test scores were used to make this decision. Further camera-condition testing belongs to later phases.

## Preprocessing contract

OpenCV color decode (BGR), direct resize to 64x64 with INTER_AREA, BGR-to-RGB conversion, cloned owned pixel tensor, float32 divide by 255, HWC-to-CHW permutation and contiguous layout. Single image: [3,64,64]; CNN batch: [N,3,64,64]. Linear inference flattens to [N,12288]. No augmentation or learned normalization statistics. Training-only statistics/augmentation would be required if added later.

## Reproduce checks

From the project root in an initialized VS 2022 x64 developer terminal with OpenCV on PATH:

```bat
cmake --build build-msvc --target LiveVision LiveVisionTests
build-msvc\LiveVision.exe preprocessing-check
build-msvc\LiveVision.exe dataset-check
build-msvc\LiveVision.exe dataset-check-list docs/manifests/train.txt docs/manifests/val.txt docs/manifests/test.txt
ctest --test-dir build-msvc --output-on-failure
```

The optional Python support audit uses Pillow and NumPy; it is not an application runtime dependency. With those available, regenerate to a scratch directory and compare JSON/manifests:

```text
python docs/audit_dataset.py --output build-msvc/dataset-audit
```

Review generated candidate sheets manually; the script does not decide capture identity or change splits. Image sheets remain local review artifacts. Never tune against held-out model scores during this audit.

## September 30 evaluation update
The selected CNN was evaluated on the unchanged test split after validation-based selection: 85/125 correct (68%). Test results did not guide tuning. Earlier audit statements describe Phase 6; session independence remains unverified. See [Phase 7 results](../phase-7-results.md).
