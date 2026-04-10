# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-10 18:41 (HKT)_

---

> ## ════════════════════════════════════
> ## 🔴 AI 每次必讀（無論什麼 session）🔴
> ## ════════════════════════════════════
> **第一步永遠是：讀 `project_overview.md` 和 `unrealbullshit.md`，不讀不准做任何事**
>
> **所有回答只能寫在 task.md，禁止在 chat 回答用戶**
>
> 1. ❌ 禁止探索式 MCP calls
> 2. ❌ 禁止重複建 Python 腳本（先查 project_overview.md 清單）
> 3. ❌ **禁止動 `BP_SekiroCharacter`（除非特別說明）**
> 4. ⛔ **先 Plan → 等批准 → 才執行**

---

## ✅ 已修復：跳躍漂浮問題（2026-04-10 18:41 HKT）

### 症狀確認

| 問題 | 確認內容 |
|------|---------|
| 症狀 | 原地可以跳，但跳起後漂浮在空中不落地 |
| 觸發條件 | 跳起後按 WASD → `Move()` 強制 `MOVE_Walking` → 漂浮 |
| 朋友在改 | 主角材質 + 場地水效果（不是 SekiroCharacter.cpp）|

### 根本原因

`SekiroCharacter.cpp` 的 `Move()` 函數無條件每幀強制 `SetMovementMode(MOVE_Walking)`。
跳起後 MovementMode 改為 `MOVE_Falling`，但 `Move()` 立即把它強制改回 `MOVE_Walking`，
導致重力失效、角色在空中「行走」漂浮。

### 執行記錄

| 步驟 | 狀態 | 說明 |
|------|------|------|
| 定位問題 | ✅ | `Move()` 裡 `SetMovementMode(MOVE_Walking)` 無條件每幀執行 |
| 用戶批准 | ✅ | 2026-04-10 18:39 HKT |
| 修改 `SekiroCharacter.cpp` | ✅ | 已完成（見下方）|
| Build + 測試 | ⏳ 等你做 | 見下方步驟 |

### 已做修改

**文件：** `Source/FYP/Private/Characters/SekiroCharacter.cpp`
**函數：** `Move()`，Line 633

```cpp
// ❌ 修改前（舊）
// 確保 CharacterMovement 永遠係 Walking mode，唔會被 Montage 停止
if (GetCharacterMovement()) {
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

// ✅ 修改後（已完成）
// 確保 CharacterMovement 在地面時係 Walking mode，唔會被 Montage 停止
// ⚠️ 修復：只有在地面上才強制 Walking，空中跳躍時不覆蓋 MOVE_Falling（否則角色漂浮）
if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) {
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
```

---

### ⏳ 你需要做：Build + 測試

**Step 1：在 UE5 重新 Build**

```
推薦：UE5 裡 Ctrl+Alt+F11（Live Coding 熱重載）
或：VS / Rider → Build Solution（Ctrl+Shift+B）
```

**Step 2：按 Play 測試**

| 測試項目 | 預期結果 |
|---------|---------|
| 原地按 Space | ✅ 跳起然後正常落地 |
| 移動中按 Space | ✅ 跳起然後正常落地 |
| 格擋/攻擊時移動 | ✅ 仍然可以移動（現有功能不影響）|

**Step 3：回報結果**
- ✅ 正常 → 問題解決，關閉此任務
- ❌ 仍有問題 → 告訴我具體症狀

---

## 📌 歷史任務記錄

### ✅ Level_Environment GameMode 修復（已完成，上個 session）
- 修復了 T-Pose + HUD 消失問題（GameMode 設定錯誤）
- `Level_Environment` 已設定正確 GameMode

### ✅ 場景遷移（已完成）
- 戰鬥設定從 `Map_CombatDemo` 遷移到 `Level_Environment`
- PlayerStart + Boss Actor 已放置

---
