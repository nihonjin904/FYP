# Phase 4: Checkpoint Core & Upgrade UI

## Decisions
- **Interaction Trigger**: `GroupActor0` will be converted into a formal Blueprint Actor (`BP_Checkpoint`). It will house a collision component (Trigger Box/Sphere) to listen for player overlap.
- **Interaction UI**: Upon entering the trigger box, display a Genshin-style "Press F to Interact" prompt anchored to the bottom-right of the screen. Hide this prompt on overlap end.
- **State Machine**:
  - Unactivated state: Pressing 'F' activates the checkpoint permanently.
  - Activated state: Pressing 'F' opens the Upgrade Menu.
- **Game State & Camera**: Opening the upgrade menu will pause the engine (`SetGamePaused=True`). The interface will unlock the mouse cursor (`SetInputModeUIOnly`). The camera will seamlessly transition from the player to a dedicated checkpoint camera using `SetViewTargetWithBlend`.
- **Attribute Backend**: Checkpoint upgrades will NOT destructively modify the original base values in the Attribute Component. Instead, upgrades apply to an additive/multiplicative modifier layer (e.g., `BaseHealth + CheckpointHealthBonus`), maintaining core architecture cleanliness.

## Deferrals
- Visual effects or particle emitters around the checkpoint activation are deferred unless purely functional placeholders.
- Resource cost/currency deduction during upgrades is deferred (upgrades are currently free for prototyping).
