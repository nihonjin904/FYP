# 01-CONTEXT.md — Phase 1: Boss 危攻擊 AnimNotify

## Phase Overview

**Goal:** 在 BP_SekiroEnemy 的 5 個危攻擊 Montage 加入 `AN_PerilousHit` AnimNotify，在正確時機呼叫 C++ 函數 `PerformPerilousHitCheck()`，令危攻擊無法被玩家格擋。

**危攻擊 Montage 清單（全部需要加 AnimNotify）：**
```
AM_Perilous_Slash_Patchouli
AM_Perilous_Thrust_Patchouli
AM_PerilousAttack_Slash
AM_PerilousAttack_Sweep
AM_PerilousAttack_Thrust
```

**危字 Widget：** `WBP_PerilousWarning`（已存在，C++ OnPerilousAttackStarted.Broadcast() 已觸發它）

---

## Decisions

### AnimNotify 類型
- **決定:** AnimNotify（點，單幀觸發）
- **理由:** 危攻擊是瞬間判定，不需要持續窗口（AnimNotifyState）

### 呼叫方式
- **決定:** Blueprint AnimNotify（非 C++ class）
- **具體做法:**
  1. 在 Montage 編輯器的 Notifies 軌道，右鍵加一個新的 AnimNotify
  2. 命名為 `AN_PerilousHit`
  3. 在 `BP_SekiroEnemy` 的 AnimBlueprint 的 Event Graph 監聽 `AN_PerilousHit`
  4. 在 Event 節點後呼叫 C++ 函數 `PerformPerilousHitCheck()`
- **理由:** 不需要 C++ compile，Blueprint 內可直接拖動調整時機，debug 容易（可加 PrintString 即時看）

### 觸發時機點
- **決定:** 由 AI/Planner 決定，Planner 視動畫判斷武器接觸玩家的那一幀
- **參考位置:** 看危攻擊動畫 timeline，在武器「揮到」玩家位置那一刻放 Notify

---

## Existing C++ Context

以下已完成，Planner 直接使用：

```
函數:   USekiroCombatComponent::PerformPerilousHitCheck()
位置:   Source/FYP/Private/Components/SekiroCombatComponent.cpp
Tag:    Attack.Perilous (繞過 DeflectComponent 格擋判定)
變數:   PerilousAttackDamage, PerilousPostureDamage
```

---

## Canonical Refs

- `Source/FYP/Private/Components/SekiroCombatComponent.cpp` — PerformPerilousHitCheck() 實現（Line 577）
- `Source/FYP/Public/Components/SekiroCombatComponent.h` — 變數聲明（Line 48）
- `Content/UI/WBP_PerilousWarning` — 危字 Widget（OnPerilousAttackStarted 廣播觸發）
- 全部 5 個 AM_Perilous* Montage assets — 需要加 AnimNotify

---

## UAT Criteria (from ROADMAP)

1. 玩家格擋狀態下被危攻擊仍然受傷
2. 危字出現時間與攻擊判定時間對齊
3. 不影響其他普通攻擊邏輯

---

## Deferred Ideas

- C++ AnimNotify class（更乾淨，但現階段不需要，可在 v2.0 重構時考慮）
