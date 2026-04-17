---
plan: 6-02
status: complete
completed: 2026-04-17
---

# Plan 6-02 SUMMARY: Circular Minimap UMG Widget Integration

## What Was Built

Manual editor work completed by user in Unreal Editor:

- **RT_Minimap**: Created 256×256 RTF_RGBA8 RenderTarget in `Content/UI/`
- **M_MinimapMask**: Created User Interface material with Translucent blend mode
  - `TextureSampleParameter2D` (name: "MinimapTexture") → Emissive Color
  - SphereMask/SmoothStep soft circular falloff → Opacity (feathered edge)
- **WBP_GameHUD**: Added minimap overlay
  - `MinimapOverlay`: Overlay widget, top-left anchor (20,20), 150×150px
  - `MinimapImage`: Image widget (150×150, Is Variable = true)
  - `PlayerArrow`: Image widget (24×24, centered, Is Variable = true, arrowhead texture)
- **BP_SekiroCharacter**: `MinimapRenderTarget` = RT_Minimap
- **WBP_GameHUD Class Defaults**: `MinimapMaskMaterial` = M_MinimapMask

## Self-Check: PASSED

- [x] RT_Minimap exists in Content/UI/
- [x] M_MinimapMask Material Domain = User Interface, Blend = Translucent
- [x] MinimapImage is Variable in WBP_GameHUD
- [x] PlayerArrow is Variable in WBP_GameHUD
- [x] MinimapMaskMaterial assigned in WBP_GameHUD Class Defaults

## key-files
created:
  - Content/UI/RT_Minimap.uasset (new)
  - Content/Materials/M_MinimapMask.uasset (new)
  - Content/UI/WBP_GameHUD.uasset (modified)
  - Content/BP_SekiroCharacter.uasset (modified)
