[architecture.md](https://github.com/user-attachments/files/31886768/architecture.md)
# Planned architecture and timeline

Status: September 6, 2026. Only Phase 1 setup/testing is complete. All runtime components below are planned, not existing code.

## System and data flow

```mermaid
flowchart TD
    webcam["WebcamSource - planned MVP"] --> source["FrameSource interface: getFrame()"]
    fallback["Still image or recorded input - planned fallback"] --> source
    iphone["Possible iPhoneSource - future"] -.-> source
    source --> frame["Captured frame"]
    frame --> prep["OpenCV preprocessing: crop / resize / color / normalize"]
    prep --> model["C++ classifier defined with selected ML library - library TBD"]
    checkpoint["Saved grocery-model checkpoint"] --> model
    model --> pred["Class label + model score"]
    pred --> overlay["Application overlay and display"]
    frame --> overlay
    frame -.-> detprep["Phase 9: detector-specific preprocessing"]
    detprep -.-> ort["ONNX Runtime + pretrained detection model"]
    ort -.-> post["Detector postprocessing: boxes / labels / scores"]
    post -.-> detview["Separate detection overlay and comparison"]
    frame -.-> detview
    checkpoint -.-> export["Optional ONNX export if supported"]
    export -.-> ortclass["ONNX Runtime classifier parity / latency comparison"]
    prep -.-> ortclass
```

Solid edges describe the intended MVP. Dashed edges describe optional/future paths. FrameSource isolates capture from inference. Its planned getFrame() operation should represent either a frame or an explicit failure/end-of-input condition; decide the exact C++ signature in Phase 8.

WebcamSource is the first live adapter. A still-image or recorded adapter provides repeatable testing and a venue fallback. A possible iPhone source shares the interface but is outside the MVP commitment.

Keep the raw frame for overlays. Document the classifier's crop, input size, color order, value range, tensor layout, and label mapping. Use the same preprocessing contract for training and inference. Detector preprocessing is separate because a pretrained model may require different inputs. Postprocessing may include box decoding and suppression if required by that model.

The developer owns the C++ application, library-based model definition, training loop orchestration, evaluation, and integration. The ML library owns tensor storage/operations, layer implementations, autograd, and optimizer machinery. No handwritten Matrix or NN engine is planned.

## Training and saved-model handoff

```mermaid
flowchart LR
    data["Images + labels + source records"] --> split["Grouped train / validation / test manifests"]
    split --> train["Training preprocessing and batches"]
    train --> loop["Developer-written C++ training loop using library APIs"]
    loop --> ckpt["Checkpoint + configuration + label map"]
    split --> val["Validation: select model"]
    ckpt --> val
    val --> chosen["Selected model"]
    chosen --> test["Held-out test: final metrics and error analysis"]
    chosen --> app["Phase 8 inference application"]
```

Validation guides selection; the held-out test set does not guide tuning. Learn gradient concepts with worked examples and the library's automatic differentiation, not a custom differentiation engine.

## Roadmap timeline

```mermaid
gantt
    title Live-Vision targets - planned work, not completion claims
    dateFormat YYYY-MM-DD
    axisFormat %b %d
    section Completed
    Phase 1 setup confirmed done :done, p1, 2026-09-05, 1d
    section Core learning and MVP
    Phase 2 foundations and library decision :p2, 2026-09-06, 2026-09-09
    Phase 3 C++ library exercise :p3, 2026-09-09, 2026-09-10
    Phase 4 grocery baseline :p4, 2026-09-11, 2026-09-15
    Phase 5 CNN experiments :p5, 2026-09-16, 2026-09-18
    Phase 6 dataset engineering :p6, 2026-09-16, 2026-09-20
    Phase 7 model selection :p7, 2026-09-20, 2026-09-21
    Phase 8 live camera MVP :p8, 2026-09-22, 2026-09-25
    Phase 1-8 MVP cutoff - career fair :crit, milestone, fair, 2026-10-01, 0d
    section Release work overlaps learning
    Phase 10 docs and presentation :p10, 2026-09-06, 2026-09-30
    Verified release ready :milestone, release, 2026-09-30, 0d
    section After the fair
    Phase 9 ONNX comparison - tentative :p9, 2026-10-02, 2026-10-15
    Additional polish - review date only :polish, 2026-10-02, 2026-10-31
```

Dates are planning targets in America/New_York. The Phase 1 bar records confirmation of completion, not its historical duration. Phase 2 is next but is not marked active before work starts. Phase 6 quality checks begin during Phase 4; its named window formalizes them. Phase 10 overlaps the learning work rather than depending on Phase 9.

Phase 8's internal target is September 25. **October 1 is the hard Phase 1–8 MVP cutoff**, leaving time for release preparation. Phase 9's October 15 date is tentative. October 31 is a review date for additional polish, not another phase or a required completion claim.

See [project plan](project-plan.md) for milestone definitions, exit criteria, and progress rules. Keep these dates synchronized when revising the plan.
