"""
Diagnose: Check ComboMontages notify counts in original vs Patchouli montages.
Run in Unreal Output Log: exec(open(r'C:/Unreal Projects/FYP/diagnose_notifies.py').read())
"""
import unreal

def check_montage(path):
    m = unreal.load_asset(path)
    if not m:
        print(f"  MISSING: {path}")
        return
    length = m.get_play_length()
    print(f"\n  [{m.get_name()}] length={length:.3f}s")
    
    # Try to access notifies via Python reflection
    try:
        notifies = m.get_editor_property("notifies")
        print(f"  Notifies count: {len(notifies)}")
        for n in notifies:
            print(f"    - {n.get_editor_property('notify_state_class') if hasattr(n, 'get_editor_property') else n}")
    except Exception as e:
        print(f"  Could not read notifies: {e}")
    
    # Try notify track
    try:
        tracks = m.get_editor_property("notify_tracks")
        print(f"  Notify tracks: {len(tracks)}")
    except Exception as e2:
        print(f"  Could not read tracks: {e2}")

print("=== Original Mannequin Montages ===")
for i in range(1, 5):
    check_montage(f"/Game/Combo_Attack_01_0{i}_Seq_Montage")

print("\n=== Patchouli Retargeted Montages ===")
for i in range(1, 5):
    check_montage(f"/Game/Combo_Attack_01_0{i}_Seq_Montage_Patchouli")

# Also check ComboAttackCount in BP_SekiroEnemy
print("\n=== BP_SekiroEnemy EnemyAttributeComponent ===")
try:
    bp = unreal.load_asset("/Game/BP_SekiroEnemy")
    if bp:
        cdo = unreal.get_default_object(bp.generated_class())
        enemy_attr = cdo.find_component_by_class(unreal.SekiroEnemyAttributeComponent) if hasattr(unreal, 'SekiroEnemyAttributeComponent') else None
        if enemy_attr:
            print(f"  ComboAttackCount: {enemy_attr.get_editor_property('combo_attack_count')}")
            print(f"  ComboAttackInterval: {enemy_attr.get_editor_property('combo_attack_interval')}")
            print(f"  bAutoAttack: {enemy_attr.get_editor_property('b_auto_attack')}")
        else:
            print("  SekiroEnemyAttributeComponent not found via Python (C++ class)")
    else:
        print("  Could not load BP_SekiroEnemy")
except Exception as e:
    print(f"  Error: {e}")
