# Phase 1 Context

## Domain
Contextual UI Enhancement (Overhead Widget Space Calibration)

## Locked Decisions
- **Widget Space:** Ensure the enemy health / posture widgets are strictly configured to "World Space" rather than "Screen Space."
- **Scale and Alignment:** With the widget now in World Space, base sizing and positioning offsets must be naturally calibrated relative to character capsules so they sit nicely above hostile entities' heads and scale naturally as the camera zooms/pans.

## Prior Constraints
- Retain the newly implemented `BindToActor` manual bindings, preventing decoupling.

## Canonical Refs
- No specific external UX documentation; behavior should match standard 3D action game overhead health bars (perspective shrinking, distance scaling).

## Deferred Ideas
None.
