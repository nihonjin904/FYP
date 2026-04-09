"""
Exact mirror of setup_patchouli_bp.py — but for Reimu as the PLAYER character.

Part 1: Retarget ABP_SekiroCharacter + BS_SekiroMovement → Reimu skeleton
         (include_referenced_assets=True captures all locomotion anims automatically)
Part 2: Retarget player combat montages → Reimu skeleton
Part 3: Set BP_SekiroCharacter: Reimu mesh + ABP_SekiroCharacter_Reimu + WeaponSocketName

Run in Unreal Editor:  File -> Execute Python Script
"""
import unreal

# === Configuration ===
RETARGETER_PATH  = "/Game/Characters/reimu/RTG_UE4__\u9b54_\u535a\u9e97_\u970a\u5922"
SOURCE_MESH_PATH = "/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin"
TARGET_MESH_PATH = "/Game/Characters/reimu/SK__\u9b54_\u535a\u9e97_\u970a\u5922"

retargeter   = unreal.load_asset(RETARGETER_PATH)
source_mesh  = unreal.load_asset(SOURCE_MESH_PATH)
target_mesh  = unreal.load_asset(TARGET_MESH_PATH)

if not retargeter:
    unreal.log_error("Cannot load IKRetargeter: " + RETARGETER_PATH)
    raise RuntimeError("Missing retargeter!")
if not source_mesh or not target_mesh:
    raise RuntimeError("Missing source or target mesh!")

unreal.log("Assets loaded OK")

# ================================================================
# Part 1: Retarget ABP_SekiroCharacter + BS_SekiroMovement
#         (mirrors setup_patchouli_bp.py Part 1 exactly)
# ================================================================
CORE_ASSETS = [
    "/Game/BS_SekiroMovement",      # BlendSpace (references Idle + Walk anims)
    "/Game/ABP_SekiroCharacter",    # AnimBlueprint (Mannequin locomotion + combat slot)
]

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
core_data = []
for path in CORE_ASSETS:
    full_path = path + "." + path.split("/")[-1]
    ad = asset_registry.get_asset_by_object_path(full_path)
    if ad.is_valid():
        core_data.append(ad)
        unreal.log(f"[OK] {path}")
    else:
        unreal.log_warning(f"[MISSING] {path}")

if core_data:
    unreal.log("\nRetargeting ABP + BlendSpace to Reimu...")
    results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        assets_to_retarget=core_data,
        source_mesh=source_mesh,
        target_mesh=target_mesh,
        ik_retarget_asset=retargeter,
        search="",
        replace="",
        prefix="",
        suffix="_Reimu",
        include_referenced_assets=True   # captures Idle_Seq, Walk_Loop_F_0_Seq, etc.
    )
    unreal.log(f"Part 1 done: {len(results)} assets created:")
    for r in results:
        unreal.log(f"  -> {r.package_name}")

# ================================================================
# Part 2: Retarget player combat montages
#         (Combo_Attack_01 — same base as Patchouli enemy)
# ================================================================
COMBAT_MONTAGES = [
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_01_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_02_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_03_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/01_Combo_Attack_01/Combo_Attack_01_04_Seq_Montage",
]

combat_data = []
for path in COMBAT_MONTAGES:
    full_path = path + "." + path.split("/")[-1]
    ad = asset_registry.get_asset_by_object_path(full_path)
    if ad.is_valid():
        combat_data.append(ad)
        unreal.log(f"[OK] {path}")
    else:
        unreal.log_warning(f"[MISSING] {path}")

if combat_data:
    unreal.log("\nRetargeting combat montages to Reimu...")
    results2 = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        assets_to_retarget=combat_data,
        source_mesh=source_mesh,
        target_mesh=target_mesh,
        ik_retarget_asset=retargeter,
        search="", replace="", prefix="", suffix="_Reimu",
        include_referenced_assets=True
    )
    unreal.log(f"Part 2 done: {len(results2)} assets created:")
    for r in results2:
        unreal.log(f"  -> {r.package_name}")

# ================================================================
# Part 3: Set BP_SekiroCharacter with Reimu mesh + AnimBP
#         (mirrors how setup_patchouli_bp.py sets BP_SekiroEnemy)
# ================================================================
unreal.log("\nSetting BP_SekiroCharacter...")

bp_path = "/Game/BP_SekiroCharacter"
bp = unreal.load_asset(bp_path)
if not bp:
    unreal.log_error(f"Cannot load {bp_path}")
else:
    cdo = unreal.get_default_object(bp.generated_class())

    # Set AnimBP to retargeted version
    abp_reimu = unreal.load_asset("/Game/ABP_SekiroCharacter_Reimu")
    if abp_reimu:
        mesh_comp = cdo.get_editor_property("Mesh")
        if mesh_comp:
            mesh_comp.set_editor_property("AnimClass", abp_reimu.generated_class())
            unreal.log("  Set AnimClass -> ABP_SekiroCharacter_Reimu")
    else:
        unreal.log_warning("  ABP_SekiroCharacter_Reimu not found yet (expected after Part 1)")

    # Set Reimu skeletal mesh
    reimu_mesh = unreal.load_asset(TARGET_MESH_PATH)
    if reimu_mesh:
        mesh_comp = cdo.get_editor_property("Mesh")
        if mesh_comp:
            mesh_comp.set_editor_property("SkeletalMeshAsset", reimu_mesh)
            unreal.log("  Set SkeletalMesh -> Reimu")

    # Set WeaponSocketName to VRM right hand bone
    cdo.set_editor_property("WeaponSocketName", "\u53f3\u624b\u9996")  # 右手首
    unreal.log("  Set WeaponSocketName -> 右手首")

    unreal.EditorAssetLibrary.save_asset(bp_path)
    unreal.log("BP_SekiroCharacter saved!")

unreal.log("\n=== All done! Please run Ctrl+Alt+F11 to Live Compile ===")
