"""
Step 4 (v2): Add AttackWindow notifies to all retargeted Patchouli combo montages.
Uses AnimationBlueprintLibrary API directly (avoids protected Notifies property).
Run in Unreal Editor: Output Log -> Python
"""
import unreal

# All retargeted combo montages that need attack windows
MONTAGES = [
    "/Game/Combo_Attack_01_01_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_02_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_03_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_04_Seq_Montage_Patchouli",
    "/Game/AM_Attack_03_Patchouli",
]

# Get the AttackWindow class
aw_class = None
try:
    aw_class = unreal.load_class(None, "/Script/FYP.AnimNotifyState_AttackWindow")
except:
    pass

if not aw_class:
    try:
        aw_class = unreal.AnimNotifyState_AttackWindow.static_class()
    except:
        pass

if not aw_class:
    unreal.log_error("Cannot find AnimNotifyState_AttackWindow class!")
    raise RuntimeError("Missing class")

unreal.log(f"Found AttackWindow class: {aw_class.get_name()}")

total = 0
for path in MONTAGES:
    montage = unreal.load_asset(path)
    if not montage:
        unreal.log_warning(f"Not found: {path}")
        continue
    
    # Get montage play length
    play_length = 0.0
    try:
        play_length = montage.get_editor_property("SequenceLength")
    except:
        pass
    if play_length <= 0:
        try:
            play_length = unreal.AnimationBlueprintLibrary.get_sequence_length(montage)
        except:
            play_length = 1.0
    
    # Attack window: 20% to 65% of animation
    start = play_length * 0.20
    duration = play_length * 0.45
    
    name = path.split("/")[-1]
    unreal.log(f"Adding AttackWindow to {name}: {start:.3f}s - {start+duration:.3f}s (length: {play_length:.3f}s)")
    
    try:
        result = unreal.AnimationBlueprintLibrary.add_animation_notify_state_event(
            montage, "1", start, duration, aw_class
        )
        total += 1
        unreal.EditorAssetLibrary.save_asset(path)
        unreal.log(f"  OK!")
    except Exception as e:
        unreal.log_error(f"  Failed: {e}")

unreal.log(f"=== Done! Added AttackWindow to {total}/{len(MONTAGES)} montages ===")
unreal.log("Now press Ctrl+Alt+F11 to compile C++ (weapon mesh fix)")
