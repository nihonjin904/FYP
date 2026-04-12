---
wave: 2
depends_on: [".planning/phases/4/4-PLAN-1.md"]
files_modified: []
autonomous: true
---

# Wave 2: Upgrade Menu UI & State Management

<task>
<read_first>
- .planning/phases/4/4-CONTEXT.md
</read_first>
<action>
1. Create a new User Widget `WBP_UpgradeMenu` in `Content/UI/`. Add three simplistic Buttons with Text (Health+, Attack+, Posture Slow) and a Close button.
2. Inside `BP_PlayerCharacter` or `BP_Checkpoint` (whichever owns the Input): Bind the 'F' key input Action. Ensure it only fires if `bIsPlayerInZone` is true.
3. Checkpoint Logic: Add a boolean `bIsActivated`. 
   - If false: First 'F' press sets `bIsActivated = true`. Play an activation sound/effect.
   - If true: Next 'F' press triggers `OpenUpgradeMenu`.
4. Inside `OpenUpgradeMenu`:
   - Create and Add `WBP_UpgradeMenu` to viewport.
   - Execute `SetGamePaused(True)`.
   - Execute `SetInputModeUIOnly()` and unlock the mouse cursor on the PlayerController (`bShowMouseCursor = true`).
5. Camera Transition: Use the `SetViewTargetWithBlend` node on the Player Controller, blending from the Player Character target to the `BP_Checkpoint` target (which will naturally pick up the Checkpoint's Camera Component). Blend time ~0.8s.
6. Closing Menu: Hitting Close reverses everything (`SetGamePaused(False)`, `SetInputModeGameOnly`, hide cursor, `SetViewTargetWithBlend` back to Player).
</action>
<acceptance_criteria>
- `WBP_UpgradeMenu.uasset` exists in `Content/UI/`.
- Pressing F inside the zone while unactivated activates the checkpoint.
- Pressing F while activated opens the menu, pauses the game, unlocks the mouse, and smoothly interpolates the camera to the checkpoint.
- Closing the menu smoothly restores normal gameplay.
</acceptance_criteria>
</task>
