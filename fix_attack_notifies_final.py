"""
Add AnimNotifyState_AttackWindow to Patchouli combo montages 2, 3, 4.
Attack 1 already has it. Attack 2 needs a 2nd window. Attacks 3/4 need 1 window each.
Run AFTER restarting the editor with AnimationBlueprintLibrary plugin enabled.

Run in Unreal Output Log:
  py "C:/Unreal Projects/FYP/fix_attack_notifies_final.py"
"""
import unreal

# Find the C++ AttackWindow class
aw_class = unreal.load_class(None, "/Script/FYP.AnimNotifyState_AttackWindow")
if not aw_class:
    try:
        aw_class = unreal.AnimNotifyState_AttackWindow.static_class()
    except:
        pass

if not aw_class:
    print("ERROR: AnimNotifyState_AttackWindow class not found!")
    raise RuntimeError("Missing class - check FYP.uproject has AnimationBlueprintLibrary plugin enabled")

print(f"Found class: {aw_class.get_name()}")

# Config for each montage: (path, [(start_pct, end_pct), ...])
# Attack 1 already has its notify - skip
# Attack 2: Mannequin original had 2 hit windows
# Attack 3, 4: each needs 1 hit window
montage_configs = [
    ("/Game/Combo_Attack_01_02_Seq_Montage_Patchouli", [(0.15, 0.45), (0.50, 0.70)]),  # 2 windows
    ("/Game/Combo_Attack_01_03_Seq_Montage_Patchouli", [(0.20, 0.65)]),                 # 1 window
    ("/Game/Combo_Attack_01_04_Seq_Montage_Patchouli", [(0.20, 0.65)]),                 # 1 window
]

total = 0
for path, windows in montage_configs:
    montage = unreal.load_asset(path)
    if not montage:
        print(f"MISSING: {path}")
        continue

    # Get length
    try:
        length = montage.get_editor_property("SequenceLength")
    except:
        length = 0.0
    if length <= 0:
        length = unreal.AnimationBlueprintLibrary.get_sequence_length(montage)

    print(f"\n{montage.get_name()} (length={length:.3f}s)")

    for start_pct, end_pct in windows:
        start_t = length * start_pct
        dur_t = length * (end_pct - start_pct)
        print(f"  Adding window: {start_t:.3f}s - {start_t+dur_t:.3f}s")
        try:
            unreal.AnimationBlueprintLibrary.add_animation_notify_state_event(
                montage, "1", start_t, dur_t, aw_class
            )
            total += 1
            print(f"  OK!")
        except Exception as e:
            print(f"  FAILED: {e}")

    unreal.EditorAssetLibrary.save_asset(path)
    print(f"  Saved.")

print(f"\n=== Done! Added {total} notify windows ===")
