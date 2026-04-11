# Project Stack

## Core Technologies
- **Engine**: Unreal Engine 5.5
- **Languages**: C++ (Core Gameplay Logic), Blueprints (Visual Scripting & UI), Python 3.x (Editor Automation & Diagnosis)

## External Plugins
- **VRM4U**: Anime model loading and IK processing
- **MotionWarping**: Enhanced movement mapping during animations (e.g., thrusts and executions)
- **EnhancedInput**: Modern input mapping action framework
- **ProjectCleaner**: Asset management

## Architecture Patterns
- **Entity Component System (UE-flavor)**: The main character `SekiroCharacter` delegates logic strictly to isolated Actor Components (e.g. `SekiroCombatComponent`, `SekiroAttributeComponent`, `SekiroPostureComponent`).
- **Python Automation**: Routine tasks like retargeting, blueprint setups and state validation are driven by Editor Utility python scripts in the project root.
