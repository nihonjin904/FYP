---
wave: 1
depends_on: []
files_modified:
  - Source/FYP/Private/Characters/SekiroCharacter.cpp
autonomous: true
---

# Phase 1: Contextual UI Enhancement Plan

## Tasks

<task>
<description>Update OverheadWidget space definition to EWidgetSpace::World</description>
<read_first>Source/FYP/Private/Characters/SekiroCharacter.cpp</read_first>
<action>
In `Source/FYP/Private/Characters/SekiroCharacter.cpp`, find the `ASekiroCharacter::ASekiroCharacter()` constructor.

1. Replace `OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);` with `OverheadWidget->SetWidgetSpace(EWidgetSpace::World);`
2. Replace `DeathblowWidget->SetWidgetSpace(EWidgetSpace::Screen);` with `DeathblowWidget->SetWidgetSpace(EWidgetSpace::World);`

Since World Space renders the widget using Unreal length units (instead of standard relative 2D screen pixels), inject an explicit UI downscale right after setting the Widget Space:
`OverheadWidget->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));`
`DeathblowWidget->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));`
</action>
<acceptance_criteria>
- `Source/FYP/Private/Characters/SekiroCharacter.cpp` must contain `OverheadWidget->SetWidgetSpace(EWidgetSpace::World);`
- `Source/FYP/Private/Characters/SekiroCharacter.cpp` must contain `DeathblowWidget->SetWidgetSpace(EWidgetSpace::World);`
</acceptance_criteria>
</task>

## Verification
<must_haves>
- Source code successfully compiles after integrating the new scaling properties.
</must_haves>
