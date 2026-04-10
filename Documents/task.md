# Task.md — Arcane Souls: Rebirth
_最後更新：2026-04-10 16:26 (HKT)_

---

> ## 🔴🔴🔴 AI 強制提醒（每次 session 第一步，無例外）
> **每次開 session 必須先讀以下兩個文件再做 ANYTHING：**
> - `project_overview.md` ← 項目架構、已有腳本、禁止事項
> - `unrealbullshit.md` ← 過去犯過的錯誤，禁止重蹈覆轍
>
> **所有回答必須寫在 task.md 裡面並更新，不準在 chat 對話回答用戶**
>
> 1. ❌ 禁止探索式 MCP calls（唔係必要唔准 call）
> 2. ❌ 禁止重複建 Python 腳本（先查 project_overview.md 清單）
> 3. ❌ **禁止動 `BP_SekiroCharacter`（朋友在改時禁止）**
> 4. ⛔ **先 Plan，等用戶批准才執行任何東西**

---

## 💬 用戶問題回答（2026-04-10 15:50 HKT）

### Q: 我覺得你直接一次性做就行了，萬一出錯 GitHub 要怎麼操作？

**答：同意可以一次性執行（見下方 Plan batching）。**

關於 **GitHub 出錯如何回滾**，以下是完整操作：

---

### 🛡️ GitHub 出錯回滾操作

#### 情況 A：你還沒有 commit，只是做了改動（最安全）

```bash
# 丟棄所有未 commit 的改動，回到上一個 commit 狀態
git restore .

# 或只丟棄某個文件
git restore Source/FYP/Private/Characters/SekiroCharacter.cpp
```

#### 情況 B：你已經 commit 但還沒 push

```bash
# 回到上一個 commit（保留工作區改動）
git reset --soft HEAD~1

# 回到上一個 commit（完全丟棄所有改動）
git reset --hard HEAD~1
```

#### 情況 C：你已經 push 了（最麻煩）

```bash
# 找回要回到的 commit hash（看 log）
git log --oneline -10

# 強制回滾遠端（⚠️ 危險，會覆蓋遠端歷史，先確認沒有隊友在 pull）
git reset --hard <commit_hash>
git push origin main --force

# 更安全的做法：用 revert（不改歷史，只加一個「還原」commit）
git revert HEAD
git push origin main
```

#### 情況 D：只是 UE 場景文件出錯（.umap）

UE 的 `.umap` 是二進制文件，Git 的 diff 沒用。但回滾方法一樣：

```bash
# 回滾特定 .umap 文件到上一個版本
git restore --source HEAD~1 Content/JapaneseShrine/Maps/Level_Environment.umap
```

---

#### ⭐ 建議實際操作流程（做之前）

1. **先 commit 現在的狀態作備份點：**
   ```bash
   git add .
   git commit -m "Backup before environment migration 2026-04-10"
   ```
2. 然後讓 AI 一次性執行所有 MCP 操作
3. 如果出錯，直接 `git reset --hard HEAD~1` 回到備份點

這樣就算 AI 搞爛了，一條命令就能完全還原。

---

---

## ✅ 已確認資訊（2026-04-10）

| 項目 | 確認值 |
|------|--------|
| UE5 開著？ | ✅ 是 |
| Active Map | `Level_Environment` ✅ |
| Game Mode | `BP_SekiroGameMode` ✅ |
| PlayerStart 位置 | X=1000, Y=0, Z=204（室外廣場） ✅ |
| 神社場景路徑 | `/Game/JapaneseShrine/Maps/Level_Environment` ✅ |
| 戰鬥測試場景 | `/Game/Maps/Map_CombatDemo` ✅ |
| Boss Blueprint | `/Game/BP_SekiroEnemy` ✅（路徑更新，之前 /Game/Blueprints/ 是錯的）|
| 問題 A/B/C | ✅ 全部朋友已解決 |

---

## 🟢 已完成任務：環境遷移 Level_Environment（2026-04-10 16:07 HKT）

### 執行記錄

| Step | 操作 | 結果 | 備注 |
|------|------|------|------|
| 1 | `get_actors_in_level` | ✅ 確認無 PlayerStart / Boss | 地面 Z ≈ 82~104 |
| 2 | `set_game_mode_default_pawn` | ✅ GameMode 設為 `BP_SekiroGameMode`，PlayerStart 放在 X=1000, Y=0, Z=204 | Default Pawn = BP_SekiroCharacter_C |
| 3 | `spawn_blueprint_actor_in_level` | ✅ Boss `BP_SekiroEnemy_C_0` 放置在廣場 | 面向 PlayerStart（Yaw=180°） |
| 4 | `snap_actor_to_ground` | ✅ Boss snap 到地面 Z≈133，落在 Landscape_0 上 | 廣場 X=500, Y=500 |

### ⚠️ 發現的路徑錯誤（已修正）

**舊 project_overview.md 記錄的路徑是錯的：**

| 資產 | 舊路徑（錯） | 新路徑（對） |
|------|------------|------------|
| Boss Blueprint | `/Game/Blueprints/BP_SekiroEnemy` | `/Game/BP_SekiroEnemy` ✅ |
| Character Blueprint | `/Game/Blueprints/BP_SekiroCharacter` | `/Game/BP_SekiroCharacter` ✅ |
| Game Mode | `BP_ThirdPersonGameMode` | `/Game/BP_SekiroGameMode` ✅ |

---

## 📋 下一步（待用戶測試）

1. **在 UE5 按 Play** → 確認角色在廣場 spawn
2. **確認 Boss 出現** 在廣場中間，面向玩家
3. **測試戰鬥** — 攻擊、格擋、鎖定是否正常

如果有問題：`git reset --hard HEAD~1`（前提是已 backup commit）

---

## 🔴 新問題（2026-04-10 16:15 HKT）：Level_Environment T-Pose + 無 HUD

### 根本原因（已確認）

`set_game_mode_default_pawn` 改了 **Project Settings**，不是 **Level 的 World Settings**。
→ Level_Environment 的 GameMode Override 仍然是 **None** → T-Pose + 無 HUD + Boss 不動

---

## ❌ MCP 修復嘗試失敗（16:26 HKT）

- `set_actor_property` on `WorldInfo_0` → 失敗（WorldSettings 沒有 Component，MCP 無法改）
- **結論：MCP 不支援直接改 World Settings GameMode Override，必須手動在 UE Editor 改**

已記錄到 `unrealbullshit.md`（新增 #28）

---

## ✅ 你需要做的事（手動，約 1 分鐘）

### 步驟 1：改 Level_Environment 的 GameMode

```
1. UE5 確認打開 Level_Environment（標題欄顯示 Level_Environment）
2. 右側面板點 【World Settings】Tab
   ⚠️ 如果看不到：頂部選單 Window → World Settings
3. 找 "Game Mode" 區段 → "GameMode Override" 下拉
4. 下拉選 【BP_SekiroGameMode】
5. 確認下方自動顯示：
   - Default Pawn Class = BP_SekiroCharacter
   - HUD Class = BP_SekiroHUD
6. Ctrl+S 存檔
```

### 步驟 2：按 Play 測試

測試以下幾項：

| 測試項目 | 預期結果 |
|---------|---------|
| 主角 spawn | ✅ Reimu 在廣場中間，有正常動畫 |
| HUD | ✅ 有血條 + 技能欄 |
| Boss 出現 | ✅ BP_SekiroEnemy 在廣場另一邊 |
| Boss 移動 | ✅ Boss 向玩家走過來 |
| 攻擊/格擋 | ✅ 正常戰鬥 |

### 步驟 3（可選）：Boss 位置調整

如果 Boss 位置不對（太遠/太近/在地下），告訴我，我用 MCP 調整。

---

## ⏳ 等你測試後回報結果

完成步驟 1-2 後，告訴我：
- ✅ 正常 → 任務完成
- ❌ 有問題 → 說明什麼問題，我繼續修

---

## 🔗 確認資產位置（已更新）

| 資產 | Content Browser 路徑 |
|------|---------------------|
| 神社場景 | `/Game/JapaneseShrine/Maps/Level_Environment` |
| 戰鬥測試場景 | `/Game/Maps/Map_CombatDemo` |
| Boss Blueprint | `/Game/BP_SekiroEnemy` ✅ |
| Character Blueprint | `/Game/BP_SekiroCharacter` ✅ |
| Game Mode | `/Game/BP_SekiroGameMode` ✅ |

---

_資料截至：2026-04-10 16:15 (HKT)_
