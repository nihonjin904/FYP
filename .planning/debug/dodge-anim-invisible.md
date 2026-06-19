# DEBUG: dodge-anim-invisible
**Status:** INVESTIGATING  
**Date:** 2026-05-03

---

## Symptoms
- Expected: Reimu plays visible dodge animation when Left Shift pressed
- Actual: Character "teleports" slightly, ZERO visible animation
- PlayAnimMontage returns duration=1.333 (not 0) — montage IS valid for skeleton

---

## Confirmed Facts (已排除)
- [x] duration=1.333 → montage valid for SKEL__魔_博麗_霊夢 ✅
- [x] AnimGraph: StateMachine → Slot(DefaultGroup.DefaultSlot) → Output Pose ✅
- [x] EventGraph: only sets Speed & IsBlocking_Final, no interference ✅
- [x] Root motion on Doge_Combat_B_Seq_Reimu = false ✅
- [x] StopAllMontages called BEFORE PlayAnimMontage ✅
- [x] AM_Dodge_Reimu montage slot = DefaultGroup.DefaultSlot (matches AnimBP) ✅

---

## Hypotheses

### H1: Compile did NOT complete [HIGH PRIORITY]
- [DODGE-DBG] log never appeared in Output Log
- MCP log reader also failed to access log file
- **Test:** Ctrl+Alt+F11 → compile → check Output Log for `[DODGE-DBG] AnimInst class=`

### H2: ABP_Post__魔_博麗_霊夢 overriding visual [MEDIUM]
- Found: `/Game/Characters/reimu/ABP_Post__魔_博麗_霊夢.uasset` (114KB)
- VRM post-process ABP for hair/cloth physics
- SHOULD only modify secondary bones, not main body
- **Test:** Check if it's set as PostProcessAnimBlueprint on SK__魔_博麗_霊夢

### H3: Retarget quality too poor [HIGHEST CONFIDENCE]
- Doge_Combat_B_Seq is full-body lateral sword dodge
- RTG_UE4__魔_博麗_霊夢 maps Mannequin → VRM bones (approx.)
- Hit animations (upper body) work → but FULL BODY dodge might not transfer
- Evidence: root bone disabled → only relative body lean matters
  → if pelvis/spine mapping is poor, animation looks like idle
- **Test:** Double-click AM_Dodge_Reimu → see preview animation

### H4: Wrong AnimInstance [LOW — but decisive if H1 confirmed]
- PlayAnimMontage(DodgeMontage, 0.1f) added at line 1810
- If compile completes: character should be FROZEN in first pose for 13s
- If still nothing at 0.1x → AnimInstance is WRONG (plays to wrong mesh)

---

## Next Actions (in order)
1. Compile (Ctrl+Alt+F11) → wait for "Compile Complete"
2. Play → press Left Shift → observe for 5+ seconds
3. Output Log → search `DODGE-DBG` → send me AnimInst class= value
4. If retarget confirmed bad: use different animation (e.g., use Reimu's own VRM animations directly)
