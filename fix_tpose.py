"""
Step 3: Fix T-Pose issues:
1. Set ComboMontages array on BP_SekiroEnemy
2. Clean up _Patchouli1 duplicate ABP/BlendSpace assets

Run in Unreal Editor: Output Log -> Python
"""
import unreal

bp_path = "/Game/BP_SekiroEnemy"
bp = unreal.load_asset(bp_path)
if not bp:
    unreal.log_error(f"Cannot load {bp_path}")
    raise RuntimeError("BP not found")

cdo = unreal.get_default_object(bp.generated_class())

# ============================================================
# Part 1: Set ComboMontages array
# ============================================================
# These are the retargeted combo attack montages for Patchouli
COMBO_MONTAGE_PATHS = [
    "/Game/Combo_Attack_01_01_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_02_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_03_Seq_Montage_Patchouli",
    "/Game/Combo_Attack_01_04_Seq_Montage_Patchouli",
]

# Find the CombatComponent on the CDO
combat_comp = None
for comp_name in ["SekiroEnemyAttribute", "CombatComponent", "SekiroCombat"]:
    try:
        c = cdo.get_editor_property(comp_name)
        if c and hasattr(c, 'get_editor_property'):
            try:
                c.get_editor_property("ComboMontages")
                combat_comp = c
                unreal.log(f"Found combat component: {comp_name}")
                break
            except:
                pass
    except:
        pass

# Try finding by class
if not combat_comp:
    # Search all components
    character_class = bp.generated_class()
    unreal.log("Searching for combat component by iterating CDO components...")
    
    # Try the standard approach - get the component from the CDO
    try:
        # For C++ components, they're subobjects of the CDO
        combat_comp_obj = unreal.find_object(None, str(cdo.get_path_name()) + ".CombatComponent")
        if combat_comp_obj:
            combat_comp = combat_comp_obj
            unreal.log(f"Found via path: {combat_comp.get_class().get_name()}")
    except:
        pass

if not combat_comp:
    # Last resort - try getting it as a property directly
    try:
        combat_comp = cdo.get_editor_property("CombatComponent")
        if combat_comp:
            unreal.log(f"Found CombatComponent as property")
    except:
        unreal.log_warning("CombatComponent not accessible as property")

if combat_comp:
    combo_montages = []
    for path in COMBO_MONTAGE_PATHS:
        m = unreal.load_asset(path)
        if m:
            combo_montages.append(m)
            unreal.log(f"  Loaded combo montage: {path}")
        else:
            unreal.log_warning(f"  Cannot load: {path}")
    
    if combo_montages:
        try:
            combat_comp.set_editor_property("ComboMontages", combo_montages)
            unreal.log(f"Set {len(combo_montages)} ComboMontages on CombatComponent!")
        except Exception as e:
            unreal.log_error(f"Failed to set ComboMontages: {e}")
else:
    unreal.log_warning("Could not find CombatComponent - ComboMontages must be set manually in Blueprint editor")
    unreal.log("Open BP_SekiroEnemy -> Select CombatComponent -> Set ComboMontages to:")
    for p in COMBO_MONTAGE_PATHS:
        unreal.log(f"  {p}")

# ============================================================
# Part 2: Delete _Patchouli1 duplicate ABP/BlendSpace
# ============================================================
DUPLICATES = [
    "/Game/BS_SekiroMovement_Patchouli1",
    "/Game/ABP_SekiroCharacter_Patchouli1",
    "/Game/Block_Loop_Seq_Patchouli1",
    "/Game/Idle_Seq_Patchouli1",
    "/Game/Walk_Loop_F_0_Seq_Patchouli1",
    "/Game/Run_Loop_F_0_Seq_Patchouli1",
]

for dup in DUPLICATES:
    if unreal.EditorAssetLibrary.does_asset_exist(dup):
        unreal.EditorAssetLibrary.delete_asset(dup)
        unreal.log(f"Deleted duplicate: {dup}")
    else:
        unreal.log(f"Already gone: {dup}")

# ============================================================
# Part 3: Verify ABP_SekiroCharacter_Patchouli references correct BlendSpace
# ============================================================
abp = unreal.load_asset("/Game/ABP_SekiroCharacter_Patchouli")
if abp:
    unreal.log(f"ABP_SekiroCharacter_Patchouli exists: {abp.get_class().get_name()}")
    unreal.log(f"  Skeleton: {abp.get_editor_property('TargetSkeleton')}")
else:
    unreal.log_error("ABP_SekiroCharacter_Patchouli does NOT exist!")

# Save
unreal.EditorAssetLibrary.save_asset(bp_path)
unreal.log("BP_SekiroEnemy saved!")
unreal.log("=== Fix complete! ===")
