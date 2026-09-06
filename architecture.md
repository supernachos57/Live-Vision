# Planned architecture and timeline

Status: September 6, 2026. Only Phase 1 setup/testing is complete. All runtime components below are planned, not existing code.

## System and data flow

```mermaid
flowchart TD
    webcam["WebcamSource"] --> source["FrameSource<br/>getFrame()"]
    fallback["Image or recording"] --> source
    iphone["Future iPhone source"] -.-> source
    source --> frame["Captured frame"]
    frame --> prep["OpenCV preprocessing<br/>Resize and normalize"]
    prep --> model["Library-based classifier<br/>ML library TBD"]
    checkpoint["Saved checkpoint"] --> model
    model --> pred["Class and model score"]
    pred --> overlay["Live overlay"]
    frame --> overlay
    frame -.-> detprep["Phase 9<br/>Detector preprocessing"]
    detprep -.-> ort["ONNX Runtime<br/>Pretrained detector"]
    ort -.-> post["Boxes, labels, scores"]
    post -.-> detview["Detection comparison"]
    checkpoint -.-> export["Optional ONNX export"]
    export -.-> ortclass["Classifier comparison<br/>Output and latency"]
    prep -.-> ortclass
```

Solid edges describe the intended MVP. Dashed edges describe optional/future paths. FrameSource isolates capture from inference. Its planned getFrame() operation should represent either a frame or an explicit failure/end-of-input condition; decide the exact C++ signature in Phase 8.

WebcamSource is the first live adapter. A still-image or recorded adapter provides repeatable testing and a venue fallback. A possible iPhone source shares the interface but is outside the MVP commitment.

Keep the raw frame for overlays. Document the classifier's crop, input size, color order, value range, tensor layout, and label mapping. Use the same preprocessing contract for training and inference. Detector preprocessing is separate because a pretrained model may require different inputs. Postprocessing may include box decoding and suppression if required by that model.

The developer owns the C++ application, library-based model definition, training loop orchestration, evaluation, and integration. The ML library owns tensor storage/operations, layer implementations, autograd, and optimizer machinery. No handwritten Matrix or NN engine is planned.

## Training and saved-model handoff

```mermaid
flowchart TD
    data["Images and labels<br/>Source records"] --> split["Split manifests<br/>Train / validation / test"]
    split --> train["Training preprocessing<br/>Batches"]
    train --> loop["C++ training loop<br/>Library APIs"]
    loop --> ckpt["Checkpoint<br/>Config and label map"]
    split --> val["Validation<br/>Select model"]
    ckpt --> val
    val --> chosen["Selected model"]
    chosen --> test["Held-out test<br/>Metrics and errors"]
    chosen --> app["Phase 8<br/>Inference application"]
```

Validation guides selection; the held-out test set does not guide tuning. Learn gradient concepts with worked examples and the library's automatic differentiation, not a custom differentiation engine.

## Roadmap timeline

```mermaid
flowchart TD
    p1["Phase 1 - DONE<br/>Setup and testing"] --> p2["Phase 2 - Next<br/>Foundations and library<br/>Sep 6-9"]
    p2 --> p3["Phase 3<br/>C++ library exercise<br/>Sep 9-10"]
    p3 --> p4["Phase 4<br/>Grocery baseline<br/>Sep 11-15"]
    p4 --> p5["Phase 5<br/>CNN experiments<br/>Sep 16-18"]
    p4 --> p6["Phase 6<br/>Dataset engineering<br/>Sep 16-20"]
    p5 --> p7["Phase 7<br/>Model selection<br/>Sep 20-21"]
    p6 --> p7
    p7 --> p8["Phase 8<br/>Live camera MVP<br/>Sep 22-25"]
    p1 -.-> p10["Phase 10 - Parallel<br/>Docs and presentation<br/>Sep 6-30"]
    p8 --> release["Release ready<br/>Sep 30"]
    p10 --> release
    release --> fair["OCT 1 CAREER FAIR<br/>Phases 1-8 MVP cutoff"]
    fair --> p9["Phase 9 - Stretch<br/>ONNX comparison<br/>Oct 2-15 tentative"]
    fair -.-> polish["Additional polish<br/>Review Oct 31"]
    classDef completed fill:#dcfce7,stroke:#166534,color:#14532d
    classDef deadline fill:#fee2e2,stroke:#b91c1c,color:#7f1d1d,stroke-width:3px
    class p1 completed
    class fair deadline
```

All dates are 2026 planning targets. The vertical timeline keeps labels readable on narrow pages. Phase 1 is confirmed complete; its historical duration is not shown. Phase 2 is next but is not marked active before work starts. Phase 6 quality checks begin during Phase 4; its named window formalizes them. Phase 10 overlaps the learning work rather than depending on Phase 9.

Phase 8's internal target is September 25. **October 1 is the hard Phase 1–8 MVP cutoff**, leaving time for release preparation. Phase 9's October 15 date is tentative. October 31 is a review date for additional polish, not another phase or a required completion claim.

See [project plan](project-plan.md) for milestone definitions, exit criteria, and progress rules. Keep these dates synchronized when revising the plan.
