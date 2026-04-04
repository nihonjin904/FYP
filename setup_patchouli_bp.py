"""
Step 2: Retarget ABP + BlendSpace, then set BP_SekiroEnemy montage references.
Run this in Unreal Editor: Output Log -> Python console, or Tools -> Execute Python Script
"""
import unreal

# ============================================================
# Part 1: Retarget BS_SekiroMovement + ABP_SekiroCharacter
# ============================================================
RETARGETER_PATH = "/Game/Characters/patchouli/RTG_UE4__魔王産_パチュリー・ノーレッジ"
SOURCE_MESH_PATH = "/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin"
TARGET_MESH_PATH = "/Game/Characters/patchouli/SK__魔王産_パチュリー・ノーレッジ"

retargeter = unreal.load_asset(RETARGETER_PATH)
source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
target_mesh = unreal.load_asset(TARGET_MESH_PATH)

# Retarget the BlendSpace and ABP
ASSETS_TO_RETARGET = [
    "/Game/BS_SekiroMovement",
    "/Game/ABP_SekiroCharacter",
]

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
asset_data_list = []
for path in ASSETS_TO_RETARGET:
    full_path = path + "." + path.split("/")[-1]
    ad = asset_registry.get_asset_by_object_path(full_path)
    if ad.is_valid():
        asset_data_list.append(ad)
        unreal.log(f"Found: {path}")
    else:
        unreal.log_warning(f"Not found: {path}")

if asset_data_list:
    unreal.log("Retargeting ABP + BlendSpace...")
    results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        assets_to_retarget=asset_data_list,
        source_mesh=source_mesh,
        target_mesh=target_mesh,
        ik_retarget_asset=retargeter,
        search="",
        replace="",
        prefix="",
        suffix="_Patchouli",
        include_referenced_assets=True
    )
    unreal.log(f"Retarget complete! {len(results)} assets created:")
    for r in results:
        unreal.log(f"  -> {r.package_name}")

# ============================================================
# Part 2: Set BP_SekiroEnemy montage references and AnimClass
# ============================================================
unreal.log("Setting BP_SekiroEnemy montage references...")

bp_path = "/Game/BP_SekiroEnemy"
bp = unreal.load_asset(bp_path)
if not bp:
    unreal.log_error(f"Cannot load {bp_path}")
else:
    cdo = unreal.get_default_object(bp.generated_class())
    
    # Map: property_name -> retargeted asset path
    MONTAGE_MAP = {
        "AttackMontage": "/Game/AM_Attack_03_Patchouli",
        "ParryAttemptMontage": "/Game/AM_ParryAttempt_Patchouli",
        "BlockLoopMontage": "/Game/BlockLoop_Root_Montage_Patchouli",
        "BlockHitMontage": "/Game/AM_BlockHit_Patchouli",
        "BlockEndMontage": "/Game/BlockEnd_Root_Montage_Patchouli",
        "ParrySuccessMontage": "/Game/AM_ParrySuccess_Patchouli",
        "HitMontage": "/Game/AM_Hit_Patchouli",
        "ExecutionMontage": "/Game/AM_Execution_Patchouli",
        "StunMontage": "/Game/AM_Stun_Patchouli",
        "DeathMontage": "/Game/AM_Death_Patchouli",
    }
    
    for prop_name, asset_path in MONTAGE_MAP.items():
        montage = unreal.load_asset(asset_path)
        if montage:
            cdo.set_editor_property(prop_name, montage)
            unreal.log(f"  Set {prop_name} -> {asset_path}")
        else:
            unreal.log_warning(f"  Cannot load montage: {asset_path}")
    
    # Set AnimClass to retargeted ABP (if it was created)
    abp_patchouli = unreal.load_asset("/Game/ABP_SekiroCharacter_Patchouli")
    if abp_patchouli:
        mesh_comp = cdo.get_editor_property("Mesh")
        if mesh_comp:
            # AnimClass is set on the SkeletalMeshComponent
            mesh_comp.set_editor_property("AnimClass", abp_patchouli.generated_class())
            unreal.log("  Set AnimClass -> ABP_SekiroCharacter_Patchouli")
        else:
            unreal.log_warning("  Cannot access Mesh component")
    else:
        unreal.log_warning("  ABP_SekiroCharacter_Patchouli not found - set AnimBP manually")

    # Set WeaponSocketName
    cdo.set_editor_property("WeaponSocketName", "右手首")
    unreal.log("  Set WeaponSocketName -> 右手首")
    
    # Save
    unreal.EditorAssetLibrary.save_asset(bp_path)
    unreal.log("BP_SekiroEnemy saved!")

unreal.log("=== All done! ===")
