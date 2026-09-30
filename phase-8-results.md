# Phase 8 — live camera demo results
September 30, 2026. Career fair: October 1, 2026.

## Implemented and verified
OpenCV camera capture with index selection; shared image/frame preprocessing; CPU GroceryCNN inference; predicted class/model-score overlay; optional uncertain display below a score threshold; clean-shutdown paths; and timing printed after normal shutdown. Camera and predict-cnn both load build-msvc/phase7-run-b.pt. No retraining occurred. Checkpoint SHA256 verified unchanged: C03EBD305C8A5C531E1588BDF556425F5E2C6E8B9B580483D51FDB4A1876D902.

Release build passed after timing additions. Shared preprocessing checks passed. Invalid arguments/thresholds reject with exit 1. Camera-open/frame-read failures report errors. Still-image fallback predicted avocado and returned 0; missing image returned 1. A competing camera process was stopped before the successful live retry; the precise cause of the earlier capture failure was not conclusively established.

## Launch on the verified machine
PowerShell:

```powershell
cd C:\Users\ryanw\source\programming_projects\Live-Vision
$env:PATH = "C:\Users\ryanw\Libraries\opencv\build\x64\vc16\bin;$env:PATH"
.\build-msvc\LiveVision.exe camera-check 2 0.90
```

EMEET SmartCam worked at index 2; numbering can change after reconnection. Close other camera applications and run only one copy. Keep one item prominent against a plain, well-lit background. The whole frame is directly resized to 64x64; no detection/cropping is performed. Press Q with the preview focused to stop and print timing. The user verified camera reconnection followed by successful previews and error-free shutdown with Escape and the window close button in separate runs.

Omit 0.90 to always show the highest-scoring class. The threshold is an unvalidated display setting, not calibrated confidence or reliable unknown-object rejection. The model always computes avocado/banana/lemon scores, including for unfamiliar objects.

## Offline still-image fallback
With the same working directory and PATH:

```powershell
.\build-msvc\LiveVision.exe predict-cnn "data/GroceryStoreDataset/dataset/val/Fruit/Avocado/Avocado_001.jpg"
```

This known validation example predicts avocado. It is not a new camera capture. The console fallback always selects a class and does not apply the camera threshold. Local image and checkpoint files are required and are ignored by Git.

## Measured performance
User-provided terminal output for camera-check 2 0.90:

| Measurement | Result |
| --- | ---: |
| Frames | 778 |
| Mean model inference | 0.37 ms |
| Mean application read-to-display latency | 33.70 ms |
| Achieved FPS | 29.65 |

About 26.24 seconds elapsed, inferred from rounded frames/FPS. One CPU computation thread. User described the preview as responsive. One run is not a sustained-performance guarantee. Camera resolution/backend and detailed hardware inventory were not recorded.

steady_clock measures model.forward separately. Application latency spans the camera.read call through preprocessing, inference, caption drawing and imshow/waitKey event handling. It excludes unknown camera/driver buffering and actual monitor presentation delay; physical camera-to-screen latency remains unmeasured. FPS is frames divided by loop wall time. Model/camera startup is excluded; first-frame overhead is included. The timing difference cannot identify a specific bottleneck without further measurements.

## Observations and limits
User reported scores dropping when moving into the frame. Cause is unverified: whole-frame composition changes and motion blur are possible explanations. Earlier hand-only scores were about 85%; face/body about 60%. An unfamiliar input can still produce a high score. No formal live accuracy evaluation was performed. The printed 'CNN predicted grocery: avocado' came from the still-image command and is not independent live-avocado evidence.

Phase 7 held-out still-image result remains 85/125 (68%), primarily banana/lemon confusion. Test results were not used for tuning. Capture-session independence and CNN superiority over the historical linear baseline remain unestablished.

## Rehearsal and remaining closeout
Explain: "I built a C++ classifier using OpenCV and CPU LibTorch for avocado, banana and lemon. The selected checkpoint scored 68% on 125 held-out still images. This machine achieved about 30 FPS in one camera run. Scores are not calibrated confidence; unfamiliar objects can still get fruit labels."

Show one centered avocado and the actual output, including uncertainty/mistakes. If the camera fails, use the saved-image fallback. Do not run train, which overwrites Run B artifacts. Defer optional model work and Phase 9. Reserve 20–30 minutes for two rehearsals, a reconnect check, and Q/Escape/window-close checks.

Phase 8 technical closeout is complete: build, preprocessing, fallback, argument validation, timing, reconnect, and Q/Escape/window-close evidence are recorded. Final CTest passed 1/1 (infrastructure smoke test). Phase 7 PR #9 is verified merged at commit 8242aa0df43a58e89b9cddac1f469d4cf5cca519. Phase 8 Git synchronization remains pending; access restrictions currently block fetch. Presentation rehearsal is a Phase 10 activity. No revised Phase 8 due date was agreed. Career fair remains October 1, 2026.