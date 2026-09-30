# Live-Vision Roadmap

**Career-fair deadline: October 1, 2026**
**Current status (September 30, 2026): Phases 1–7 learning/engineering checkpoints complete; Phase 8 is next. Git closeout pending. Older schedules are historical.**
**Selected library/toolchain: CPU LibTorch with MSVC x64; OpenCV for image preprocessing**

## Project goal

Build a C++ computer-vision application that recognizes a small, clearly defined set of grocery items from images and a live camera feed. Deliver a reliable demonstration, measured model results, and a repository that explains the engineering and learning behind the project by October 1, 2026.

The learning goal is to **use existing ML libraries in C++, understand the underlying concepts, and write the application and ML pipeline code personally**. The chosen library supplies tensors, layers, automatic differentiation, optimizers, and other framework machinery. The developer writes model definitions through its APIs, data handling, training orchestration, evaluation, inference integration, and application behavior.

This roadmap replaces the earlier plan to build a handwritten ML engine. Implementing a tensor library, backpropagation engine, or framework internals is outside the project scope. Small worked examples can support understanding without becoming production implementations.

## Guiding principles

- **Learn through implementation.** Explain each concept, try a small example, then apply the library API in code you can explain and debug.
- **Keep C++ central.** Select a library and supported toolchain that let you learn model construction and training in C++, as well as inference. Any later tooling exception should be explicit and documented.
- **Build a complete small pipeline early.** Begin with a few grocery classes and expand only after data loading, training, evaluation, and prediction work together.
- **Separate classification from detection.** The core demo classifies one prominent item or a selected crop. Recognizing and locating multiple items belongs to the Phase 9 detection comparison.
- **Measure before claiming improvement.** Preserve a baseline, fixed data splits, experiment settings, and actual results.
- **Protect the deadline.** A dependable small demo and an honest explanation take priority over more classes, complex architectures, or optional runtime experiments.
- **Keep the developer in control.** Use mentorship to understand, implement, and review each step rather than copy a finished application without understanding it.

## The 10 revised phases

### Phase 1 — C++ engineering setup and testing

**Status: complete. Preserve this work.**

Completed in the prior session:

- Established the Windows/MSYS2 UCRT64 development workflow with C++20, CMake, Ninja, and g++.
- Established the project structure and `LiveVision` application target.
- Added GoogleTest through CMake FetchContent and the `LiveVisionTests` target.
- Enabled CTest registration and GoogleTest test discovery.
- Built and ran `SmokeTest.BasicEquality`: **1/1 tests passed**.
- Committed and pushed the testing work on `feature/google-test`.
- Completed the pull request, squash merge, and cleanup flow, as reported by the user.

**Preserved outcome:** a working build/test foundation and practiced Git workflow. The smoke test verifies the test infrastructure; future phases add tests for meaningful application behavior.

**Documentation follow-up:** carry forward the planned README/setup notes if they remain unwritten. Their completion was not confirmed in the conversation; this does not reopen the completed technical phase.

### Phase 2 — ML foundations and library decision

**Status: planned. Choose the library tomorrow, not in this roadmap.**

Learn features and labels, tensor shapes, batches, train/validation/test splits, predictions, loss, gradient descent, and overfitting. Work through a small numerical example to connect the concepts to library operations.

Tomorrow, compare candidates against C++ training APIs, documentation, Windows/compiler compatibility, installation effort, available hardware, model saving/loading, and eventual ONNX interchange. Verify support before committing to a toolchain; do not assume the current compiler can link every library distribution. GPU acceleration is optional for the initial experiment.

**Deliverable / exit check:** a short decision record and a minimal C++ program that links the selected library and performs a tensor operation. No library is selected by this document.

### Phase 3 — Learn the selected library in C++

Use the library's tensors, dataset/batching interfaces, model layers, forward calls, loss functions, automatic differentiation, and optimizer APIs. Learn training versus evaluation behavior and how to avoid tracking gradients during inference.

Write a small training program with a readable loop: load a batch, predict, compute loss, reset gradients, request gradient computation, and update parameters through the library. Practice saving and reloading a model.

**Deliverable / exit check:** a tiny reproducible learning exercise that trains successfully and produces consistent predictions after reload. Explain what gradients mean without implementing the differentiation engine.

### Phase 4 — Baseline grocery classifier

Choose a modest initial class set and a small, traceable dataset slice. Establish a provisional split immediately so even the baseline has a meaningful evaluation. Start with a simple library-defined classifier that is easy to train and inspect.

Write image-to-tensor conversion, label mapping, batching, configuration, training orchestration, and a command-line prediction path. Record the initial accuracy, training settings, and representative mistakes; compare against a simple majority-class reference.

**Deliverable / exit check:** an image can travel through loading, preprocessing, training, saved-model loading, and labeled prediction. Preserve this baseline for later comparisons.

### Phase 5 — CNNs and image classification

Learn convolution, channels, pooling, receptive fields, activations, and how tensor dimensions change through a network. Define a small CNN with the selected library's components. Consider transfer learning only if it fits the chosen tooling and schedule.

Compare the CNN with Phase 4 using the same provisional split and evaluation procedure. Keep architecture changes small enough to explain their purpose and cost.

**Deliverable / exit check:** a working CNN experiment with recorded results and an explanation of its behavior. An improvement is measured, not assumed.

### Phase 6 — Dataset engineering and reproducibility

**Completed September 29, 2026 under an explicit scope revision:** the user accepted unchanged upstream splits with the completed bounded audit and documented unverified session independence. This supersedes the stronger grouped-split completion requirement below; no certification of independent captures is claimed. See [Phase 6 results](phase-6-results.md).

Refine the initial data into a documented dataset. Inspect labels, corrupt images, duplicate or near-duplicate images, class counts, and background shortcuts. Record dataset sources, usage terms, and any redistribution restrictions before distributing data.

Group related captures of the same physical item or capture session into the same split to reduce leakage. Keep held-out test data untouched during tuning. Apply random augmentation only to training data; derive any learned normalization statistics from training data alone.

Define one preprocessing contract covering resize/crop policy, color order, value scaling, tensor layout, and label order. Record split manifests and random seeds. Repeat the baseline on the finalized split if the split changed.

**Deliverable / exit check:** reproducible manifests, documented preprocessing, and checks for invalid inputs, label mapping, shapes, and split overlap. Dataset quality work begins in Phase 4; this phase formalizes it.

### Phase 7 — Train, evaluate, and select the grocery model

Train on the finalized training set and use validation results for model and configuration selection. Track loss curves, accuracy, per-class precision/recall, and a confusion matrix. Inspect errors by class and capture conditions.

Use a small experiment budget. Record the data version, configuration, seed, model checkpoint, and hardware for each run. Evaluate the selected model on the held-out test set after tuning; report limitations and class support alongside aggregate scores.

**Deliverable / exit check:** a selected checkpoint, reproducible evaluation report, and known failure cases. Verify saved-model inference uses the same preprocessing and class order as training.

### Phase 8 — Live camera application and pipeline

Integrate camera capture and image handling with OpenCV. Write the capture, preprocessing, model invocation, prediction display, and shutdown behavior in C++. Organize those responsibilities so they can be checked independently.

Start with one item centered in the frame or a visible region of interest. Show the predicted class and model score with appropriate wording: a high score is not proof that an unfamiliar object belongs to a supported class. Add graceful handling for missing cameras, invalid model paths, and bad input.

Measure model inference time separately from capture-to-display latency and achieved frame rate on the demo machine. Add smoothing or an uncertainty threshold only if it improves observed behavior and is explained.

**Deliverable / exit check:** a repeatable live demo plus a still-image or recorded-input fallback. Set a practical responsiveness target after measuring the first working version.

### Phase 9 — Production, interchange, and object-detection comparison with ONNX Runtime

Keep this phase as a distinct **ONNX Runtime comparison**, while the selected ML library remains the primary learning/training tool.

1. **Interchange:** if supported by the selected tooling, export the grocery classifier to ONNX. Compare outputs on identical inputs with an explicit numerical tolerance. Record unsupported operations or export limitations.
2. **Production inference:** write a C++ ONNX Runtime inference path. Compare startup/model-load time, inference latency, end-to-end latency, memory where measurable, and integration complexity under matched conditions.
3. **Object detection:** integrate a suitable pretrained ONNX detector as a separate experiment. Learn its input/output contract, bounding boxes, class mapping, and required postprocessing. Explain how localization and multiple-object predictions differ from the grocery classifier.

Do not assume a pretrained detector recognizes the chosen grocery categories. Inspect its labels before defining the demo. Keep detector results separate from classifier accuracy; compare task coverage and engineering tradeoffs honestly.

**Deliverable / exit check:** a comparison note and, if time permits, a runnable alternate inference/detection mode. If classifier export is unsupported, document that limitation and scope the runtime experiment to a compatible model rather than claiming equivalence.

**Deadline priority:** stretch work before October 1; continue afterward if it would endanger the core demo. Phase 9 remains on the roadmap even when deferred.

### Phase 10 — Career-fair release and presentation

Begin documentation and demo preparation throughout the project; do not wait for Phase 9. Freeze features, verify setup from documented instructions, rehearse on the actual machine, and fix reliability problems.

Prepare a short demo recording, screenshots, a concise architecture explanation, measured results, limitations, and a two-minute project walkthrough. Explain which components you wrote and which capabilities the libraries provide. Create a release tag once the intended demo revision is verified.

**Deliverable / exit check:** a presentable repository and tested demo package ready **before October 1, 2026**, with an offline fallback and a clear explanation of the work.

## Milestones and priorities before October 1

These are proposed planning targets, not completed work or guarantees. Learning phases may overlap; release preparation proceeds even if Phase 9 is deferred.

| Target | Priority | Evidence of completion |
| --- | --- | --- |
| Tomorrow's session | Required | Choose and validate the ML library/toolchain; begin Phase 2 |
| By September 10 | Required | Foundations exercise and a working C++ library training example |
| By September 15 | Required | Small grocery baseline works from data through saved-model prediction |
| By September 21 | Required | Finalized data split, CNN experiment, selected model, and evaluation report |
| By September 25 | Required | Camera demo works on the intended machine; fallback input works |
| September 26–28 | Required | Reliability fixes, README, results, recording, and presentation rehearsal |
| By September 30 | Required | Freeze and verify the career-fair release |
| Before October 1 only if core work is secure | Stretch | Phase 9 ONNX Runtime/interchange/detection comparison |
| October 1, 2026 | Fixed deadline | Demonstrate the verified release and explain its limitations |

**Minimum career-fair scope:** a small supported class set, a trained library-based C++ classifier, honest held-out results, a live single-item demo, a reliable fallback, and usable documentation.

If progress slips, reduce the class count, model size, and experiment count first. Defer GPU tuning, dataset expansion, and Phase 9 implementation before cutting core reliability, evaluation, or presentation work. Keep all ten phases as the longer-term learning plan.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Library/compiler integration consumes the schedule | Validate a minimal linked program tomorrow before building the pipeline; document any deliberate toolchain change and rerun Phase 1 checks afterward. |
| Training exceeds available compute | Start with a small dataset/model, measure run time early, and limit experiments. |
| Leakage or background cues inflate accuracy | Group related captures, inspect duplicates and mistakes, and evaluate on held-out conditions. |
| Training images differ from webcam images | Collect a small representative camera evaluation set and test lighting/background changes early. |
| Training and inference preprocessing diverge | Share or explicitly verify the preprocessing contract and label mapping. |
| New classes and detection expand the scope | Keep the minimum demo narrow and treat Phase 9 as optional before the deadline. |
| Camera or venue conditions break the demo | Rehearse on the actual hardware and carry a local recording and still-image fallback. |
| Framework use becomes unexplained copying | Keep short learning notes and explain each API's role, inputs, outputs, and failure modes. |

## Documentation and Git workflow

Maintain these documents as the project develops:

- `README.md`: goal, current status, supported demo behavior, dependencies, build/test/run instructions, results, and limitations.
- `docs/phase-1-setup.md`: environment, CMake/Ninja/compiler roles, GoogleTest versus CTest, and setup lessons.
- `docs/ml-library-decision.md`: tomorrow's choice, compatibility checks, tradeoffs, and pinned versions.
- `docs/dataset.md`: sources, usage terms, class definitions, split policy, manifests, and preprocessing.
- `docs/experiments.md`: configurations, metrics, checkpoints, failures, and reasons for decisions.
- `docs/architecture.md`: capture → preprocessing → model → postprocessing → display, with component responsibilities.
- `docs/onnx-comparison.md`: Phase 9 scope, matched test conditions, results, and limitations when attempted.

Use one focused branch per coherent change. Inspect changes, stage intended files, commit with a descriptive message, push, open a pull request, and review the diff. Include the concrete behavior change and actual validation in the PR description. Squash merge after review, update local `main`, and clean up the merged branch. Tag meaningful verified milestones.

Retain the established configure/build/test loop:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Document any new library-specific configuration arguments. Add useful tests around data validation, preprocessing, label mapping, model loading, and application error handling; model quality is evaluated with datasets and metrics, not the smoke test alone.

Keep generated builds, large datasets, model checkpoints, and secrets out of ordinary source commits. Document how to obtain the exact data/model artifacts required to reproduce the results, with versions or hashes where practical.

## Original next-session note (historical)

Choose the ML library together, validate its C++ integration, and begin the Phase 2 foundations exercise. The decision remains open until that session. Phase 1 stays complete, and the October 1, 2026 deadline stays fixed.

## Phase 4 closeout — September 23, 2026

The status above supersedes the original planning language in the phase descriptions. Phases 2 and 3 were completed using CPU LibTorch and MSVC; Phase 4 now provides a verified three-class grocery baseline with separate data lists, RGB image preprocessing, training, validation, checkpoint parity, and prediction-only command-line use. See [Phase 4 results](phase-4-results.md) and the [current build/run instructions](README.md).

The previously agreed conversation targets were Phase 4 September 18, Phase 5 September 19, and Phase 6 September 20, 2026. These replace the older targets for those phases in the original plan. Phase 4 implementation was verified September 22 and documentation completed September 23. Later targets have passed; no new dates have been agreed. Historical UCRT64 commands above describe Phase 1 and are not the current LibTorch build instructions.

Next: Phase 5 CNN learning and comparison, retaining the baseline. Dataset auditing, live camera work, and release preparation are still outstanding. The October 1 career-fair date has not changed.

## Phase 5 closeout and revised plan — September 27, 2026

Phases 1–5 checkpoints are complete. The small CNN trained on the same provisional split, achieved 8/16 validation accuracy versus the retained linear baseline's 10/16, passed checkpoint score parity, and predicted from saved state without training. See [Phase 5 results](phase-5-results.md). CNN training now saves saved-cnn.pt; saved-model.pt remains the linear baseline.

The user explicitly revised the Phase 6 completion target to September 27, 2026 (today in that session), with Phase 7 to start afterward. This replaces Phase 6's historical September 20 target. No Phase 7 completion deadline was agreed. Phase 6 dataset auditing/reproducibility and Phase 7 evaluation/selection remain outstanding; planned dates are not completion evidence. The career fair remains October 1, 2026.

This update supersedes older current-status and no-replacement-date statements below/above. Historical schedules are retained as history. No GitHub milestones, issues, or project-board status were changed.

## September 29, 2026 — Phase 6 audit and reproducibility evidence

This entry supersedes older current-status text. Dataset/preprocessing commands and 12 fixture cases have been verified. Seeded runs repeat printed logs; checkpoint metadata is generated; selected manifests and image fingerprints are recorded. Initial Phase 7 confusion-matrix and per-class reporting remain in the branch, but selection and held-out evaluation have not occurred.

The bounded audit found no repeated paths, byte-identical files, or identical decoded RGB images among 269 selected records. All test thumbnails and 36 cross-split similarity candidates were reviewed for data quality only. Upstream split membership is retained. Capture-session independence remains unverified because session identifiers are absent; visual groups are recorded as inferences. The user subsequently accepted this limitation on September 29 and authorized Phase 6 completion under the bounded-audit scope. This explicitly revises the original stronger grouped-split criterion; session independence is not claimed. See [Phase 6 results](phase-6-results.md) and [dataset audit](docs/dataset.md).

The September 27 Phase 6 target has passed; no replacement date or Phase 7 completion deadline is invented. The career-fair date remains October 1, 2026. No GitHub milestone or project board was changed.

## September 30, 2026 — Phase 7 closeout
Phases 1–7 learning/engineering checkpoints are complete; the user reviewed and accepted the assisted implementation. Git commit/PR/merge remains pending.

The controlled comparison changed only SGD learning rate from 0.001 (A) to 0.003 (B), keeping seed 42, split, preprocessing, CNN, batch size 16 and 20 epochs fixed. Lowest validation loss selected Run B epoch 19 (0.792704, 9/16 correct). Subsequent held-out testing scored 85/125 (68%), loss 0.68481. Reload parity passed; standalone validation reproduced results and test evaluation left the checkpoint unchanged. See [Phase 7 report](phase-7-results.md).

This selects between two CNN configurations, not proof of superiority over the historical linear baseline. Session independence remains unverified. No test-based tuning occurred. Next is Phase 8 camera capture, matching preprocessing, selected-checkpoint loading, display, fallback/error handling, and performance measurement.

No revised Phase 7 or Phase 8 due date was agreed. The career fair remains October 1, 2026. Older schedules are historical. No GitHub milestone or board fields were changed.