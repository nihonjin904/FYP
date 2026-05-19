# STACK.md — 技術棧

## Engine
- **Unreal Engine 5.5** (C++ / Blueprint 混合)
- **Build System**: UnrealBuildTool (.Build.cs)

## Language
- **C++17** (UE5 標準)
- **Blueprint** (BP_SekiroEnemy, BP_PlayerCharacter, WBP_BossHUD 等)

## Core Modules (FYP.Build.cs)
- `Core`, `CoreUObject`, `Engine`, `InputCore`
- `EnhancedInput` — 新輸入系統 (IA_Attack, IA_Block, IA_LockOn 等)
- `GameplayTags` — 攻擊類型標記 (Action.Attack.Light, Attack.Perilous, State.Stunned)
- `MotionWarping` — 處決 Warp 動畫（概念階段）
- `Niagara` — 粒子特效 (NS_PerfectParrySpark, NS_BlockSpark)
- `UMG` — UI Widget 系統 (WBP_OVERHEAD, WBP_GameHUD)
- `SlateCore`, `Slate` — UI 底層

## Asset Types
| 類別 | 路徑範例 |
|------|---------|
| Sound | `/Game/sound_effect/Perfect_Parry_音效` |
| Niagara | `/Game/character_block_particle/NS_PerfectParrySpark` |
| Camera Shake | `/Game/character_block_particle/BP_PerfectParryShake` |
| AnimMontage | `/Game/Combo_Attack_03_0X_Seq_Montage_Reimu` (玩家) |
| AnimMontage | `/Game/boss_anim_retarget/AM_Perilous_Slash_Patchouli` (Boss) |
| Input | `/Game/ThirdPerson/Input/Actions/IA_Attack` |
| Widget | `/Game/WBP_OVERHEAD` |

## Character Skeletons
- **Reimu** — 玩家角色 (VRM 骨架，右手首 bone)
- **Patchouli** — Boss 敵人 (標準 Mannequin 骨架, hand_r socket)
