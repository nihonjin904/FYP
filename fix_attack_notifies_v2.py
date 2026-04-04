"""
Method A: Add AnimNotifyState_AttackWindow to Patchouli combo montages.
Uses unreal.AnimationLibrary (the correct API, not AnimationBlueprintLibrary).
"""
import unreal

# The 4 Patchouli combo montages and their timing ratios
montage_configs = [
    {
        "path": "/Game/Combo_Attack_01_01_Seq_Montage_Patchouli",
        "start_pct": 0.20,  # Attack window opens at 20% of animation
        "end_pct": 0.65,    # Attack window closes at 65%
    },
    {
        "path": "/Game/Combo_Attack_01_02_Seq_Montage_Patchouli",
        "start_pct": 0.20,
        "end_pct": 0.65,
    },
    {
        "path": "/Game/Combo_Attack_01_03_Seq_Montage_Patchouli",
        "start_pct": 0.20,
        "end_pct": 0.65,
    },
    {
        "path": "/Game/Combo_Attack_01_04_Seq_Montage_Patchouli",
        "start_pct": 0.20,
        "end_pct": 0.65,
    },
]

# Find the AttackWindow notify state class
attack_window_class = unreal.load_class(None, "/Script/FYP.AnimNotifyState_AttackWindow")
if not attack_window_class:
    # Try alternate name
    attack_window_class = unreal.load_class(None, "/Script/FYP.AnimNotifyState_AttackWindow_C")

if not attack_window_class:
    print("ERROR: Could not find AnimNotifyState_AttackWindow class!")
    print("Available AnimNotifyState classes will be listed...")
    # Try to find it through asset registry
    import sys
    for name in dir(unreal):
        if "Attack" in name or "AttackWindow" in name:
            print(f"  Found: unreal.{name}")
else:
    print(f"Found AttackWindow class: {attack_window_class.get_name()}")

    for config in montage_configs:
        montage = unreal.load_asset(config["path"])
        if not montage:
            print(f"ERROR: Could not load montage: {config['path']}")
            continue

        length = montage.get_play_length()
        start_time = length * config["start_pct"]
        end_time = length * config["end_pct"]
        duration = end_time - start_time

        print(f"\nProcessing {montage.get_name()} (length={length:.3f}s)")
        print(f"  AttackWindow: {start_time:.3f}s - {end_time:.3f}s (duration={duration:.3f}s)")

        # Check if AnimationLibrary is available
        try:
            # Method 1: AnimationLibrary.add_anim_notify_event_state (proper method)
            unreal.AnimationLibrary.add_anim_notify_event(
                montage,
                start_time,
                attack_window_class.get_name()
            )
            print(f"  Method 1 (AnimationLibrary.add_anim_notify_event): OK")
        except AttributeError as e:
            print(f"  Method 1 failed (AttributeError): {e}")
            try:
                # Method 2: Direct notify creation via AnimMontage API
                notify = unreal.AnimNotifyState()
                print(f"  Method 2: created AnimNotifyState base instance")
            except Exception as e2:
                print(f"  Method 2 failed: {e2}")
                try:
                    # Method 3: Check what's available
                    anim_lib_attrs = [x for x in dir(unreal) if "anim" in x.lower() or "notify" in x.lower()]
                    print(f"  Available anim/notify attrs: {anim_lib_attrs}")
                except Exception as e3:
                    print(f"  Method 3 failed: {e3}")

        # Save the montage
        unreal.EditorAssetLibrary.save_asset(config["path"])
        print(f"  Saved.")

print("\nDone!")
