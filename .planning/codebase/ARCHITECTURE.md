# Architecture Overview

## Top-Level Entity Patterns
The project strictly segregates character logic from game rules via Unreal Engine's Component paradigm.

### Characters
- `ASekiroCharacter`: The base class that instantiates all necessary components upon loading. Holds visual and physical states (Mesh, Capsule).

### Actor Components (The Core Logic)
All modular responsibilities lay here:
1. `USekiroAttributeComponent` (and `USekiroEnemyAttributeComponent`): Manages health and posture pools. Handles damage application.
2. `USekiroCombatComponent`: Processes weapon logic, hit traces.
3. `USekiroPostureComponent`: Specifically isolates posture mechanics.
4. `USekiroDeflectComponent`: Decouples deflection logic and parry timing.

### Animation and Events
- `AnimNotifyState_AttackWindow`: A custom animation notify state that controls the precise frame intervals where attacks generate hitboxes.

### Data Flow
1. **Inputs** trigger Character actions.
2. Action triggers **Animation Montage**.
3. Montage contains **AnimNotifies** (e.g. AttackWindow).
4. Notifies trigger `SekiroCombatComponent` line-traces.
5. Hits route to the target's `SekiroAttributeComponent`.
6. Attribute modifications fire Delegates (`OnHealthChanged`), which independently update the `USekiroWidgetBase` instances.
