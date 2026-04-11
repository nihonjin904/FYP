# Directory Structure

## Source/FYP
The C++ codebase strictly adheres to Unreal Engine's Public/Private monolithic separation, further segregated into logical groups:
- **`Animation/`**: Anim Notifies and state machine classes (e.g. `AnimNotifyState_AttackWindow`).
- **`Characters/`**: Core actors like `SekiroCharacter`.
- **`Components/`**: State and logic actor components (e.g. `SekiroAttributeComponent`).
- **`Core/`**: GameModes and overall orchestrators.
- **`UI/`**: UMG C++ base classes (`SekiroWidgetBase`, `SekiroHUD`).

## Editor Scripts (Root Directory)
Located in the project root are various Python scripts (`*.py`). These form a suite of editor automation tools:
- `diagnose_*.py`: Diagnostic checking scripts for ensuring properties and blueprints are well-formed.
- `batch_retarget_*.py`: Pipeline scripts to automatically assign materials, animations, and perform IK Retargeting.
- `fix_*.py`: Ad-hoc automatic debugging and asset fixing solutions.

## Content/
Visual assets (Blueprints, Mannequins, Particle FX). Notably:
- **`/Game/Characters`**: Separated per source (e.g. `reimu`, `patchouli`).
- **`/Game/boss_anim_retarget`**: Explicit directories for runtime retargeted animation generation.
