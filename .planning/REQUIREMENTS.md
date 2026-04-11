# Requirements

## 1. Contextual World Space UI
- **Goal:** Shift the enemy overhead widgets (health, posture, lock-on marker) from Screen Space projection to 3D World Space rendering.
- **Context:** Currently, Screen Space forces UI to remain a fixed pixel size regardless of distance, which ruins visual scale.
- **Criteria:** 
  - Ensure the Overhead Widget component is configured for `World` Space.
  - Scale parameters are adjusted to look correct from the camera's default lock-on distance.

## 2. Boss Special Attacks AI
- **Goal:** Enable the Boss (`BP_SekiroEnemy`) to dynamically pick special attacks like `Great Sword Slash` and `Upward Thrust`.
- **Context:** The C++ `SekiroCombatComponent` currently only manages a linear `ComboMontages` array. The boss lacks basic logical paths to fire off detached heavy attacks.
- **Criteria:**
  - Introduce an array of `SpecialAttacks` or update the randomized triggering logic within C++ or the Blueprint.
  - The Boss uses more than just the base Attack Combo montage pool during combat engagement.
