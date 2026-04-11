# 📋 今日改動記錄 — 2026-04-11（星期六）

_生成時間：2026-04-11 22:51 (HKT)_

---

## 🎯 今日目標

為 Boss（`BP_SekiroEnemy`，Patchouli VRM 模型）實現 **Perilous Attack（危字攻擊）系統**，以及修復在此過程中出現的 **T-Pose 問題**。

---

## 📁 Commit 記錄

**Commit：`2242756`**
> "搞唔到無法格擋的攻擊 先push" — 22:12 (HKT)

---

## ✅ 一、C++ 代碼改動

### 1. `SekiroEnemyAttributeComponent.h` _(+32 lines)_
**位置：** `Source/FYP/Public/Components/`

新增：
- `TArray<UAnimMontage*> PerilousAttackMontages` — Boss 危字攻擊 Montage 陣列（`UPROPERTY(EditAnywhere, BlueprintReadWrite)`，可在 BP Details 面板設定）
- `bool bPerilousAttackActive` — 標記危字攻擊進行中
- `void TryPerilousAttack()` — 嘗試觸發危字攻擊
- `void OnPerilousAttackMontageEnded(UAnimMontage*, bool)` — Montage 結束回調

---

### 2. `SekiroEnemyAttributeComponent.cpp` _(+137 lines)_
**位置：** `Source/FYP/Private/Components/`

新增：
- `TryPerilousAttack()` 實現：
  - 隨機從 `PerilousAttackMontages` 選一個播放
  - 透過 `Montage_Play` 播放到 `DefaultSlot`
  - 播放結束後清除 `bPerilousAttackActive` flag
  - BeginPlay 時顯示 🔵 Debug 訊息：`Perilous Montages loaded: X`
- `StartComboAttackCycle()` — 在 Combo 開始前有機率觸發危字攻擊
- Debug 訊息系統（紅色 `⚠ BOSS: PERILOUS ATTACK!`，黃色 Ended）

---

### 3. `SekiroDeflectComponent.cpp` _(+14 lines)_
**位置：** `Source/FYP/Private/Components/`

新增：
- **無法格擋（Perilous Attack）邏輯**：若攻擊帶有 `Perilous` Gameplay Tag，格擋無效，改為直接受傷

---

### 4. `SekiroHUD.cpp` _(+95 lines)_
**位置：** `Source/FYP/Private/UI/`

新增：
- **Perilous Attack 警告 UI** 整合
- 觸發危字攻擊時顯示警告 Widget（`WBP_PerilousWarning`）

---

### 5. `SekiroHUD.h` _(+16 lines)_
**位置：** `Source/FYP/Public/UI/`

新增：
- `ShowPerilousWarning()` / `HidePerilousWarning()` 函數宣告

---

## ✅ 二、Unreal 資產改動

### 動畫資產（新增）

| 資產 | 路徑 | 說明 |
|------|------|------|
| `A_Great_Sword_Slash` | `/Game/boss_anim_retarget/` | 大劍斬擊動畫（Retargeted 到 Patchouli） |
| `AM_PerilousAttack_Slash` | `/Game/boss_anim_retarget/` | 危字斬擊 Montage（Mannequin 版） |
| `AM_PerilousAttack_Sweep` | `/Game/boss_anim_retarget/` | 危字橫掃 Montage（Mannequin 版） |
| `AM_PerilousAttack_Thrust` | `/Game/boss_anim_retarget/` | 危字刺擊 Montage（Mannequin 版） |
| `AM_Perilous_Slash_Patchouli` | `/Game/boss_anim_retarget/` | ✅ **危字斬擊 Montage（Patchouli 骨架）** |
| `AM_Perilous_Thrust_Patchouli` | `/Game/boss_anim_retarget/` | ✅ **危字刺擊 Montage（Patchouli 骨架）** |

### 動畫資產（原始 FBX 匯入）

| 資產 | 路徑 | 說明 |
|------|------|------|
| `A_Great_Sword_Slash` | `/Game/boss_animation/` | 原始 FBX 匯入（Mixamo 骨架） |
| `A_Upward_Thrust` | `/Game/boss_animation/` | 原始 FBX 匯入（Mixamo 骨架） |
| `Great Sword Slash.fbx` | `/Game/boss_animation/` | 原始 FBX 文件 |
| `Upward Thrust.fbx` | `/Game/boss_animation/` | 原始 FBX 文件 |

### IK Retarget 資產

| 資產 | 說明 |
|------|------|
| `IKRig_Mixamo` | Mixamo 骨架 IK Rig |
| `IKRig_UE4Mannequin` | UE4 Mannequin IK Rig |
| `RTG_NewRetargeter` | Mixamo → Patchouli Retargeter |
| `RTG_Great_Sword_Slash` | 大劍斬擊 Retargeter |
| `RTG_Upward_Thrust` | 刺擊 Retargeter |

### AnimBlueprint 改動

| 資產 | 改動 |
|------|------|
| `ABP_SekiroCharacter_Patchouli` | ❌ **刪除**（被 setup_blendspace_locomotion 破壞後移除） |
| `ABP_SekiroEnemy_New` | ✅ **新建 + 修復**：重建 State Machine（Idle + Walk），AnimGraph: `StateMachine → Slot(DefaultSlot) → Output Pose` |

### UI 資產

| 資產 | 路徑 | 說明 |
|------|------|------|
| `WBP_PerilousWarning` | `/Game/UI/` | 危字攻擊警告 Widget Blueprint |

### 其他

| 資產 | 改動 |
|------|------|
| `IK_NewIKRig` | 新建 IK Rig |
| `BP_SekiroEnemy` | AnimClass 更新為 `ABP_SekiroEnemy_New` _(未 commit)_ |

---

## ✅ 三、Config 改動

### `Config/DefaultEngine.ini`
- 調整了引擎設定（具體項目待確認）

### `Config/DefaultGameplayTags.ini` _(+5 lines)_
- 新增 `Perilous` Gameplay Tag（用於標記無法格擋的攻擊）

---

## ⚠️ 四、今日遇到的問題與修復

| 問題 | 原因 | 解決方案 |
|------|------|----------|
| Boss T-Pose | `ABP_SekiroCharacter_Patchouli` 被破壞後刪除，AnimClass 遺失 | 新建 `ABP_SekiroEnemy_New`，修復 State Machine，重設 AnimClass |
| State Machine 空的 | 之前多次 MCP 操作覆蓋了 AnimGraph | `setup_locomotion_state_machine` 重建 Idle + Walk States |
| Boss instance 動畫模式錯誤 | 測試時用 `set_skeletal_animation` 把 Boss 切換成 SingleNode 模式 | 刪除舊 instance，重新 Spawn `BP_SekiroEnemy_C_1` |
| Perilous Attack 動畫不播放 | Boss instance 是在 CDO 設定 Montage 前 Spawn 的 | 刪除舊 instance，重新 Spawn 繼承最新 CDO |

---

## 🔴 五、未完成 / 待測試

- [ ] **Play 模式測試攻擊動畫** — ComboMontages 自動載入邏輯需在 Play 模式觸發 BeginPlay 才能確認
- [ ] **Perilous Attack 警告 UI** — `WBP_PerilousWarning` 需確認在危字攻擊時正確顯示
- [ ] **無法格擋邏輯** — `SekiroDeflectComponent` 的 Perilous Tag 檢查需 Play 測試
- [ ] **未 commit 改動**：`ABP_SekiroEnemy_New.uasset`、`BP_SekiroEnemy.uasset`、`task.md`

---

## 📊 今日改動統計

| 類別 | 數量 |
|------|------|
| C++ 文件改動 | 5 個 |
| 新增代碼行 | ~294 行 |
| 新增 Unreal 資產 | ~60 個 |
| 刪除資產 | 1 個（ABP_SekiroCharacter_Patchouli） |
| Git Commit | 1 個（`2242756`） |
| 未 commit 文件 | 3 個 |

---

## 🔢 系統架構（修復後）

```
BP_SekiroEnemy（Boss）
├── CharacterMesh0（SkeletalMesh: Patchouli VRM）
│   └── AnimClass: ABP_SekiroEnemy_New
│       ├── State Machine: Idle ↔ Walk（速度驅動）
│       ├── Slot: DefaultSlot（播放 Montage 覆蓋）
│       └── Output Pose
├── SekiroCombatComponent
│   └── ComboMontages: 自動載入 Combo_Attack_01_01~04_Patchouli
├── SekiroEnemyAttributeComponent
│   ├── PerilousAttackMontages[0]: AM_Perilous_Slash_Patchouli
│   ├── PerilousAttackMontages[1]: AM_Perilous_Thrust_Patchouli
│   └── TryPerilousAttack() → Combo Cycle 開始前觸發
└── SekiroDeflectComponent
    └── Perilous Tag → 格擋無效，直接受傷
```
