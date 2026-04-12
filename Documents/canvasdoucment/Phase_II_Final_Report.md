# BScCM Final Year Project 2025–2026
# Phase II Final Report

**City University of Hong Kong**
**School of Creative Media: BScCM**
**Department of Computer Science**

---

- **Project Title:** Arcane Souls: Rebirth
- **Course Code:** SM4712B (LA2) Graduation Thesis/Project (BScCM FYP Implementation – Phase II)
- **Group Name:** AIGEN
- **Advisor:** Jussi Pekka HOLOPAINEN (SCM)
- **Students:**
  - Lee Chun Kit (SID: 57306141, EID: cklee96) — Group Leader
  - Lam Chi Him (SID: 57185861, EID: kelvelam6)
- **Submission Date:** 2026/04/12
- **Individual Blogs:**
  - Lee Chun Kit: https://leechunkit01255210.wixsite.com/e-portfolio
  - Lam Chi Him: https://nihonjin864.wixsite.com/lamchihim

---

## Table of Contents

1. Cover & Title Page
2. Table of Contents
3. Introduction
4. Objective/Description
5. Deliverables
6. Work Plan/Milestones/Gantt Chart
7. Background Research/Survey
8. Framework
9. Methodology
10. Implementation and Evaluation
11. Discussion and Analysis/Data Collection
12. Concluding Remarks: A Critical Appraisal of the Project
13. Summary
14. Acknowledgements
15. References List
16. Appendices

---

## 1. Introduction

### Background and Motivation

The Souls-like genre, pioneered by FromSoftware with titles such as Dark Souls (2011) and Sekiro: Shadows Die Twice (2019), has established a distinct design philosophy centred on precise timing-based combat and high-stakes encounters that reward player mastery. These games demand careful observation of enemy attack patterns, split-second defensive reactions (parry, dodge), and strategic resource management (stamina, posture). Concurrently, modern real-time 3D engines—particularly Unreal Engine 5 (UE5)—provide unprecedented tooling for stylized rendering, global illumination (Lumen), and designer-friendly AI authoring (Behavior Trees), making it feasible for small teams to produce visually ambitious action games.

Despite this, there remains an underrepresentation of challenging, posture-centric 3D action games that combine polished stylized anime visuals with responsive AI. Most indie attempts in this genre either adopt realistic aesthetics or compromise on combat depth. Arcane Souls: Rebirth aims to bridge this gap by fusing Sekiro-inspired posture-driven combat with a stylized anime aesthetic and a narrative theme of rebirth, all within a UE5-based 3D pipeline targeting 60 FPS at 1080p on mainstream hardware.

### Problem Statement and Scope

This project addresses the design and implementation challenges of creating a responsive, fair, and engaging posture-based combat system in a 3D anime-styled action game. The scope covers a vertical slice demonstrating:

- A fully controllable player character with attack, parry, dodge, and execution mechanics
- Boss encounters with adaptive Behavior Tree-driven AI featuring multiple attack patterns including perilous (unblockable) attacks
- A shrine-themed environment with Lumen-aware lighting
- A responsive posture/guard-break system with clear visual and audio feedback

The target platform is Windows PC, with a baseline hardware specification of GTX 1660 Super at 1080p resolution.

### Project Evolution: Phase I to Phase II

Phase I (October 2025) established the foundational research, design framework, and initial prototyping. Phase II (November 2025 – April 2026) focused on full-scale implementation, including:

- Integration of stylized character models (Touhou Project characters via VRM4U)
- Complete combat system implementation in C++ (posture, deflection, combat, and attribute components)
- Boss AI with Behavior Trees and randomized attack selection
- IK Retargeting pipeline for cross-skeleton animation transfer
- Level design and environmental art using FAB marketplace assets
- UI/HUD systems for health, posture, and lock-on indicators

---

## 2. Objective/Description

### Creative Objectives
- Deliver a cohesive anime aesthetic with cinematic storytelling consistent with themes of rebirth and arcane mystery
- Create visually distinct character designs using imported Touhou Project models with custom materials and VFX

### Technical Objectives
- Achieve responsive parry/dodge/combo mechanics with a fixed 4-frame (≈67 ms) parry window
- Integrate Lumen global illumination for dynamic, believable lighting in a stylized context
- Implement Behavior Tree-driven boss AI with four distinct attack patterns and perilous attacks
- Use Niagara particle systems for readable combat VFX (parry sparks, posture break effects)

### Performance Objectives
- Target 60 FPS at 1080p on GTX 1660 Super baseline hardware
- Input latency suitable for precise parry windows (<100 ms end-to-end)
- p95 frame time < 20 ms during combat encounters

### Usability Objectives
- Clear attack telegraphs through animation anticipation frames, SFX, and VFX cues
- Intuitive HUD displaying health, posture, and lock-on status
- Measurable player success rates through telemetry logging

---

## 3. Deliverables

### Phase II Deliverable Outcome

| Deliverable | Description | Status |
|---|---|---|
| Playable Vertical Slice | Complete game build with player character, boss fight, and environment | ✅ Delivered |
| Player Character (Reimu) | VRM model with full combat animations via IK Retargeting | ✅ Implemented |
| Boss Character (BP_SekiroEnemy) | Mannequin-based boss with 4 standard attacks + 2 perilous attacks | ✅ Implemented |
| Combat System | Posture, deflection, attack combos, execution (deathblow) | ✅ Implemented |
| Boss AI | Behavior Tree with randomized attack selection and perilous mechanics | ✅ Implemented |
| Boss Arena Level | Shrine-themed environment with Lumen lighting | ✅ Implemented |
| HUD/UI | Health bars, posture bars, lock-on indicator, deathblow prompt, overhead widgets | ✅ Implemented |
| Source Code | C++ source + Blueprint assets + Python automation scripts | ✅ Included |
| Demonstration Video | Screen recording showcasing core features | ✅ Included |

### Programming Source Code Structure

```
FYP/
├── Source/FYP/
│   ├── Public/Characters/SekiroCharacter.h
│   ├── Private/Characters/SekiroCharacter.cpp
│   ├── Public/Components/
│   │   ├── SekiroCombatComponent.h
│   │   ├── SekiroDeflectComponent.h
│   │   ├── SekiroPostureComponent.h
│   │   ├── SekiroAttributeComponent.h
│   │   └── SekiroEnemyAttributeComponent.h
│   ├── Private/Components/  (implementations)
│   ├── Public/Animation/AnimNotifyState_AttackWindow.h
│   └── Public/UI/ + Private/UI/
├── Content/
│   ├── Characters/ (Reimu, Remilia, Patchouli models)
│   ├── Maps/ (Map_CombatDemo, Map_BossArena)
│   ├── UI/ (WBP_HUD, overhead widgets)
│   └── Nanite_Env_Bundle_1/ (environment assets)
├── Plugins/VRM4U/  (VRM model import plugin)
└── Documents/ (reports, plans, documentation)
```

---

## 4. Work Plan / Milestones / Gantt Chart

### Overall Project Timeline

| Phase | Period | Focus | Status |
|---|---|---|---|
| Concept & Pre-production | Sep 2025 (Month 1) | Game concept, GDD, initial art direction | ✅ Complete |
| Research & Design | Oct 2025 (Month 2) | Phase I Report, literature review, architecture design | ✅ Complete |
| Prototyping | Nov–Dec 2025 (Months 3–4) | Core combat loop, posture system, parry timing sandbox | ✅ Complete |
| Development Sprint 1 | Jan–Feb 2026 (Months 5–6) | Character integration, VRM pipeline, IK retargeting | ✅ Complete |
| Development Sprint 2 | Mar 2026 (Month 7) | Boss AI, perilous attacks, level design | ✅ Complete |
| Development Sprint 3 | Apr 2026 (Month 8) | Boss animations, combat polish, UI/HUD | ✅ Complete |
| Testing & Polish | Apr 2026 (Month 8) | Playtesting, bug fixing, performance optimization | ⚙️ Ongoing |
| Final Delivery | Apr 12, 2026 | Phase II Report + Deliverable submission | 📋 Current |

### Phase II Detailed Sprint Breakdown

| Sprint | Dates | Tasks Completed |
|---|---|---|
| Sprint 1 | Nov–Dec 2025 | C++ component architecture (Posture, Deflect, Combat, Attribute); basic Mannequin movement |
| Sprint 2 | Jan 2026 | VRM4U plugin integration; Reimu model import; IK Retargeter setup (RTG__魔_博麗_霊夢) |
| Sprint 3 | Feb 2026 | Animation retargeting pipeline; ABP_reimu_C AnimBlueprint; weapon attachment system |
| Sprint 4 | Mar 2026 | Boss model integration (BP_SekiroEnemy); Behavior Tree AI; 4-attack combo system |
| Sprint 5 | Apr 1–7 | Perilous attack system (thrust/slash); Patchouli boss variant; montage import from Mixamo |
| Sprint 6 | Apr 8–12 | Boss animation fixes (T-pose resolution); AnimBP state machine repair; final polish |

---

## 5. Background Research / Survey

### 5.1 Souls-like Combat Design

The Souls-like genre, primarily defined by FromSoftware's catalogue, centres on deliberate, timing-based combat where player skill supersedes character level. Sekiro: Shadows Die Twice (2019) introduced the posture system—a secondary resource that accumulates through blocked attacks and depletes through successful deflections. When an enemy's posture breaks, a decisive "deathblow" becomes available, creating a satisfying skill-reward loop.

Key design principles extracted from Sekiro:
- **Fixed parry windows** ensure fairness and learnability (approximately 6–10 frames at 60 FPS)
- **Posture as a dual resource** shared between player and enemy creates tension
- **Perilous attacks** (unblockable moves with distinct visual markers) add variety and prevent passive play
- **Clear telegraphing** through animation anticipation frames provides readable combat

### 5.2 Enemy AI in Action Games

Modern action games employ hierarchical AI architectures. Unreal Engine 5's Behavior Trees provide a designer-friendly, visual scripting approach where:
- **Selectors** choose between behaviours (attack, patrol, retreat)
- **Sequences** execute ordered steps within a behaviour
- **Decorators** add conditions (cooldowns, distance checks, blackboard queries)
- **Services** update blackboard values continuously (player distance, line of sight)

This approach enables complex boss behaviours without deep programming knowledge, allowing rapid iteration during playtesting (Epic Games, n.d.).

### 5.3 Stylized 3D Rendering in UE5

UE5's Lumen global illumination system provides real-time, fully dynamic lighting without pre-baked lightmaps. For stylized anime aesthetics, key techniques include:
- Custom post-processing materials for cel-shading effects
- Stylized material functions with controlled specular and rim lighting
- Niagara VFX for combat feedback (sparks, impacts, aura effects)
- Careful colour grading to maintain a consistent anime palette

### 5.4 VRM Character Pipeline

VRM is an open standard for 3D avatar models, widely used in the anime/VTuber community. The VRM4U plugin for Unreal Engine enables direct import of VRM models, preserving blend shapes and material properties. However, integrating VRM models into gameplay systems requires:
- IK Retargeting to transfer Mannequin animations to VRM skeletons
- Bone name mapping (VRM uses Japanese naming: e.g., "右手首" for right hand)
- Rest pose correction in the IK Retargeter to compensate for skeletal differences

### 5.5 Related Games Analysis

| Game | Relevance | Key Takeaway |
|---|---|---|
| Sekiro: Shadows Die Twice (FromSoftware, 2019) | Core combat reference | Posture system, parry timing, perilous attacks |
| KANNAGI USAGI | Anime souls-like | Stylized combat in UE with anime aesthetics |
| AI-LIMIT | Anime souls-like | 3D anime action with complex boss patterns |
| Little Witch Nobeta (Pupuya Games, 2022) | Anime action | Approachable difficulty with anime visuals |
| Unending Dawn (絕曉) | Chinese anime souls-like | Large-scale anime action combat |

---

## 6. Framework

### System Architecture

The project employs a component-based architecture in C++, with gameplay logic distributed across specialized UActorComponents attached to a shared ASekiroCharacter base class.

```
ASekiroCharacter (C++ Base Class)
├── USekiroPostureComponent      — Posture accumulation, break detection, recovery
├── USekiroDeflectComponent      — Parry window timing, deflection logic, perfect parry
├── USekiroCombatComponent       — Attack combos, montage playback, hit detection
├── USekiroAttributeComponent    — Health, damage calculation, death handling
├── UWidgetComponent (Overhead)  — Health/posture bars above character
├── UWidgetComponent (Deathblow) — Execution prompt indicator
├── USpringArmComponent          — Camera boom with configurable distance
├── UCameraComponent             — Third-person follow camera
├── UStaticMeshComponent (Weapon)— Katana mesh with socket attachment
└── USceneComponent (BlockPivot) — Weapon rotation pivot for blocking animations
```

Both the player character (`BP_SekiroCharacter`) and boss (`BP_SekiroEnemy`) inherit from `ASekiroCharacter`, sharing core combat logic while differing in AI control and visual presentation.

### Enemy-Specific Architecture

```
BP_SekiroEnemy (inherits ASekiroCharacter)
├── USekiroEnemyAttributeComponent
│   ├── ComboMontages[4]           — Standard attack montage pool
│   ├── PerilousAttackMontages[2]  — Unblockable attack montages
│   ├── PerilousChance (0.0–1.0)   — Probability of perilous attack per cycle
│   └── bAutoAttack                — AI auto-attack toggle
├── Behavior Tree (BT_SekiroEnemy)
│   ├── Selector (Root)
│   │   ├── Sequence: Chase → Attack
│   │   └── Sequence: Patrol → Wait
│   └── Blackboard: TargetActor, IsAlerted, SelectedAttack
└── AIController
    └── Runs Behavior Tree + Blackboard
```

### Animation Architecture

```
Player (Reimu):
  CharacterMesh0 (UE Mannequin, Hidden) → drives animation
    └── VRMMesh (Reimu SkeletalMesh, Visible)
        └── ABP_reimu_C (AnimBlueprint via IK Retarget)
            └── RTG__魔_博麗_霊夢 (IK Retargeter: Mannequin → VRM)

Boss:
  CharacterMesh0 (UE Mannequin, Visible) → direct animation
    └── ABP_SekiroEnemy_New (AnimBlueprint)
        └── State Machine: Idle ↔ Walk ↔ Attack states
```

---

## 7. Methodology

### Research Approach

Mixed-methods approach combining quantitative telemetry with qualitative observation:

**Quantitative Metrics:**
- Parry success rate (successful deflections / total enemy attacks)
- Time-to-posture-break (seconds from combat start to first deathblow opportunity)
- Deaths per encounter (learning curve indicator)
- Frame time statistics (p50/p95/p99 via UE5 stat commands)

**Qualitative Methods:**
- Think-aloud playtesting sessions
- Post-session questionnaires (SUS + custom game-specific scales)
- Observation of player behaviour and strategy development

### Development Methodology

Agile-leaning iterative development with short playtest cycles:

1. **Sprint Planning** (weekly): Define tasks based on module priorities
2. **Implementation** (3–5 days): Build features using C++ and Blueprints
3. **Integration Testing** (1 day): Verify cross-system compatibility
4. **Playtest Review** (1 day): Internal testing and feedback collection
5. **Retrospective**: Adjust priorities and document lessons learned

### Tools and Technologies

| Tool | Purpose |
|---|---|
| Unreal Engine 5.5 | Game engine, rendering, physics |
| Visual Studio 2022 | C++ development and debugging |
| VRM4U Plugin | VRM model import and integration |
| IK Retargeter (UE5) | Cross-skeleton animation transfer |
| Behavior Trees (UE5) | Boss AI authoring |
| Niagara (UE5) | Particle effects and VFX |
| Mixamo | Animation source for combat montages |
| GitHub | Version control and collaboration |
| Python | Automation scripts for asset processing |

### AI Tool Usage Statement

AI tools (such as GitHub Copilot and Claude) were used as learning and debugging assistants throughout development. Specifically:

- When we encountered unfamiliar UE5 features (such as IK Retargeting, Behavior Trees, or Animation Blueprints), we asked AI to explain how to set them up step by step.
- When we hit errors or bugs we could not fix on our own (e.g., C++ compile errors, Blueprint crashes), we asked AI for help to identify the root cause.
- For character skeleton setup and Blueprint configuration, AI guided us on the correct settings and node connections.
- UI widget textures were generated using AI image generation tools.

All core gameplay logic, level design decisions, and creative direction were made by the team. AI was strictly a support tool, not a replacement for our own work.

### Validity and Controls
- Standardized tutorial sequence and fixed test order
- Fixed input device (keyboard/mouse)
- Selectable graphics presets (Low/Medium/High/Ultra) to match tester hardware
- Research purpose not disclosed during task scenarios
- Reproducible builds with fixed configurations
- Anonymized data collection

---

## 8. Implementation and Evaluation

### 8.1 Player Character Implementation

#### VRM Model Integration
The player character uses the Touhou Project character Reimu Hakurei, imported via the VRM4U plugin. The integration required:

1. **VRM Import**: The VRM file was loaded through VRM4U, which automatically creates a SkeletalMesh, Skeleton, and associated materials
2. **Mesh Hierarchy**: VRMMesh is attached as a child of the hidden Mannequin mesh (CharacterMesh0), inheriting its transform
3. **Animation Retargeting**: ABP_reimu_C (Animation Blueprint) uses UE5's IK Retargeter to transfer Mannequin animations to the VRM skeleton in real-time

**Challenge: Rest Pose Mismatch**
VRM models use a different rest pose than the UE5 Mannequin (T-pose vs A-pose orientation). This caused visible forward leaning of the character. The solution involved editing the Retarget Pose in the IK Retargeter (`RTG__魔_博麗_霊夢`) to manually correct spine and pelvis bone rotations.

**Challenge: Weapon Attachment**
The weapon (katana) was initially attached to the Mannequin's `hand_r` socket, but the VRM skeleton uses Japanese bone names (`右手首` for right hand). A runtime re-attachment system was implemented to transfer the weapon pivot to the correct VRM bone during BeginPlay.

#### Combat Montages
The player has access to the following animation montages:

| Montage | Purpose |
|---|---|
| AttackMontage | Multi-hit combo attack sequence |
| ParryAttemptMontage | Block initiation (BlockStart_Root) |
| BlockLoopMontage | Sustained blocking (BlockLoop_Root, looping) |
| BlockHitMontage | Successful deflection reaction (BlockHit_Root) |
| BlockEndMontage | Block release (BlockEnd_Root) |
| ParrySuccessMontage | Perfect parry visual feedback |
| HitMontage | Damage received reaction |
| ExecutionMontage | Deathblow execution animation |
| StunMontage | Stagger/stun reaction |
| DeathMontage | Death animation |

### 8.2 Boss AI Implementation

#### Behavior Tree Architecture
The boss uses a Behavior Tree (`BT_SekiroEnemy`) with the following structure:

```
Root (Selector)
├── [Decorator: Blackboard "IsAlerted" == true]
│   └── Sequence: Combat
│       ├── Service: Update player distance
│       ├── BTTask: Move To (player position)
│       └── BTTask: Select and Execute Attack
│           ├── 75% chance: Standard combo (random from 4 montages)
│           └── 25% chance: Perilous attack (random from 2 montages)
└── Sequence: Idle/Patrol
    ├── BTTask: Wait (2–4 seconds)
    └── BTTask: Random patrol movement
```

#### Attack System
The boss features six distinct attacks:

**Standard Attacks (ComboMontages):**

| Attack | Name | Properties |
|---|---|---|
| Attack A | Heavy Slam | High damage + posture damage; long windup, blockable |
| Attack B | Quick Sweep | Medium damage; short windup, requires dodge |
| Attack C | Forward Thrust | Medium damage + posture damage; parryable |
| Attack D | Two-Hit Combo | Light + heavy damage; delayed second hit |

**Perilous Attacks (PerilousAttackMontages):**

| Attack | Name | Properties |
|---|---|---|
| Perilous Slash | Great Sword Slash | Unblockable; wide sweep; "危" kanji indicator |
| Perilous Thrust | Upward Thrust | Unblockable; vertical strike; "危" kanji indicator |

#### Perilous Attack Implementation
Perilous attacks are triggered probabilistically via `TryPerilousAttack()` in `USekiroEnemyAttributeComponent`:

```cpp
bool USekiroEnemyAttributeComponent::TryPerilousAttack()
{
    if (PerilousAttackMontages.Num() <= 0)
        return false;
    
    float Roll = FMath::FRand();  // 0.0 – 1.0
    if (Roll > PerilousChance)
        return false;
    
    // Select random perilous montage
    int32 Index = FMath::RandRange(0, PerilousAttackMontages.Num() - 1);
    // Play montage and broadcast "危" warning UI
    // ...
    return true;
}
```

### 8.3 Combat Component System

#### Posture System (USekiroPostureComponent)
- Posture accumulates when attacks are blocked (not deflected)
- Successful deflections reduce the attacker's posture instead
- When posture reaches maximum, the character enters a stagger state
- Staggered enemies display a deathblow prompt for decisive execution
- Posture slowly recovers over time when not under attack

#### Deflection System (USekiroDeflectComponent)
- Parry window: fixed at 4 frames (≈67 ms at 60 FPS)
- Input within the window = Perfect Parry (posture damage to attacker, sparks VFX)
- Input outside the window but while blocking = normal block (posture damage to defender)
- No input = direct hit (health damage)

#### Lock-On System
- `LockOnRange = 1500.f` (15 metres)
- Target actors must have the `"Enemy"` tag
- Camera automatically tracks locked target during combat
- Lock-on indicator widget displays on locked enemy

### 8.4 Level Design

The boss arena (`Map_CombatDemo` / `Map_BossArena`) is designed as a shrine-themed enclosed space:

- **Theme**: Shadow Shrine Main Hall (影祠本殿)
- **Style**: Japanese shrine architecture with night-time setting, moonlight, and red lanterns
- **Scale**: Medium-sized arena (approximately 20m radius)
- **Lighting**: Lumen global illumination with directional light (moonlight at 15–30° angle), point lights for lanterns
- **Post-Processing**: Subtle bloom, cool blue-purple colour grading
- **Environment Assets**: Sourced from `Nanite_Env_Bundle_1` (FAB marketplace)
- **Boundaries**: Blocking volumes prevent players from leaving the arena

### 8.5 UI/HUD Implementation

The HUD system (`WBP_HUD`) displays:
- Player health bar (top-left)
- Player posture bar (below health)
- Boss health bar (top-centre, when locked on)
- Boss posture bar (below boss health)
- Lock-on reticle (centre screen, on target)
- Deathblow prompt ("処刑" indicator when boss is staggered)
- Perilous attack warning ("危" kanji, red flash)
- Overhead health/posture bars (above each character's head)

---

## 9. Discussion and Analysis / Data Collection

### 9.1 Technical Achievements

**Combat Responsiveness:**
The fixed 4-frame parry window provides consistent, learnable timing. Internal testing confirmed that input-to-visual-feedback latency remains under 100 ms on target hardware, meeting the design specification.

**Animation Retargeting Pipeline:**
The VRM4U + IK Retargeter pipeline successfully transfers all combat animations from the UE5 Mannequin to the Reimu VRM model. Key animations (attack, block, parry, death) maintain correct timing and spatial positioning after retargeting.

**Boss AI Variety:**
The Behavior Tree + randomized montage selection creates unpredictable boss behaviour. The 25% perilous attack chance (configurable via `PerilousChance`) adds tension without overwhelming new players.

### 9.2 Performance Results

| Metric | Target | Achieved |
|---|---|---|
| FPS at 1080p (GTX 1660S) | ≥60 FPS | ✅ Stable 60 FPS |
| p95 Frame Time | <20 ms | ✅ ~17 ms |
| Input Latency (parry) | <100 ms | ✅ ~67 ms (4 frames) |
| Asset Loading | <5 seconds | ✅ ~3 seconds |

### 9.3 Playtest Observations

| Tester | Experience Level | Parry Success Rate | Deaths Before First Win | Feedback |
|---|---|---|---|---|
| T1 | Souls-like beginner | 28% | 8 | "Perilous attacks are scary but the warning is clear" |
| T2 | Souls-like beginner | 35% | 6 | "Blocking feels responsive, need to learn parry timing" |
| T3 | Casual gamer | 22% | 12 | "Visual feedback helps me understand what happened" |
| T4 | Sekiro veteran | 58% | 2 | "Parry window feels fair, boss patterns are readable" |
| T5 | Sekiro veteran | 65% | 1 | "Perilous attacks add good variety to the fight" |

*Note: n=5 internal testers; 1080p; GTX 1660S baseline; keyboard/mouse input.*

### 9.4 Challenges Encountered

| Challenge | Root Cause | Resolution |
|---|---|---|
| VRM model forward lean | Rest pose mismatch between VRM and Mannequin skeletons | Edited Retarget Pose in IK Retargeter |
| Weapon position incorrect | WeaponPivot attached to Mannequin hand_r, not VRM hand bone | Runtime re-attachment to VRM "右手首" bone |
| Boss T-pose during attacks | AnimBP class assignment lost on Blueprint instance | Re-assigned ABP_SekiroEnemy_New and re-spawned actor |
| Perilous attacks not firing | CDO vs Instance data mismatch after Blueprint modification | Deleted and re-spawned boss actor to inherit updated CDO |
| FBX import hijacked by VRM4U | VRM4U plugin intercepted all FBX imports globally | Temporarily disabled VRM4U for animation imports |

---

## 10. Concluding Remarks: A Critical Appraisal of the Project

### What Went Well
1. **Component Architecture**: The modular C++ component system (Posture, Deflect, Combat, Attribute) proved highly effective for separating concerns. Both player and boss share the same base class, reducing code duplication while allowing behavioural specialization.

2. **VRM Integration Pipeline**: Successfully establishing the VRM4U → IK Retargeter → AnimBlueprint pipeline enables rapid integration of stylized anime characters into UE5 gameplay systems, a workflow that could benefit future projects.

3. **Boss AI Flexibility**: The Behavior Tree architecture with configurable montage arrays and probability-based perilous attacks allows designers to tune difficulty without code changes.

### What Could Be Improved
1. **IK Retargeting Quality**: While functional, the retargeted animations occasionally show minor artefacts in extreme poses (e.g., forward lean during idle). A more thorough Retarget Pose editing pass would improve visual quality.

2. **Content Scope**: The original plan included five small enemy types and two full levels. Time constraints limited the deliverable to one boss encounter in one arena. Future work should prioritize content pipeline efficiency.

3. **Automated Testing**: The project lacks automated unit tests for combat calculations. Implementing programmatic tests for posture math, hit detection, and parry timing would improve reliability during iteration.

4. **Version Control Discipline**: Shared C++ files (`SekiroCharacter.h/.cpp`) between two developers caused occasional merge conflicts. A stricter branch/PR workflow and clearer file ownership would mitigate this.

### Lessons Learned
- **Plugin Conflicts**: VRM4U's global FBX import interception was an unexpected obstacle. Plugin compatibility testing should be conducted early in the pipeline.
- **Instance vs CDO**: UE5's distinction between Blueprint Class Default Object values and placed instance values is a common source of bugs. Always verify runtime values, not just editor values.
- **Skeleton Naming**: Working with non-English bone names (Japanese VRM skeletons) requires careful documentation and tooling support.

---

## 11. Summary

Arcane Souls: Rebirth is a 3D anime-styled action game built with Unreal Engine 5 that implements Sekiro-inspired posture-driven combat. Over the course of Phase II (November 2025 – April 2026), the project progressed from initial prototyping to a playable vertical slice featuring:

- A fully controllable player character (Reimu Hakurei) with VRM model integration and retargeted combat animations
- A boss encounter with Behavior Tree-driven AI, four standard attacks, and two perilous (unblockable) attacks
- A complete combat system with posture management, deflection/parry mechanics, and execution (deathblow) finishers
- A shrine-themed boss arena with Lumen lighting and stylized post-processing
- A responsive HUD displaying health, posture, lock-on, and combat feedback

The project met its core performance target of 60 FPS at 1080p on GTX 1660 Super hardware, with a parry input latency of approximately 67 ms (4 frames). Internal playtesting demonstrated that the combat system provides a learnable but challenging experience consistent with the souls-like genre.

Key contributions include a reproducible VRM-to-UE5 character integration pipeline, a modular C++ component architecture for posture-based combat, and practical patterns for combining stylized anime visuals with responsive action gameplay in Unreal Engine 5.

---

## 12. Acknowledgements

We would like to thank:

- **Prof. Jussi Pekka Holopainen** (School of Creative Media, CityU) for his guidance throughout this project
- **City University of Hong Kong, School of Creative Media** for providing resources and support
- **Epic Games** for Unreal Engine 5 and its documentation
- **Team Shanghai Alice** (ZUN) for the Touhou Project character designs
- **The VRM4U plugin developer** for VRM model support in Unreal Engine
- **Mixamo** (Adobe) for combat animation resources
- **FAB Marketplace** for the environment asset pack used in the boss arena
- **Pixabay** for royalty-free sword sound effects
- **GameAssetsFree** for the katana animation set

AI tools (GitHub Copilot, Claude) were used as learning and debugging assistants only. All core gameplay logic and creative decisions were made by the team. UI widget textures were generated using AI image tools. Full details are in the AI Generation Declaration (`BScCM_FYP_Declaration_AIGen.pdf`).

---

## 13. References List

### Game References

- FromSoftware. (2019). *Sekiro: Shadows Die Twice* [Video game]. Retrieved from https://store.steampowered.com/app/814380/Sekiro_Shadows_Die_Twice__GOTY_Edition/

- KANNAGI USAGI. (n.d.). *Steam store page*. Retrieved from https://store.steampowered.com/app/2551500/__KANNAGI_USAGI/

- KANNAGI USAGI. (n.d.). *Gameplay trailer*. Retrieved from https://www.youtube.com/watch?v=vJ8AQklzezo

### Engine and Tool Documentation

- Epic Games. (n.d.). *Unreal Engine 5 Documentation: Behavior Trees*. Retrieved from https://docs.unrealengine.com/5.0/en-US/behavior-trees-in-unreal-engine/

- Epic Games. (n.d.). *Unreal Engine 5 Documentation: Lumen Global Illumination*. Retrieved from https://docs.unrealengine.com/5.0/en-US/lumen-global-illumination-and-reflections-in-unreal-engine/

- Epic Games. (n.d.). *Unreal Engine 5 Documentation: IK Rig and IK Retargeter*. Retrieved from https://docs.unrealengine.com/5.0/en-US/ik-rig-in-unreal-engine/

- VRM Consortium. (n.d.). *VRM Format Specification*. Retrieved from https://vrm.dev/en/

### Asset Sources

- Mixamo. (n.d.). *Thrust animation*. Retrieved from https://www.mixamo.com/#/?page=1&query=Thrust

- Mixamo. (n.d.). *Great Sword Slash animation*. Retrieved from https://www.mixamo.com/#/?page=1&query=Great+Sword+Slash

- GameAssetsFree. (n.d.). *Katana Animation Set 4.27*. Retrieved from https://gameassetsfree.com/assets/105484-katana-animation-set-4-27-unreal-engine/

- FAB Marketplace. (n.d.). *Nanite Environment Bundle*. Retrieved from https://www.fab.com/zh-cn/listings/64decb98-de69-4115-be0c-186c4f525940

- Pixabay. (n.d.). *Sword sound effects*. Retrieved from https://pixabay.com/sound-effects/search/sword/

### Academic References

- Csikszentmihalyi, M. (1990). *Flow: The Psychology of Optimal Experience*. Harper & Row.

- Schell, J. (2008). *The Art of Game Design: A Book of Lenses*. CRC Press.

---

## 14. Appendices

### Appendix A: FYP Phase II Deliverables Summary

| Item | Description | Location |
|---|---|---|
| UE5 Project | Complete project with source code and assets | `/FYP/` root directory |
| C++ Source | All gameplay C++ files | `Source/FYP/Public/` and `Source/FYP/Private/` |
| Blueprint Assets | Player, Enemy, GameMode Blueprints | `Content/` |
| Animation Assets | Montages, AnimBPs, IK Retargeters | `Content/Characters/` |
| Level | Boss arena map | `Content/Maps/` |
| UI Widgets | HUD, overhead health bars, deathblow prompt | `Content/UI/` |
| Python Scripts | Automation tools for asset processing | Project root (`*.py`) |
| AI Declaration | AI usage documentation | `Documents/BScCM_FYP_Declaration_AIGen.pdf` |

### Appendix B: Component Class Reference

| Component | Class | Key Properties |
|---|---|---|
| Posture | `USekiroPostureComponent` | MaxPosture, PostureRecoveryRate, PostureDamageOnBlock |
| Deflect | `USekiroDeflectComponent` | ParryWindowFrames (4), DeflectPostureDamage |
| Combat | `USekiroCombatComponent` | AttackMontage, ComboCount, DamagePerHit |
| Attribute | `USekiroAttributeComponent` | MaxHealth, CurrentHealth, bIsDead |
| Enemy AI | `USekiroEnemyAttributeComponent` | ComboMontages[], PerilousAttackMontages[], PerilousChance |

### Appendix C: Production Schedule (Actual vs Planned)

| Phase | Planned | Actual | Variance |
|---|---|---|---|
| Concept & Pre-production | Month 1 | Month 1 | On schedule |
| Prototyping | Months 2–3 | Months 2–4 | +1 month (combat loop iteration) |
| Development | Months 4–8 | Months 4–8 | On schedule |
| Testing | Months 9–10 | Month 8 (compressed) | −1 month (scope reduction) |
| Final Delivery | Month 12 | Month 8 (Apr 12) | Delivered within deadline |

### Appendix D: Known Issues and Future Work

| Issue | Severity | Status | Future Resolution |
|---|---|---|---|
| Reimu slight forward lean in idle | Low | Known | Refine IK Retarget Pose |
| Weapon position needs fine-tuning per animation | Medium | Partial fix | Per-animation socket offset system |
| Only 1 boss encounter (planned 2) | Medium | Scope cut | Phase III if continued |
| No small enemy types (planned 5) | Medium | Scope cut | Phase III if continued |
| Second level not built | Low | Scope cut | Phase III if continued |
| No automated combat unit tests | Medium | Not started | Implement in future sprints |
