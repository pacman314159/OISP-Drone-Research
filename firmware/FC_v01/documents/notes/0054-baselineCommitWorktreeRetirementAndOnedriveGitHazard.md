---
id: "0054"
title: Single Baseline Commit, Worktree Retirement & OneDrive-Resident Git Hazard
author: Drone Research Team / OpenCode
date: 2026-11-21
time: 18:05:00 +07:00
tags: ["#zettelkasten", "#firmware", "#fc_v01", "#git", "#repository", "#process", "#audit"]
references:
  - "[[0033-platformioToolchainPackageMigrationAndArduinoDependencyAudit]]"
  - "[[0049-firmwareRefactoringAuditSessionSummaryAndVerification]]"
  - "[[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]"
---

# Zettel 0054: Single Baseline Commit, Worktree Retirement & OneDrive-Resident Git Hazard

## 1. Metadata & Scope
- **ID**: `0054`
- **Author**: Drone Research Team / OpenCode
- **Date**: 2026-11-21
- **Time**: 18:05:00 +07:00
- **Status**: Approved & Implemented
- **Scope**: repository layout, history shape, worktree/branch lifecycle, and the placement of `.git` relative to OneDrive.

---

## 2. Context & Motivation

By 2026-11-20 the repository carried 41 modified paths, 8,361 staged deletions (the G1–G3 untracks) and 7 untracked paths, all mixing the uncommitted **0052 refactor** with every audit fix. Two branches existed that nobody had reconciled (`wip/agy-390e0b16-snapshot`, `agents/greeting-response`), one stash, and six in-flight session worktrees created by the agent client — each a detached-HEAD checkout of the repository.

Two facts forced the decision:

1. **The split was no longer executable.** 0052 and the audit fixes sit inside the same files; hunk-level separation costs hours for cosmetic history.
2. **An unmerged branch is not an empty branch.** `wip/agy-390e0b16-snapshot` had been listed as "stale, delete after confirming nothing unique". It carried the thrust-bench analyzer pipeline, the GPS/GNSS selection guide behind `[[0051-gpsGnssModuleSelectionAndAntennaPhysics]]`, and an async-I2C architecture note that existed nowhere else.

---

## 3. Architectural Decision

1. **One baseline commit, referenced by a tag.** The dirty tree (0052 + audit fixes + untracks) is committed whole as `69dd292`, and the *tag*, not a split history, becomes the reference point for "before the audit". Splitting history for aesthetics is rejected.
2. **Worktrees are session-scoped and must be audited, not assumed.** Before removal each tree is checked for uncommitted content and each detached HEAD is compared against `main`. Five of six were genuinely empty; one held only render captures; the sixth proved the "stale" assumption wrong.
3. **Unmerged work is anchored before anything is deleted.** Every branch and dangling commit that carries unique content gets an `archive/*` tag (`archive/wip-agy-390e0b16`, `archive/zed-checkpoint-2026-10-28`, `-10-29`, `-10-29-a`, `-10-29-b`) so `git gc` cannot reclaim it. Deleting a branch is only permitted once its content is either merged or tagged.
4. **Generated artefacts are deleted, not merely ignored.** `.archify/` (0.9 MB) and `.playwright-mcp/` are removed and added to `.gitignore` alongside `~$*`; `.archify/` had been tracked since `69dd292` and was removed from the index as well.
5. **`.git` must not live inside a synchronised folder.** This repository's `.git` sits under OneDrive. The object database was found corrupted — `pack-f6ef8b0e….pack` truncated, its header claiming 343 objects against 348 in its index, rejected on SHA1 — which is the known failure mode of a sync client mutating packfiles mid-write. Nothing reachable was lost (`git archive` of both `HEAD` and `727aefe` materialised in full), the pack was quarantined, and `git fsck` is now clean. The durable fix is moving `.git` out of the synced tree or excluding it from sync.

---

## 4. Impacted Artefacts

| Area | Change |
|---|---|
| History | `69dd292` baseline; later `070b279`, `c0b22bf`, `81f6268`, `41d8c89`, `770b9a9`; `origin/main` in sync |
| Branches | `wip/agy-390e0b16-snapshot` and `agents/greeting-response` deleted after content audit; only `main` remains |
| Tags | 5 `archive/*` tags anchoring rescued work; `baseline-2026-*` **not yet created** |
| Worktrees | 6 removed; `git worktree list` shows the main tree only |
| Stash | `stash@{0}` dropped — it deleted JST footprints at a path that no longer exists in the tree |
| Ignore rules | `.archify/`, `.playwright-mcp/`, `~$*` added |
| Rescued content | Analyzer thrust-bench pipeline, GPS/GNSS guide + PDFs, async-I2C note — all committed to `main` |

---

## 5. Verification

- `git fsck` → exit 0, no dangling objects; `git archive HEAD` and `git archive 727aefe` both complete.
- `git worktree list` → 1 entry; `git branch` → `main` only; `git stash list` → empty.
- `git rev-list --count origin/main..main` → 0 after push.
- Four empty directories remain under the client's worktree store; Windows keeps handles on them until the app restarts.

---

## 6. Cross-References & Traceability
- [[0033-platformioToolchainPackageMigrationAndArduinoDependencyAudit]]: earlier repository/toolchain hygiene decision in the same family.
- [[0049-firmwareRefactoringAuditSessionSummaryAndVerification]]: the audit session this note closes out; its ledger is only trustworthy once history and the working tree agree.
- [[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]: the code that landed in these commits, still awaiting the build gate.
