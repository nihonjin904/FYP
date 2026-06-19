# Phase 3: 玩家閃避系統 - Context

**Gathered:** 2026-05-02
**Status:** Ready for planning

<domain>
## Phase Boundary

玩家按 **Left Shift** 觸發閃避（Dodge Roll/Dash）。
主角永遠面向 Boss（Lock-on 旋轉），WASD 控制閃避方向。
閃避期間全程無敵幀（i-frames），可躲避危攻擊及普通攻擊。
有 Cooldown 防止無限連閃。

**不在此 Phase 範圍內：**
- 閃避反擊（Counter）窗口 — 未來 Phase
- 閃避動畫 Blend 優化 — 未來 Phase

</domain>

<decisions>
## Implementation Decisions

### D-01: 閃避觸發鍵
- **Left Shift** 鍵觸發閃避（Space 已被 Jump 佔用）
- 任何時候可按（不限於危攻擊窗口期）

### D-02: 閃避方向邏輯
- **Lock-on 方向系統** — 主角永遠面向 Boss
- WASD 控制相對於面向方向的 dash 方向：
  - S + Shift = 向後退（最常用，躲開危攻擊）
  - A + Shift = 向左橫閃
  - D + Shift = 向右橫閃
  - W + Shift = 向前衝（近身）
  - 無 WASD 輸入 = 預設向後退
- 位移方式：`LaunchCharacter()` impulse 或 `AddMovementInput()` burst
- 位移距離：短距離（約 200-400 UE units）

### D-03: 無敵幀 (i-frames) 實作
- **選擇：全程無敵版（最簡單）**
- 整個 Dodge Animation 期間，設 `bIsInvincible = true`
- 動畫結束（或 AnimNotify）後 reset `bIsInvincible = false`
- 在 `SekiroDeflectComponent::TryParry()` 或傷害函數中：
  若 `bIsInvincible == true` → 略過所有傷害判定

### D-04: Cooldown
- Dodge 有 Cooldown（建議 0.8~1.0 秒），防止連閃
- 用 UE5 Timer 或 `GetWorld()->GetTimerManager()` 實作
- Cooldown 期間再按 Shift = 忽略輸入

### D-05: 動畫
- 觸發 Dodge Montage（AM_PlayerDodge 或類似名稱）
- 若無現成 Dodge 動畫 → 用 Roll/Step 替代，或用 Root Motion 位移
- **the agent's Discretion：** 若現有角色無 Dodge 動畫資源，改用程序化 impulse + 短暫姿態過渡

### D-06: 現有系統整合
- 傷害函數位於 `SekiroCombatComponent.cpp`
- 玩家 Blueprint：`BP_PlayerCharacter`
- Input 綁定在 Enhanced Input 或舊版 Input（需查現有 `BP_PlayerCharacter` Input Action 設定）
- Cooldown flag 放在 `BP_PlayerCharacter` 或對應 C++ Component

### Agent's Discretion
- 動畫資源選擇（若無 AM_PlayerDodge）
- Dodge 位移速度曲線（linear / ease-out）
- i-frame 期間是否播放特效（如速度線/閃光）— 可選

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### 現有戰鬥系統
- `SekiroCombatComponent.cpp` / `.h` — 傷害邏輯、PerformPerilousHitCheck()
- `SekiroDeflectComponent.cpp` / `.h` — TryParry() 判斷，需加 bIsInvincible 檢查
- `.planning/phases/01-boss-animnotify/01-01-PLAN.md` — 危攻擊系統參考（delegate 架構）
- `.planning/ROADMAP.md` — Phase 3 完整 Goal 和 Success Criteria

### 玩家 Blueprint
- `BP_PlayerCharacter` — 玩家角色主 Blueprint（Input、移動、動畫）

</canonical_refs>

<specifics>
## Specific Ideas

- Sekiro 原作閃避感覺：短距離快速 dash，不是翻滾
- 主角面向 Boss（Lock-on）+ WASD 方向 = 類 Dark Souls/Sekiro 操作
- i-frame 全程無敵（FYP Demo 用，易 debug）
- 暫不需要 i-frame 計時精確（非 frame-perfect 窗口設計）

</specifics>

<deferred>
## Deferred Ideas

- 閃避反擊窗口（Counter-attack after successful dodge）
- 精準閃避（只有危攻擊時才有 i-frame，平時 = 純位移）
- 閃避耐力消耗系統
- 多方向閃避動畫（8方向 blend space）

</deferred>

---

*Phase: 03-player-dodge-system*
*Context gathered: 2026-05-02 via /gsd-discuss-phase 3*
