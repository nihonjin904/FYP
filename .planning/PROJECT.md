# FYP Sekiro Mechanics Game

## What This Is

An Unreal Engine 5.5 action combat prototype featuring deep parry/posture mechanics and dynamic anime-style bosses. The project leverages components to modularize combat rules and visual systems.

## Core Value

Visceral combat synchronization. The enemy overhead UI and attack responsiveness must strictly match the timing of their skeletal animations.

## Requirements

### Validated

<!-- Shipped and confirmed valuable. -->

- ✓ Player and Enemy attribute logic decoupled into distinct Actor Components.
- ✓ A-pose animation bug officially repaired via C++ Class mapping automation.
- ✓ Dynamic UI linking verified via `BindToActor` manual bindings.
- ✓ Transition overhead blood/posture UI elements from Screen Space to World Space for proper 3D perspective presence. — v1.0
- ✓ Implement and verify advanced boss attack selection, specifically ensuring `Great Sword Slash` and `Upward Thrust` are actively utilized by the AI. — v1.0

### Active

<!-- Current scope. Building toward these. -->

**Current Milestone: v1.1 World Navigation & Progression**
- [ ] Implement an interactable `GroupActor0` checkpoint system to serve as a progression & respawn anchor.
- [ ] Add upgrade UI mechanisms for Max Health, Attack, and Posture upon checkpoint interaction.
- [ ] Build a robust Top-Left HUD Minimap detailing local player position and facing direction persistently.
- [ ] Establish a full-screen togglable ('M') fast-travel map indicating active checkpoints and allowing immediate teleportations.

### Out of Scope

<!-- Explicit boundaries. Includes reasoning to prevent re-adding. -->

- Online multiplayer synchronization — not relevant for the core combat prototyping.

## Context

Shipped v1.0 Initial Prototype, successfully addressing the crucial integration pains for the Boss overhead UI and Special Animation routing.
Fixed critical AI self-canceling issues where legacy standard combo fallbacks would aggressively interrupt perilous special attacks mid-animation. Conclusively proved that Unreal Engine visual misalignments were driven by AnimMontage slot mismatches needing UI-level correction versus code bugs.

## Constraints

- **Compatibility**: Must use UE 5.5 mechanisms, observing component lifetimes correctly regarding World Space components.
- **Python Verification**: Deep Unreal Asset debugging is managed by python pipelines rather than Blueprint introspection.

## Key Decisions

<!-- Decisions that constrain future work. Add throughout project lifecycle. -->

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Retargeting via Python | Keeps source files unmodified while standardizing the pipeline | ✓ Good |
| Manual BindToActor | Circumvents late initialization CDO bugs in Editor workflows | ✓ Good |
| C++ 100% RNG Test | Bypasses gameplay statistics (30% logic) to reliably prove architectural triggers in isolation | ✓ Good |
| Stripping AI Combo Fallbacks | Eradicates the AI's ability to logically self-terminate long Perilous animations prematurely | ✓ Good |

---
*Last updated: 2026-04-12 after v1.0 milestone*

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state
