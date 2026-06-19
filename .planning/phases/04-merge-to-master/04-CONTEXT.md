# Phase 4 Context — Merge feature/perilous-attack-notify 去 master

**Discussion date:** 2026-04-20

---

<decisions>

## 1. Merge 方法
**決定：** GitHub Desktop 執行 merge
- 打開 GitHub Desktop → 切換到 master branch → 選 Branch menu → Merge into current branch → 選 feature/perilous-attack-notify
- GitHub Desktop 會自動偵測衝突，列出哪些文件需要手動解決
- 如果有衝突：GitHub Desktop 標記 → 用 VS Code 打開解決 conflict markers → 再回 GitHub Desktop 按 Continue Merge

**NOT using:** GitHub 網頁 PR merge / 本地 git merge 命令

---

## 2. SekiroHUD 衝突處理策略

**你改了什麼（從 diff 分析）：**
- `SekiroHUD.h`：加了 Constructor 聲明 + `PerilousWarningWidgetClass` + `PerilousWarningWidgetInstance`
- `SekiroHUD.cpp`：加了 Constructor 實現（載入 WBP_PerilousWarning）+ `OnPerilousAttackStarted()` 完整實現（Widget 顯示邏輯）
- `SekiroGameMode.cpp`：加了 `#include UI/SekiroHUD.h` + `HUDClass = ASekiroHUD::StaticClass()`

**朋友改了什麼（Phase 7 Full-Screen Map & Fast Travel）：**
- 可能在 SekiroHUD 加了地圖相關 UI 函數
- 可能在 SekiroGameMode 加了 Game Mode 初始化邏輯

**衝突解決原則：**
- **保留雙方所有改動**（不選一邊覆蓋另一邊）
- `.h` 衝突：兩邊的 UPROPERTY 和函數聲明都要保留
- `.cpp` 衝突：兩邊的函數實現都要保留，拼在一起

---

## 3. Blueprint .uasset 衝突策略

**決定：保留你的版本（Perilous Attack Montage 文件）**
- 你的 6 個 .uasset 都是危攻擊相關（combat animation），朋友的 Phase 7 是 Map/Fast Travel，幾乎不可能動同一批文件
- 預測衝突風險：**極低**
- 如果真的有衝突，選你的版本，然後通知朋友確認他的改動是否遺失

**關鍵：AnimNotify 在 .uasset 裡已保存**
- 你的 Montage AnimNotify 在 commit `74822ff` 已 push，只要選你的 .uasset 版本就不會遺失

---

## 4. Merge 時機

**決定：現在 merge**
- 雙方目前都沒有 uncommitted 改動（clean state）
- 這是最低風險的 merge 時機

---

## 5. Merge 後驗證清單

合併完成後，進入 UE5 Play Mode 驗證：

1. **危攻擊 UI**：Boss 發動危攻擊 → 確認「避」字出現再消失
2. **危攻擊傷害**：玩家格擋狀態下被危攻擊 → 確認仍然受傷
3. **地圖系統**（朋友功能）：打開全屏地圖 → 確認正常顯示
4. **Fast Travel**（朋友功能）：確認快速傳送正常工作
5. **架勢條**：精準擋刀架勢條增量正確（Phase 2 功能）

---

## 6. 萬一出錯了怎麼辦

**Git 永遠不會消失你的東西。** 每個 commit 都有記錄。

| 階段 | 出錯了 | 解決方法 |
|---|---|---|
| Merge **進行中**，未完成 | 看到衝突嚇到了 | GitHub Desktop → Abort Merge（中止，回原狀態）|
| Merge **剛完成**（幾秒內） | 發現不對 | GitHub Desktop → Undo（撤銷 merge commit）|
| Merge 完成**推了 push** | 功能壞了 | `git revert` 或 `git reset --hard HEAD~1` 然後 force push |
| 完全不知道從哪 | 完全亂了 | `git reflog` 找回任何一個歷史狀態 |

**最壞情況：** 找我，我幫你用 `git reflog` 回復到任何時間點的狀態。

</decisions>

<canonical_refs>
- Source/FYP/Public/UI/SekiroHUD.h
- Source/FYP/Private/UI/SekiroHUD.cpp
- Source/FYP/Private/Core/SekiroGameMode.cpp
- Content/boss_anim_retarget/AM_PerilousAttack_Slash.uasset
- Content/boss_anim_retarget/AM_PerilousAttack_Sweep.uasset
- Content/boss_anim_retarget/AM_PerilousAttack_Thrust.uasset
- Content/boss_anim_retarget/AM_Perilous_Slash_Patchouli.uasset
- Content/boss_anim_retarget/AM_Perilous_Thrust_Patchouli.uasset
- Content/ABP_SekiroEnemy_New.uasset
</canonical_refs>
