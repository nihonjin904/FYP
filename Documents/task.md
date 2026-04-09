# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-09 18:50 (HKT)_

---

> ## ⚠️ AI 每次必讀提醒（每次 session 強制執行）
> 1. **必須先讀** `project_overview.md` + `unrealbullshit.md`，才能動手
> 2. **所有回答必須寫在此 task.md 裡，不在 chat 對話回答**
> 3. ❌ 禁止探索式 MCP calls
> 4. ❌ 禁止重複建 Python 腳本（先查 project_overview.md 清單）
> 5. ❌ **禁止動 `BP_SekiroCharacter`（朋友在改）**
> 6. ⛔ **先 Plan，等用戶批准才執行**

---

## 🟡 問題 A：站姿後傾 — 等問題 C 解決
## 🟡 問題 C：人物面向反了 — 待執行
## ✅ 問題 B：武器掛載 — 已解決

---

## 🔴 當前任務：Boss Arena — Level_Environment.umap 改造

### ✅ 已確認
- `Content/JapaneseShrine/` 已 copy 完成
- `Level_Environment.umap` 已打開確認正常
- 場景：同一 Level 內有室外神社 + 室內道場（Pack 原裝）

---

## 📋 Phase 2 Plan（等你批准才執行）

> **狀態：🟡 PLAN 中 — 未動任何東西**

### 目標
把 `Level_Environment.umap` 改成可遊玩的 Boss Arena，最小改動，保留原場景不破壞。

---

### Step 1：確認場景現有 Actor（MCP 讀取，不改東西）
- 用 MCP `get_actors_in_level` 讀取 Level_Environment 裡現有的所有 Actor
- 目的：找出場景原點坐標、現有燈光、現有 PlayerStart（如有）
- **只讀，不改**

---

### Step 2：放置 PlayerStart（MCP spawn_actor）

| 項目 | 計劃 |
|------|------|
| 位置 | 室外廣場入口（鳥居方向），面向神社正殿 |
| 坐標 | 由 Step 1 的 Actor 坐標推算（等 Step 1 結果） |
| 不影響 | 不動任何 BP、不動任何 C++ |

---

### Step 3：放置 Boss（MCP spawn_blueprint_actor_in_level）

| 項目 | 計劃 |
|------|------|
| Blueprint | `BP_SekiroEnemy`（現有，不修改內部） |
| 位置 | 神社正殿前石階頂，面向玩家方向 |
| 坐標 | 由 Step 1 推算 |

---

### Step 4：World Settings Game Mode（MCP 或你手動）

| 項目 | 計劃 |
|------|------|
| 做法 | World Settings → Game Mode Override → 選現有 Game Mode |
| 目的 | 確保 Play 時用正確的 Pawn/Controller |
| ⚠️ 注意 | 不新建 Game Mode，用現有的 |

---

### Step 5：燈光微調（可選，最後才做）

| 項目 | 計劃 |
|------|------|
| ExponentialHeightFog | Level 如已有就調參數，沒有才加 |
| Directional Light | 調角度至黃昏感（如 Level 本身燈光已夠好就跳過） |
| ⚠️ 原則 | Pack 原裝燈光已很好，能不動就不動 |

---

## ❓ 批准前需要你決定

1. **UE5 現在是否開著，Level_Environment 是否 active？**  
   → 需要 UE5 開著才能用 MCP，你說一聲「開著」我就開始

2. **PlayerStart 你想放在室外（廣場入口）還是室內（道場入口）？**  
   → 我建議室外廣場

3. **Boss 放室外廣場，還是室內道場？**  
   → 我建議室外（空間大、相機好操控）

4. **現有 Game Mode 叫什麼名字？**  
   → 你去 Project Settings → Maps & Modes 看 Default Game Mode 叫什麼

---

## 🔗 資產位置

| 資產 | Content Browser 路徑 |
|------|---------------------|
| 主場景 | `/Game/JapaneseShrine/Maps/Level_Environment` |
| 資產預覽 | `/Game/JapaneseShrine/Maps/Level_AssetsOverview` |
| 所有 Mesh | `/Game/JapaneseShrine/Meshes/` |

---

## bullshit 記錄

- lint errors = clang 找不到 UE5 header，不影響 VS Build
- Fab.com listing 需要登入，AI 讀 403，只能用戶自己看

---

_資料截至：2026-04-09 18:50 (HKT)_
