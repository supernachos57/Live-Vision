# Schedule, milestones, and progress tracking

Plan date: **September 6, 2026**. Fixed career-fair deadline: **October 1, 2026**. This is an ambitious learning schedule; dates are targets and progress requires evidence.

**Current update (September 29): Phases 1–6 checkpoints complete. The user accepted the bounded audit and unchanged upstream splits on September 29, with session independence explicitly unverified. Initial Phase 7 reporting is implemented. Older schedule tables are historical.**

## Scope and precedence

The [library-focused roadmap](roadmap.md) controls conflicts. Phases 2–6 are library foundations, a C++ learning exercise, baseline classification, CNNs, and dataset engineering. Handwritten Matrix and backpropagation tasks are excluded. CPU LibTorch with MSVC x64 has been selected and verified; OpenCV handles image preprocessing. The original roadmap's next-decision language is historical.

Phase 1 setup/testing remains complete. Phase 10's essential career-fair deliverables remain due September 30, consistent with that roadmap. Phase 9 is post-fair stretch work. Additional polish continues after the fair without relabeling all Phase 10 work as deferred.

## Milestone schedule

Create nine open repository milestones, one for each Phase 2–10. Phase 1 is recorded complete in the README and needs no retroactive issue or milestone. Copy the titles, due dates, and exit criteria below into GitHub; the exit criteria are the milestone descriptions.

| Phase / milestone title | Planned start | Due date | Exit criteria |
| --- | --- | --- | --- |
| Phase 2 — ML foundations and library decision | 2026-09-06 | 2026-09-09 | Decision record, verified C++ library integration, and explained tensor example. |
| Phase 3 — Learn the selected library in C++ | 2026-09-09 | 2026-09-10 | Small training exercise works; saved and reloaded predictions agree. |
| Phase 4 — Baseline grocery classifier | 2026-09-11 | 2026-09-18 | Small class set, provisional split, end-to-end training and image prediction, baseline metrics. |
| Phase 5 — CNNs and image classification | 2026-09-16 | 2026-09-19 | Library-defined CNN evaluated against the baseline on the same split. |
| Phase 6 — Dataset engineering and reproducibility | 2026-09-16 | 2026-09-20 | Documented sources, grouped splits, preprocessing contract, manifests, and leakage checks. |
| Phase 7 — Train, evaluate, and select the grocery model | 2026-09-20 | 2026-09-21 | Selected checkpoint, held-out results, per-class metrics, and known failures. |
| Phase 8 — Live camera application and pipeline | 2026-09-22 | 2026-09-25 | Single-item live classification, error handling, latency measurements, and fallback input. |
| Phase 9 — ONNX Runtime production and detection comparison | 2026-10-02 | 2026-10-15 | Tentative stretch target: runtime comparison, conditional classifier export, and separate pretrained detection experiment. |
| Phase 10 — Career-fair release and presentation | 2026-09-06 | 2026-09-30 | Verified demo, README, architecture, results, recording, rehearsal, and release tag before October 1. |

Phase 9's date is tentative. A milestone's issue percentage is not overall project completion: a future milestone with no issues is unplanned at task level, not finished. Phase 10 can close before Phase 9 because its release work runs in parallel.

## Priority and scope control

- Required: working C++ training/prediction pipeline on a few classes, reproducible evaluation, live single-item demo, fallback input, and core documentation/rehearsal.
- If behind: shrink class count, model size, and experiment count; record the revised scope and retain evaluation and reliability work.
- Stretch: ONNX comparisons, pretrained detection, iPhone capture, GPU optimization, additional datasets, and presentation polish beyond the fair's needs.

Review actual progress at the end of each session and the full schedule on September 10, 15, 21, and 25. Log date changes and reasons rather than silently shifting the October 1 deadline. Use September 26–30 for fixes, recording, rehearsal, and release verification.

## Progress-tracking workflow

GitHub issues are the task record; milestones hold phase due dates; the Project Status field holds day-to-day state. README phase checkboxes and Mermaid dates are manually maintained summaries.

1. Before each phase, split its next work into tasks of roughly 30–90 minutes with a learning goal and observable acceptance criteria. Only Phase 2 is fully decomposed initially.
2. Assign each issue to its phase milestone and add it to the Project. Keep unstarted tasks in Todo.
3. Move one task to In Progress when beginning actual work. Keep a work-in-progress limit of one. For a blocker, record the reason/dependency and move back to Todo if work stops; do not mark it Done.
4. Implement on a focused branch and link a PR with `Closes #<actual-issue-number>`. For documentation-only learning tasks, link the resulting note or decision record.
5. Verify the acceptance criteria, review the diff, and merge the PR. Close the issue as completed and mark Done. Enable the project's issue-closed workflow if desired; inspect it so abandoned issues are not mistaken for delivered work.
6. At phase completion, verify every exit criterion and its evidence, close the milestone, and check the README phase in a documentation PR. Do not close a milestone merely because its current issues are closed if scope is missing.

Moving a Project card to Done is not a substitute for closing the issue. Milestone progress is based on its issues/PRs; to avoid double-counting, attach the milestone to issues and link the implementing PRs to those issues.

## Board and supporting views

Project name: **Live-Vision — Learning and MVP**. Keep it private and associate it with this private repository.

- Board: **Work board**, grouped into Status values **Todo / In Progress / Done**. Show milestone and assignee on cards. Keep all work visible; optionally save a Phase 2 filter view.
- Table: **Backlog**, showing title, status, milestone, assignee, and optional Target date.
- Optional roadmap view: use custom Start date and Target date fields for issue-level scheduling. Milestone due dates do not automatically populate these fields. The Mermaid timeline remains the phase-level schedule.

Initially add the eight Phase 2 issues and set all to Todo. Creating planning artifacts does not mean the ML learning tasks have started.

## Risks to review

| Risk | Response |
| --- | --- |
| C++ library/compiler incompatibility | Prove minimal linking early; document any compiler change and rerun existing checks. |
| Slow training | Use a small model/data slice and a bounded experiment budget. |
| Data leakage or camera domain shift | Group related captures and evaluate under realistic camera conditions. |
| Different train/inference preprocessing | Share or verify a written preprocessing and label-map contract. |
| Overloaded schedule | Reduce model/data scope, defer stretch work, protect release time. |
| Demo failure | Test on the actual machine and carry offline input and a recording. |

## Session log

Append a short entry after work; do not prefill future achievements.

```text
Date:
Issue worked on:
What I learned:
Evidence / PR / test or experiment result:
Blocker or schedule change:
Next small task:
```

## September 23, 2026 status update

Phases 1–4 learning checkpoints are complete. The agreed revised Phase 4/5/6 due dates are September 18/19/20, 2026; their implementation/documentation verification is tracked separately from those targets. Phase 4 implementation was verified September 22 and documentation closed out September 23. See [Phase 4 evidence](phase-4-results.md).

Phase 5 and later learning work remains outstanding. The old planned-start dates and later targets above are historical planning assumptions, not claims of actual progress. No replacement dates have been approved. October 1 remains the career-fair date. This local documentation update does not change GitHub milestones, issues, or project-board status.

## Phase 5 closeout and revised plan — September 27, 2026

Phases 1–5 checkpoints are complete. The small CNN trained on the same provisional split, achieved 8/16 validation accuracy versus the retained linear baseline's 10/16, passed checkpoint score parity, and predicted from saved state without training. See [Phase 5 results](phase-5-results.md). CNN training now saves saved-cnn.pt; saved-model.pt remains the linear baseline.

The user explicitly revised the Phase 6 completion target to September 27, 2026 (today in that session), with Phase 7 to start afterward. This replaces Phase 6's historical September 20 target. No Phase 7 completion deadline was agreed. Phase 6 dataset auditing/reproducibility and Phase 7 evaluation/selection remain outstanding; planned dates are not completion evidence. The career fair remains October 1, 2026.

This update supersedes older current-status and no-replacement-date statements below/above. Historical schedules are retained as history. No GitHub milestones, issues, or project-board status were changed.

## September 29, 2026 — Phase 6 audit and reproducibility evidence

This entry supersedes older current-status text. Dataset/preprocessing commands and 12 fixture cases have been verified. Seeded runs repeat printed logs; checkpoint metadata is generated; selected manifests and image fingerprints are recorded. Initial Phase 7 confusion-matrix and per-class reporting remain in the branch, but selection and held-out evaluation have not occurred.

The bounded audit found no repeated paths, byte-identical files, or identical decoded RGB images among 269 selected records. All test thumbnails and 36 cross-split similarity candidates were reviewed for data quality only. Upstream split membership is retained. Capture-session independence remains unverified because session identifiers are absent; visual groups are recorded as inferences. The user subsequently accepted this limitation on September 29 and authorized Phase 6 completion under the bounded-audit scope. This explicitly revises the original stronger grouped-split criterion; session independence is not claimed. See [Phase 6 results](phase-6-results.md) and [dataset audit](docs/dataset.md).

The September 27 Phase 6 target has passed; no replacement date or Phase 7 completion deadline is invented. The career-fair date remains October 1, 2026. No GitHub milestone or project board was changed.
