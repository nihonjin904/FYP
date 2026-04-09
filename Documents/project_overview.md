# Arcane Souls: Rebirth — Project Overview
_FYP 2025-2026 | Kelvin Lam | UE5.5 | 最後更新：2026-04-09_

> **AI 每次開新 session 必讀此文件。讀完再動手，唔讀唔准做任何修改。**

---

## 🎮 項目簡介

Sekiro 風格的動作戰鬥遊戲，基於 UE5.5。

- **主角**：靈夢（Reimu）- VRM4U 模型，替換 Mannequin 顯示
- **敵人**：Boss 角色（`BP_SekiroEnemy`）
- **核心玩法**：精準格擋（Perfect Parry）、架勢系統（Posture）、鎖定、處決
- **合作開發**：Kelvin + 一位朋友（朋友主改核心 C++ 文件）

---

## 📁 目錄結構

```
FYP/
├── Source/FYP/
│   ├── Public/Characters/SekiroCharacter.h      ← 主角頭文件（朋友也在改）
│   ├── Private/Characters/SekiroCharacter.cpp   ← 主角實現
│   ├── Public/Components/                       ← 所有 Component 頭文件
│   ├── Private/Components/                      ← 所有 Component 實現
│   ├── Public/Animation/AnimNotifyState_AttackWindow.h
│   └── Public/UI/ + Private/UI/                 ← HUD, Widget
├── Documents/
│   ├── project_overview.md  ← 本文件（AI 必讀）
│   ├── plan.md              ← 開發計劃（用戶留言在此）
│   ├── task.md              ← 技術任務清單
│   └── unrealbullshit.md   ← AI 犯過的錯誤記錄
├── *.py                     ← 自動化 Python 腳本（已有，唔好重複建）
└── Plugins/VRM4U/           ← VRM4U 插件
```

---

## 🏗️ C++ 架構

### `ASekiroCharacter` (SekiroCharacter.h/.cpp)

**繼承**：`ACharacter`

#### 核心 Components（全部 C++ 建立）

| Component | 類型 | 用途 |
|---|---|---|
| `PostureComponent` | `USekiroPostureComponent` | 架勢(Posture)系統 |
| `DeflectComponent` | `USekiroDeflectComponent` | 格擋/彈刀邏輯 |
| `CombatComponent` | `USekiroCombatComponent` | 戰鬥邏輯 |
| `AttributeComponent` | `USekiroAttributeComponent` | 生命值/屬性 |
| `OverheadWidget` | `UWidgetComponent` | 頭頂UI |
| `DeathblowWidget` | `UWidgetComponent` | 處決提示UI |
| `CameraBoom` | `USpringArmComponent` | 相機臂 |
| `FollowCamera` | `UCameraComponent` | 跟隨相機 |
| `WeaponMesh` | `UStaticMeshComponent` | 武器（刀） |
| `BlockWeaponPivot` | `USceneComponent` | 擋刀旋轉軸 |

#### 武器掛載（當前問題所在）
```
Constructor / BeginPlay:
BlockWeaponPivot → AttachTo → GetMesh() (CharacterMesh0/Mannequin)
                               Socket: "hand_r"
WeaponMesh → 子 Component of BlockWeaponPivot
```
**⚠️ 問題**：`GetMesh()` = Mannequin（已 hidden），Reimu VRM 手位置≠Mannequin 手位置，所以刀位置錯。

#### 武器掛載正確目標
```
正確骨骼：VRMMesh → 骨骼名 "右手首"（日文，從 IK Rig Hierarchy 確認）
⚠️ 不是 J_Bip_R_Hand（那個不存在）
```

#### 鎖定系統
- `bIsLockedOn`, `LockedTarget` 記錄鎖定狀態
- `LockOnRange = 1500.f`（cm）
- `LockOnTargetTag = "Enemy"`（BP_SekiroEnemy 要有此 Tag）
- Tick 裡旋轉相機跟蹤目標

#### Montage 列表
| 變數名 | 用途 |
|---|---|
| `AttackMontage` | 攻擊 |
| `ParryAttemptMontage` | 擋刀開始（BlockStart_Root） |
| `BlockLoopMontage` | 持續擋刀（BlockLoop_Root，循環） |
| `BlockHitMontage` | 格擋成功（BlockHit_Root） |
| `BlockEndMontage` | 放開擋刀（BlockEnd_Root） |
| `ParrySuccessMontage` | 精準格擋火花 |
| `HitMontage` | 受傷 |
| `ExecutionMontage` | 處決 |
| `StunMontage` | 眩暈 |
| `DeathMontage` | 死亡 |

---

## 🎨 Blueprint 結構

### `BP_SekiroCharacter`（主角 BP，繼承 ASekiroCharacter）

```
BP_SekiroCharacter (Self)
├── CapsuleComponent
├── CharacterMesh0 (GetMesh()) ← Mannequin，Hidden In Game = true，動畫驅動源
│   ├── BlockWeaponPivot       ← 武器旋轉軸，attach 到 hand_r socket
│   │   └── WeaponMesh (刀)   ← M_Katana，Loc=(-12,3,0)，Rot=(-41°,11°,42°)
│   └── VRMMesh               ← 靈夢 SkeletalMesh，Rotation=(0,0,0)
│       Animation: ABP_reimu_C (Use Animation Blueprint)
├── CameraBoom
├── FollowCamera
├── PostureComponent
├── DeflectComponent
├── CombatComponent
├── AttributeComponent
├── OverheadWidget
└── DeathblowWidget
```

### `BP_SekiroEnemy`（敵人 BP）
- 同樣繼承 ASekiroCharacter（敵人和玩家共用同一個 C++ 類！）
- 無 VRMMesh（只有 Mannequin mesh）
- `bFacePlayerAsAI = true`

### Animation Assets
| Asset | 用途 |
|---|---|
| `ABP_reimu_C` | Reimu 動畫藍圖，用 IK Retarget 複製 Mannequin 動畫 |
| `RTG__魔_博麗_霊夢` | IK Retargeter：Mannequin → Reimu 骨骼映射 |

---

## 🐛 當前未解決問題

### 問題 A：Reimu 向前傾站

| 項目 | 內容 |
|---|---|
| 現象 | VRMMesh Rotation=(0,0,0) 後仍明顯向前傾 |
| 根因 | VRM rest pose 與 Mannequin 不同，retarget 後 spine/pelvis 累積偏差 |
| ❌ 禁止做 | 改 VRMMesh Component Rotation 來補正（會破壞跑步方向） |
| ✅ 正確做法 | 打開 `RTG__魔_博麗_霊夢` → Edit Retarget Pose → 手動調 spine/pelvis 骨骼角度 |
| 難度 | 需在 Editor 手動操作，MCP 無法控制 IKRetargeter |

### 問題 B/C/D：刀位置不對（含擋刀、跑步）

| 項目 | 內容 |
|---|---|
| 現象 | 刀掛在錯誤位置，Block 動作和跑步時刀位全錯 |
| 根因 | `BlockWeaponPivot` attach 到 Mannequin `hand_r`，Reimu 的手在 retarget 後位置不同 |
| 正確骨骼 | VRM 右手骨骼 = `右手首`（日文，從 IK Rig Hierarchy 確認） |
| 解決方案 | `SekiroCharacter.cpp` BeginPlay() 加 runtime re-attach 代碼 |
| ⚠️ 注意 | 改 SekiroCharacter.cpp 前必須確認朋友分支，避免 Git 衝突 |
| ⚠️ 注意 | attach 後 WeaponMesh Relative Transform 需要重新調整 |

---

## 🐍 Python 腳本（已有，唔好重新建）

| 腳本 | 功能 |
|---|---|
| `diagnose_bp.py` | 診斷 Blueprint 問題 |
| `diagnose_notifies.py` | 診斷 AnimNotify |
| `fix_attack_notifies.py/.v2.py/_final.py` | 修復攻擊 AnimNotify |
| `fix_tpose.py` | 修復 T-Pose 問題 |
| `batch_retarget_patchouli.py` | 批量 retarget（Patchouli 角色） |
| `setup_patchouli_bp.py` | 設置 Patchouli BP |
| `check_and_fix_enemy_animclass.py` | 檢查及修復敵人 AnimClass |

**⚠️ 每次建新 Python 腳本前先檢查以上清單，唔好重複建！**

---

## 🤝 GitHub 協作規則

1. **`SekiroCharacter.h` / `SekiroCharacter.cpp`** 是朋友也在改的文件
2. 改這些文件前必須確認朋友當前分支
3. 優先用 Blueprint/Component 方式解決，減少 C++ 衝突
4. 如果要改，用 Branch + PR 流程

---

## 🔧 MCP Server

- **只有 1 個**：`unreal-mcp`
- **使用前提**：Unreal Editor 必須開著才能連線
- **禁止行為**：
  - 探索式 calls（list assets 來「睇吓有咩」）
  - 用 MCP 改 Transform 值（直接在 Details 面板改更快）
  - 未確認 Editor 開著就 call MCP
- **合理用途**：讀取 Blueprint 結構、加 Blueprint 節點、確認 actor 屬性

---

## 📋 重要已知事實

| 事實 | 內容 |
|---|---|
| VRM 骨骼命名 | 日文。右手 = `右手首`，不是 J_Bip_R_Hand |
| UE5 Rotation 順序 | X=Roll，Y=Pitch，Z=Yaw |
| Mesh Component Rotation | 只影響視覺渲染，不影響動畫骨骼空間 |
| Rest pose 問題 | 永遠在 IKRetargeter Edit Retarget Pose 裡修，不是旋轉 mesh component |
| UE5.5 Live Coding | Ctrl+Alt+F11（不是 F7） |
| AnimTickOption=0 | = AlwaysTickPoseAndRefreshBones（正確設置） |
| VRMMesh 位置 | attach 到 CharacterMesh0（不是 self/capsule） |
| CharacterMesh0 Rotation | (0, -90, 0) 標準 UE5 |
