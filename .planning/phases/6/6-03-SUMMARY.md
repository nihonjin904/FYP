---
plan: 6-03
status: complete
completed: 2026-04-17
---

# Plan 6-03 SUMMARY: C++ Wiring — RT to Material + Arrowhead Rotation

## What Was Built

C++ wiring in `USekiroGameHUDWidget`:

- **Header** (`SekiroGameHUDWidget.h`):
  - Forward decls: `UImage`, `UMaterialInterface`, `UMaterialInstanceDynamic`
  - `UPROPERTY(meta=(BindWidgetOptional)) UImage* MinimapImage`
  - `UPROPERTY(meta=(BindWidgetOptional)) UImage* PlayerArrow`
  - `UPROPERTY(EditDefaultsOnly) UMaterialInterface* MinimapMaskMaterial`
  - Private `UMaterialInstanceDynamic* MinimapMID`

- **NativeConstruct** (`SekiroGameHUDWidget.cpp`):
  - Creates MID via `UMaterialInstanceDynamic::Create(MinimapMaskMaterial, this)`
  - Casts player pawn to `ASekiroCharacter`, calls `SetTextureParameterValue("MinimapTexture", MinimapRenderTarget)`
  - Applies MID to `MinimapImage` via `SetBrushFromMaterial()`
  - Sets brush size to 150×150

- **NativeTick**:
  - Gets player Yaw each frame
  - Calls `PlayerArrow->SetRenderTransformAngle(Yaw)` — North-up, arrowhead rotates with player

## Files Modified

- `Source/FYP/Public/UI/SekiroGameHUDWidget.h`
- `Source/FYP/Private/UI/SekiroGameHUDWidget.cpp`

## Self-Check: PASSED

- [x] Build exits 0
- [x] BindWidgetOptional used (won't crash if widget missing)
- [x] MID created once in NativeConstruct (not every tick)
- [x] Existing flash loop in NativeTick unchanged
- [x] TextureParameterValue name "MinimapTexture" matches M_MinimapMask parameter name

## key-files
created:
  - Source/FYP/Public/UI/SekiroGameHUDWidget.h (modified)
  - Source/FYP/Private/UI/SekiroGameHUDWidget.cpp (modified)
