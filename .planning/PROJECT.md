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

### Active

<!-- Current scope. Building toward these. -->

- [ ] Transition overhead blood/posture UI elements from Screen Space to World Space for proper 3D perspective presence.
- [ ] Implement and verify advanced boss attack selection, specifically ensuring `Great Sword Slash` and `Upward Thrust` are actively utilized by the AI.

### Out of Scope

<!-- Explicit boundaries. Includes reasoning to prevent re-adding. -->

- Online multiplayer synchronization — not relevant for the core combat prototyping.

## Context

The enemy behaviors rely on hard-coded or C++ mapped execution patterns. Recent fixes allowed `SekiroEnemy` to assume its base animation properly, but its AI or Component arrays seemingly lack pointers to new skills like `Upward Thrust`.
Further, the existing `WBP_Overhead` component was functioning as a strict Screen Space HUD widget, breaking the immersion of proximity. 

## Constraints

- **Compatibility**: Must use UE 5.5 mechanisms, observing component lifetimes correctly regarding World Space components.
- **Python Verification**: Deep Unreal Asset debugging is managed by python pipelines rather than Blueprint introspection.

## Key Decisions

<!-- Decisions that constrain future work. Add throughout project lifecycle. -->

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Retargeting via Python | Keeps source files unmodified while standardizing the pipeline | ✓ Good |
| Manual BindToActor | Circumvents late initialization CDO bugs in Editor workflows | ✓ Good |

---
*Last updated: 04/11 after GSD Initialization and UI Bug Fixes*

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
