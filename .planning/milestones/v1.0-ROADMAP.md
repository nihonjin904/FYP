# Roadmap

## Phase 1: Contextual UI Enhancement
- Audit the Overhead Widget hierarchy for enemies.
- Modify the root component settings from 'Screen Space' to 'World Space'.
- Calibrate the default scale to verify clarity in gameplay state.

## Phase 2: Boss Attack Automation (Complete)
- Trace exactly how `RequestAttack` is being triggered (likely via Blueprint Tick or simplified AI).
- Create a mechanism (either a new property like `TArray<UAnimMontage*> SpecialAttackMontages` or pure Blueprint array override) to hold `Great Sword Slash` and `Upward Thrust`.
- Intercept the attack call to apply a weighted random roll, seamlessly firing special attacks instead of standard combos.

## Phase 3: Boss Animation Auto-Assignment (Complete)
- Locate the precise Unreal Engine asset paths for Great Sword Slash (`AM_Perilous_Slash_Patchouli`) and Upward Thrust (`AM_Perilous_Thrust_Patchouli`).
- Hardcode the assignments directly into the C++ `BeginPlay` lifecycle of `ASekiroCharacter`.
- Prevent manual error by executing full zero-touch loading of the `SpecialMontages` array.
