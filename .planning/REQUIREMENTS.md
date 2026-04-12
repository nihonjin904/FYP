# Requirements

## Milestone v1.1: World Navigation & Progression

### Checkpoint System [CHK]
- [ ] **CHK-01**: Player can approach an unactivated checkpoint (identified as `GroupActor0` in the environment) and press 'F' to activate it.
- [ ] **CHK-02**: Player respawns at the most recently activated checkpoint upon death.
- [ ] **CHK-03**: Interacting with an activated checkpoint opens an Upgrade UI offering three choices: Increase Health, Increase Attack Power, or Slow Posture Accumulation (free for prototyping).

### Full-Screen Map & Fast Travel [MAP]
- [ ] **MAP-01**: Player can press 'M' to toggle a full-screen overview map displaying a top-down view of the level.
- [ ] **MAP-02**: Activated checkpoints are visually highlighted on the full-screen map.
- [ ] **MAP-03**: Unactivated checkpoints are visible on the map to hint at locations, but cannot be interacted with.
- [ ] **MAP-04**: Player can click on any highlighted, activated checkpoint on the full-screen map to instantly fast travel there.

### HUD Minimap [HUD]
- [ ] **HUD-01**: A circular minimap (approx 1/4 screen height) is persistently anchored to the top-left corner of the combat HUD.
- [ ] **HUD-02**: The minimap renders a consistent top-down projection of the current environment.
- [ ] **HUD-03**: Player current world location is tracked and rendered as a yellow dot on the minimap.
- [ ] **HUD-04**: Player current facing direction is rendered as a white, 75% transparent fan/cone attached to the yellow dot.

## Future Requirements (Deferred)
- Resource consumption system for upgrades (experience points/currency).
- Checkpoint visual/particle state changes upon activation.

## Out of Scope
- Map Fog of War (overview map reveals entire level immediately).
- Dynamic enemy tracking on the minimap.

## Traceability
| Phase | Requirement IDs |
|-------|-----------------|
| 4     | CHK-01, CHK-03  |
| 5     | CHK-02          |
| 6     | HUD-01, HUD-02, HUD-03, HUD-04 |
| 7     | MAP-01, MAP-02, MAP-03, MAP-04 |
