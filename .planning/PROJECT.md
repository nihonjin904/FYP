# PROJECT.md — FYP Sekiro-like Boss Combat (Kelvin)

## 項目概覽

**項目名稱:** FYP — Sekiro-style Boss Combat System

**負責人:** Kelvin Lam (nihonjin904)

**引擎:** Unreal Engine 5

**目標:** 實現完整且高質感的 Boss 戰鬥系統，包括格擋、猛撃、危攻擊、姿態條、閃避等戰鬥核心機制。

---

## 核心 Blueprint

- `BP_SekiroEnemy` — Boss 主 Blueprint（攻擊、格擋、AI 邏輯）
- `BP_PlayerCharacter` — 玩家角色（格擋、受傷、閃避）
- `WBP_BossHUD` — Boss 血量/姿態條 UI

---

## 當前 Milestone

**v1.0 → v1.1 Boss Combat Polish**

目標：讓 Boss 戰鬥有完整的無法阻擋攻擊、危字警告、格擋反饋。

---

## 備註

- 同隊成員負責 Checkpoint / Minimap / 地圖系統（獨立 .planning 已清除）
- 本 .planning/ 只追蹤 Kelvin 自己的 Boss 戰鬥工作
- .planning/ 已加入 .gitignore，不會 push 到 GitHub
