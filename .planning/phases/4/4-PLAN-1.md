---
wave: 1
depends_on: []
files_modified: []
autonomous: true
---

# Wave 1: Base Checkpoint Transformation & Prompt UI

<task>
<read_first>
- .planning/phases/4/4-CONTEXT.md
- C:/Unreal Projects/FYP/Content/Blueprints/Player/BP_PlayerCharacter.uasset (conceptually)
</read_first>
<action>
1. Create a new Blueprint Actor named `BP_Checkpoint` in `Content/Blueprints/Environment/`.
2. Delete the existing global `GroupActor0` outliner object in the current level and replace it with a single instance of `BP_Checkpoint`.
3. Inside `BP_Checkpoint`: Add a Static Mesh Component (give it a placeholder mesh like a simple fire or sword). Add a Sphere Collision Component scaled to around 300 units radius for the trigger zone. Add a Camera Component pointing at the mesh to be used for the UI transition later.
4. Create a new User Widget `WBP_InteractPrompt` in `Content/UI/`. It should have a simplistic Genshin-style text block saying "[F] Interact", anchored conceptually bottom-center/right.
5. In `BP_Checkpoint` Event Graph, connect `OnComponentBeginOverlap` and `OnComponentEndOverlap` from the Sphere Collision.
6. When the Player overlaps, Cast to `BP_PlayerCharacter`, spawn/add `WBP_InteractPrompt` to viewport. On EndOverlap, remove it. Set a boolean `bIsPlayerInZone` = true/false.
</action>
<acceptance_criteria>
- `BP_Checkpoint.uasset` exists in `Content/Blueprints/Environment/`.
- `WBP_InteractPrompt.uasset` exists in `Content/UI/`.
- Overlapping the Checkpoint shows the prompt; walking away removes it.
</acceptance_criteria>
</task>
