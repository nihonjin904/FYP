# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-11 22:10 (HKT) 星期六_

---

> ## 🔴 AI 必讀規則 🔴
> 你每次要查看 project_overview.md unrealbullshit.md 每次都要實時更新
> 每次回答我要在 task.md 裡面回答我 並且更新
> 請先查清楚問題 / 先plan 讓我批准先
> MCP 操作後必須用 analyze_blueprint_graph 驗證

---

## 🚨 第三輪診斷 — 根因已鎖定（先 Plan，不改動）

### 截圖分析結果

| 截圖 | 看到什麼 | 意味著什麼 |
|------|---------|-----------|
| 截圖 2（BP Editor） | `Perilous Attack Montages` = 2 elements，`AM_Perilous_Slash_Patchouli` + `AM_Perilous_Thrust_Patchouli` | **BP CDO（Class Default Object）有正確的值** ✅ |
| 截圖 3+4（遊戲中） | 只有 `Attack() called, ComboMontages=4`，**完全沒有** `⚠ BOSS: PERILOUS ATTACK!` | **TryPerilousAttack() 從來沒有返回 true** 🔴 |
| 截圖 3+4（遊戲中） | 也沒有 `Perilous Montages loaded:` | BeginPlay 時的 debug message 已過期（5秒消失）或 **runtime 陣列 = 0** |

### 🔴 根因分析

**BP Editor 的 CDO 有值 ≠ 場景中的 Actor Instance 有值**

在 UE5 中，當你在 **Blueprint Editor** 修改 component 屬性時，你修改的是 **CDO（Class Default Object）**。但是：

> **場景中已經放置的 Actor 實例（`BP_SekiroEnemy_C_1`）有自己的 component instance 數據，不會自動從新的 CDO 繼承！**

```
你看到的（BP Editor CDO）：
  PerilousAttackMontages = [AM_Perilous_Slash, AM_Perilous_Thrust]

實際 runtime 用的（場景中的 instance）：
  PerilousAttackMontages = []  ← 空的！
```

**為什麼會這樣？**

1. 之前我用 MCP `spawn_blueprint_actor_in_level` 放置了 `BP_SekiroEnemy_C_1`
2. 那時候 CDO 裡面的 `PerilousAttackMontages` 是**空的**（你還沒填入新 Montage）
3. spawn 的時候，instance 複製了當時的 CDO 值 → `PerilousAttackMontages = []`
4. 之後你在 BP Editor 修改 CDO 加入了 2 個 Montage
5. **但場景中的 instance 保留了舊值（空陣列）**，不會自動更新！

**這就是為什麼：**
- `TryPerilousAttack()` 第一行 `PerilousAttackMontages.Num() <= 0` → **return false** → 從不觸發
- 沒有 `⚠ BOSS: PERILOUS ATTACK!` → 從沒走到那一行
- 沒有「危」字 → 從沒 Broadcast
- 只有普通 combo → 因為 TryPerilous 失敗後走普通 `Attack()`

### 🔎 C++ 代碼確認

```cpp
// SekiroEnemyAttributeComponent.cpp Line 170:
bool USekiroEnemyAttributeComponent::TryPerilousAttack()
{
    if (PerilousAttackMontages.Num() <= 0)   // ← runtime 是 0！
        return false;                          // ← 直接 return！後面的代碼完全不執行
    
    // ... 以下代碼永遠不會到達 ...
}
```

---

## 📋 修復方案（等你批准）

### 方案 A：刪除場景中 Boss 重新放置（推薦 ⭐）

| 項目 | 內容 |
|------|------|
| 方法 | 用 MCP 刪除場景中的 `BP_SekiroEnemy_C_1`，重新 Spawn 一個新的 |
| 原理 | 新 Spawn 的 Actor 會自動繼承最新的 CDO 值（包括 2 個 Montage） |
| 預計時間 | **10 秒**（2 個 MCP 指令） |
| 成功率 | **95%** |
| 風險 | 🟢 零風險（只是刪掉重放，位置和旋轉保持不變） |

**MCP 步驟**：
1. `delete_actor("BP_SekiroEnemy_C_1")`
2. `spawn_blueprint_actor_in_level("/Game/BP_SekiroEnemy", "BP_SekiroEnemy_C_1", location=[800, 60, 293.54], rotation=[0, -60, 0])`

### 方案 B：在場景 Outliner 選中 Boss，手動更新 Instance 值

| 項目 | 內容 |
|------|------|
| 方法 | 在 Level Editor 的 Outliner 選中 `BP_SekiroEnemy_C_1`，在 Details 面板手動修改 instance 的 PerilousAttackMontages |
| 預計時間 | **1 分鐘**（手動操作） |
| 成功率 | **90%** |
| 風險 | 🟢 零風險 |

**手動步驟**：
1. 在 Level Editor **Outliner**（場景大綱）找到 `BP_SekiroEnemy_C_1`，**點一下**選中
2. 右邊 **Details** 面板 → 找 `SekiroEnemyAttribute` component
3. 展開 **Perilous** 分類
4. 看到 `Perilous Attack Montages` → **應該是空的（0 elements）**
5. 按 `+` 加 2 個，填入 `AM_Perilous_Slash_Patchouli` 和 `AM_Perilous_Thrust_Patchouli`
6. **Ctrl+S** 保存 Level
7. 測試

### 方案 C：在 Instance 層面右鍵 Reset to Default

| 項目 | 內容 |
|------|------|
| 方法 | 在 Outliner 選中 Boss → Details → Perilous Attack Montages 旁邊的**黃色箭頭**（Reset to Default）→ 會從 CDO 重置 |
| 前提 | 需要先確認有沒有黃色箭頭 |
| 預計時間 | **10 秒** |

---

## ❓ 你選哪個？

**推薦方案 A**（最快最穩，MCP 直接做）

你批准「方案 A」的話我就直接執行。

---

## 📊 debug 邏輯鏈（供日後參考）

```
TickComponent → bAutoAttack=1, CombatComp=OK
  ↓ TimeSinceLastAttack >= 3.0
  ↓
StartComboAttackCycle()
  ↓
TryPerilousAttack()
  ↓ PerilousAttackMontages.Num() = 0  ← 🔴 instance 是空的！
  ↓ return false
  ↓
SekiroChar->Attack()  ← 走普通 combo
  ↓
"[BP_SekiroEnemy_C_1] Attack() called, ComboMontages=4"  ← 你看到的
```

---

_回答時間：2026-04-11 22:10:30 (HKT) 星期六_
_累積對話 tokens：約 1,400,000_
