# Roadmap: FYP Sekiro-style Boss Combat System

## Overview

實現完整 Sekiro 風格 Boss 戰鬥機制，包括危攻擊、架勢條、閃避系統。與朋友協作，本 .planning/ 只追蹤 Kelvin 的 Boss 戰鬥工作。

## Phases

- [/] **Phase 1: Boss 危攻擊 AnimNotify** - 危攻擊加傷害，玩家無法阻擋
- [x] **Phase 2: 玩家架勢條平衡修正** - 精準擋刀懲罰改為固定合理增量
- [ ] **Phase 3: 玩家閃避系統** - 玩家按 Space 可閃避危攻擊，成功閃避有無敵幀
- [x] **Phase 4: Merge feature/perilous-attack-notify 去 master** - 安全處理 branch merge，解決潛在衝突（SekiroHUD、uasset）

## Phase Details

### Phase 1: Boss 危攻擊 AnimNotify
**Goal**: 在已有動作、危字、音效的基礎上，加入無法阻擋的傷害邏輯。在 UE5 Montage 正確時機添加 AnimNotify 呼叫 PerformPerilousHitCheck()。
**Depends on**: Nothing (first active phase)
**Requirements**: REQ-01, REQ-02
**Success Criteria** (what must be TRUE):
  1. 玩家格擋狀態下被危攻擊仍然受傷
  2. 危字出現時間與攻擊判定時間對齊
  3. 不影響其他普通攻擊邏輯
**Plans**: TBD

Plans:
- [ ] 01-01: 在 BP_SekiroEnemy 危攻擊 Montage 加 AnimNotify + 測試驗證

### Phase 2: 玩家架勢條平衡修正
**Goal**: Boss 精準擋刀時，玩家架勢條不再瞬間滿，改為固定合理增量。
**Depends on**: Phase 1
**Requirements**: REQ-03
**Success Criteria** (what must be TRUE):
  1. 精準擋刀後架勢條增量為固定值（非乘數）
  2. 架勢條不會因一次精準擋刀而瞬間爆滿
**Plans**: 1 plan

Plans:
- [x] 02-01: SekiroCombatComponent.cpp — AttackPostureDamage * 1.5f → PerfectParryPosturePenalty (固定 15.0f)

### Phase 3: 玩家閃避系統
**Goal**: 玩家按 Space 可在危攻擊前閃避，成功閃避有無敵幀。
**Depends on**: Phase 1
**Requirements**: REQ-04
**Success Criteria** (what must be TRUE):
  1. 玩家按 Space 可觸發閃避動作
  2. 閃避成功後有無敵幀（不受危攻擊傷害）
  3. 閃避有 Cooldown，不可無限連閃
**Plans**: TBD

Plans:
- [ ] 03-01: 閃避輸入 + 無敵幀邏輯

### Phase 4: Merge feature/perilous-attack-notify 去 master
**Goal**: 安全地把 Phase 1 的危攻擊 branch merge 去 master，確保不出現代碼或 Blueprint 衝突，並同步朋友的改動。
**Depends on**: Phase 1
**Risk Files**:
  - `SekiroHUD.h / .cpp` — 高風險（雙方都改過 HUD）
  - `.uasset` Blueprint 二進位 — 衝突時只能選一個版本
  - `SekiroGameMode.cpp` — 中風險
**Success Criteria** (what must be TRUE):
  1. master branch 包含危攻擊所有功能（UI、傷害、Posture）
  2. 與朋友的 Fast Travel / 地圖系統共存，無衝突
  3. 所有 C++ 文件保留雙方改動
  4. Blueprint .uasset 功能正常（危攻擊動畫含 AnimNotify）
**Plans**: TBD

Plans:
- [x] 04-01: 檢查衝突文件 → 解決衝突 → Merge 去 master

## Progress

**Git Branch 規則:**
- master ← 穩定版本（只放測試通過的功能）
- feature/xxx ← 每個新 Phase 開一條新 branch

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Boss 危攻擊 AnimNotify | 1/1 | Complete | 2026-04-20 |
| 2. 玩家架勢條平衡修正 | 1/1 | Complete | 2026-04-17 |
| 3. 玩家閃避系統 | 0/1 | Not started | - |
| 4. Merge to master | 1/1 | Complete | 2026-04-20 |

### Phase 5: FYP Presentation PPT - Touhou Anime Style

**Goal:** [To be planned]
**Requirements**: TBD
**Depends on:** Phase 4
**Plans:** 0 plans

Plans:
- [ ] TBD (run /gsd-plan-phase 5 to break down)
