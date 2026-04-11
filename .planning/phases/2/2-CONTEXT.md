# Phase 2: Boss Attack Automation - Context

**Gathered:** 2026-04-12
**Status:** Ready for planning
**Source:** User Directive (Direct Override)

<domain>
## Phase Boundary
Implementation of an independent, randomized Boss Special Attack array inside `USekiroCombatComponent`, completely bypassing the rigid, linear sequential logic of `ComboMontages`. This is to resolve the critical issue where placing advanced skills like "Great Sword Slash" and "Upward Thrust" into the main combat array prevents them from executing randomly, and heavily scrambles standard attack sequencing.
</domain>

<decisions>
## Implementation Decisions

### Combat Component Architecture
- Introduce `TArray<UAnimMontage*> SpecialAttacks` to the `USekiroCombatComponent` header to properly hold high-impact abilities.
- Introduce an isolated randomizer mechanism within `RequestAttack()` that flips a weighted probability coin BEFORE checking combos.
- If the calculation surpasses the designated threshold, the logic independently triggers a randomly selected montage directly from the new `SpecialAttacks` array. 
- If the roll fails, or if the `SpecialAttacks` array is unpopulated, it will dynamically fall back to the standard linear `ComboMontages` sequence.
- Expose the probability scalar as an Editor property (`SpecialAttackChance`, default 30% / 0.3f) directly to Blueprints for instant designer tuning.

### The Agent's Discretion
- The implementation logic of Unreal's deterministic random seed algorithm (`FMath::FRand()`).
- Safeguard execution to ensure the standard combo indexing is reset internally when a special attack takes priority, preventing broken combo chain links on fallback.
</decisions>

<canonical_refs>
## Canonical References
- `Source/FYP/Public/Components/SekiroCombatComponent.h`
- `Source/FYP/Private/Components/SekiroCombatComponent.cpp`
</canonical_refs>

<specifics>
## Specific Ideas
- Resolves the UX flow where "Great Sword Slash" executes rigidly instead of dynamically.
- Delivers the "authentic unpredictable boss feeling" specifically requested by the developer through high/low RNG capability.
</specifics>
