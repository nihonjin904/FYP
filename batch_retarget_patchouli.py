"""
Batch retarget all Sekiro combat animations from UE4 Mannequin to Patchouli VRM skeleton.
Run this in Unreal Editor: Output Log -> Python console, or File -> Execute Python Script
"""
import unreal

# === Configuration ===
RETARGETER_PATH = "/Game/Characters/patchouli/RTG_UE4__魔王産_パチュリー・ノーレッジ"
SOURCE_MESH_PATH = "/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin"
TARGET_MESH_PATH = "/Game/Characters/patchouli/SK__魔王産_パチュリー・ノーレッジ"

# All animation assets to retarget (montages AND sequences)
ASSETS_TO_RETARGET = [
    # Sekiro combat montages
    "/Game/Characters/Sekiro/AM_Attack_03",
    "/Game/Characters/Sekiro/AM_BlockHit",
    "/Game/Characters/Sekiro/AM_ParrySuccess",
    "/Game/Characters/Sekiro/AM_Death",
    "/Game/Characters/Sekiro/AM_Execution",
    "/Game/Characters/Sekiro/AM_Hit",
    "/Game/Characters/Sekiro/AM_ParryAttempt",
    "/Game/Characters/Sekiro/AM_Stun",
    "/Game/Characters/Sekiro/AM_Deflect",
    # Katana animation sequences
    "/Game/Katana_animation/BlockStart_Root",
    "/Game/Katana_animation/Attack1_Root",
    "/Game/Katana_animation/Attack2_Root",
    "/Game/Katana_animation/Attack3_Root",
    # Katana montages
    "/Game/Katana_animation/BlockEnd_Root_Montage",
    "/Game/Katana_animation/BlockLoop_Root_Montage",
    "/Game/Katana_animation/BlockStart_Root_Montage",
    "/Game/Katana_animation/BlockHit_Root_Montage",
    # Sword combo montages
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/04_Combo_Attack_04/Combo_Attack_04_01_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/04_Combo_Attack_04/Combo_Attack_04_02_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/04_Combo_Attack_04/Combo_Attack_04_03_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/04_Combo_Attack_04/Combo_Attack_04_04_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_01_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_02_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_03_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_04_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_01_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_02_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_03_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_04_Seq_Montage",
]

# === Load Assets ===
retargeter = unreal.load_asset(RETARGETER_PATH)
source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
target_mesh = unreal.load_asset(TARGET_MESH_PATH)

if not retargeter:
    unreal.log_error("Cannot load IK Retargeter: " + RETARGETER_PATH)
    raise RuntimeError("Missing retargeter")
if not source_mesh:
    unreal.log_error("Cannot load source mesh: " + SOURCE_MESH_PATH)
    raise RuntimeError("Missing source mesh")
if not target_mesh:
    unreal.log_error("Cannot load target mesh: " + TARGET_MESH_PATH)
    raise RuntimeError("Missing target mesh")

# === Build AssetData list ===
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
asset_data_list = []
missing = []
for path in ASSETS_TO_RETARGET:
    full_path = path + "." + path.split("/")[-1]
    ad = asset_registry.get_asset_by_object_path(full_path)
    if ad.is_valid():
        asset_data_list.append(ad)
    else:
        missing.append(path)
        unreal.log_warning("Asset not found: " + path)

if missing:
    unreal.log_warning(f"{len(missing)} assets not found, retargeting {len(asset_data_list)} assets")
else:
    unreal.log(f"All {len(asset_data_list)} assets found")

# === Run Batch Retarget ===
unreal.log("Starting batch retarget...")
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

unreal.log(f"Batch retarget complete! {len(results)} assets created.")
for r in results:
    unreal.log(f"  -> {r.package_name}")
