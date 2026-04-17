---
plan: 6-01
status: complete
completed: 2026-04-17
---

# Plan 6-01 SUMMARY: SceneCapture2D Component + RenderTarget Setup

## What Was Built

Added a throttled orthographic top-down `USceneCaptureComponent2D` to `ASekiroCharacter`:

- Component attached to `RootComponent` at Z+3000, facing down (-90° pitch)
- `ProjectionType = Orthographic`, `OrthoWidth = 3000.f`
- `bCaptureEveryFrame = false` — manual throttle via Tick timer (10fps, 0.1s interval)
- Disabled ShowFlags: Fog, DynamicShadows, Bloom, AO, DepthOfField, MotionBlur
- `MinimapRenderTarget` exposed as `EditDefaultsOnly UPROPERTY` — assigned to `RT_Minimap` in `BP_SekiroCharacter`
- `BeginPlay`: assigns RT to `MinimapCapture->TextureTarget` and fires initial `CaptureScene()`

## Files Modified

- `Source/FYP/Public/Characters/SekiroCharacter.h` — forward decls, UPROPERTY fields, private timer
- `Source/FYP/Private/Characters/SekiroCharacter.cpp` — includes, constructor init, BeginPlay RT binding, Tick throttle

## Self-Check: PASSED

- [x] Build exits 0
- [x] `bCaptureEveryFrame = false`
- [x] RT assigned via Blueprint (not hardcoded path)
- [x] Expensive ShowFlags disabled

## key-files
created:
  - Source/FYP/Public/Characters/SekiroCharacter.h (modified)
  - Source/FYP/Private/Characters/SekiroCharacter.cpp (modified)
