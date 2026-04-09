"""
Diagnose weapon material differences between Enemy and Player.
Run in PIE (Play) mode for runtime actor data, or in editor for CDO data.
"""
import unreal

def check_weapon_material(bp_path, label):
    unreal.log(f"\n{'='*50}")
    unreal.log(f"=== {label}: {bp_path} ===")
    unreal.log(f"{'='*50}")
    bp = unreal.load_asset(bp_path)
    if not bp:
        unreal.log_error(f"  Cannot load {bp_path}")
        return
    cdo = unreal.get_default_object(bp.generated_class())
    
    # Find WeaponMesh
    wm = cdo.get_editor_property("WeaponMesh")
    if not wm:
        unreal.log_warning("  WeaponMesh: NOT FOUND")
        return
    
    # Static mesh
    sm = wm.get_editor_property("StaticMesh")
    unreal.log(f"  StaticMesh: {sm.get_path_name() if sm else 'None'}")
    
    # Scale
    scale = wm.get_editor_property("RelativeScale3D")
    unreal.log(f"  Scale: {scale}")
    
    # Materials
    num_mats = wm.get_num_materials()
    unreal.log(f"  Num materials: {num_mats}")
    for i in range(num_mats):
        mat = wm.get_material(i)
        if mat:
            unreal.log(f"  Material[{i}]: {mat.get_path_name()}")
        else:
            unreal.log(f"  Material[{i}]: None")
    
    # Override materials
    try:
        overrides = wm.get_editor_property("OverrideMaterials")
        unreal.log(f"  OverrideMaterials count: {len(overrides)}")
        for i, m in enumerate(overrides):
            unreal.log(f"    Override[{i}]: {m.get_path_name() if m else 'None'}")
    except:
        unreal.log("  OverrideMaterials: cannot read")

check_weapon_material("/Game/BP_SekiroEnemy", "ENEMY")
check_weapon_material("/Game/BP_SekiroCharacter", "PLAYER")

# Also check the Sword mesh itself for default material
unreal.log(f"\n{'='*50}")
unreal.log("=== Sword Static Mesh Default Material ===")
unreal.log(f"{'='*50}")
sword = unreal.load_asset("/Game/Sword_Animations/Demo/Mannequin/Character/Mesh/Sword")
if sword:
    mats = sword.get_editor_property("StaticMaterials")
    unreal.log(f"  StaticMaterials count: {len(mats)}")
    for i, sm in enumerate(mats):
        mi = sm.get_editor_property("MaterialInterface")
        unreal.log(f"    [{i}]: {mi.get_path_name() if mi else 'None'}")
else:
    unreal.log_warning("  Sword mesh not found!")
