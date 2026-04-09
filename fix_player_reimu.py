"""
Fix BP_SekiroCharacter FINAL:
1. Retarget Combo_Attack_03 montages to Reimu (player uses 03)
2. Set correct ComboMontages on BP_SekiroCharacter
3. bHiddenInGame = False
4. Remove VRMMesh from Blueprint (A-pose overlay)

Run: File -> Execute Python Script
"""
import unreal

# ============================================================
# Part 1: Retarget Combo_Attack_03 to Reimu
# ============================================================
unreal.log("=" * 60)
unreal.log("Part 1: Retarget Combo_Attack_03 to Reimu")
unreal.log("=" * 60)

RETARGETER = "/Game/Characters/reimu/RTG_UE4__魔_博麗_霊夢"
SOURCE_MESH = "/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin"
TARGET_MESH = "/Game/Characters/reimu/SK__魔_博麗_霊夢"

retargeter = unreal.load_asset(RETARGETER)
source_mesh = unreal.load_asset(SOURCE_MESH)
target_mesh = unreal.load_asset(TARGET_MESH)

if not all([retargeter, source_mesh, target_mesh]):
    unreal.log_error("Missing retargeter or meshes!")
    raise RuntimeError("Missing assets")

COMBO_03_SOURCES = [
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_01_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_02_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_03_Seq_Montage",
    "/Game/Sword_Animations/Animations/Sequence/02_Attack/03_Combo_Attack_03/Combo_Attack_03_04_Seq_Montage",
]

# Check if already retargeted
first_reimu = "/Game/Combo_Attack_03_01_Seq_Montage_Reimu"
if unreal.EditorAssetLibrary.does_asset_exist(first_reimu):
    unreal.log(f"Already exists: {first_reimu}, skipping retarget")
else:
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data_list = []
    for path in COMBO_03_SOURCES:
        full_path = path + "." + path.split("/")[-1]
        ad = asset_registry.get_asset_by_object_path(full_path)
        if ad.is_valid():
            asset_data_list.append(ad)
            unreal.log(f"  Found: {path}")
        else:
            unreal.log_warning(f"  NOT FOUND: {path}")

    if asset_data_list:
        results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
            assets_to_retarget=asset_data_list,
            source_mesh=source_mesh,
            target_mesh=target_mesh,
            ik_retarget_asset=retargeter,
            search="",
            replace="",
            prefix="",
            suffix="_Reimu",
            include_referenced_assets=True
        )
        unreal.log(f"Retarget done! {len(results)} assets:")
        for r in results:
            unreal.log(f"  -> {r.package_name}")
    else:
        unreal.log_error("No Combo_Attack_03 source assets found!")

# ============================================================
# Part 2: Fix BP_SekiroCharacter
# ============================================================
unreal.log("=" * 60)
unreal.log("Part 2: Fix BP_SekiroCharacter CDO")
unreal.log("=" * 60)

bp_path = "/Game/BP_SekiroCharacter"
bp = unreal.load_asset(bp_path)
cdo = unreal.get_default_object(bp.generated_class())

# Fix bHiddenInGame
mesh = cdo.get_editor_property("Mesh")
if mesh:
    old = mesh.get_editor_property("bHiddenInGame")
    mesh.set_editor_property("bHiddenInGame", False)
    unreal.log(f"  bHiddenInGame: {old} -> False")

# Set ComboMontages to Combo_Attack_03_Reimu
REIMU_03_PATHS = [
    "/Game/Combo_Attack_03_01_Seq_Montage_Reimu",
    "/Game/Combo_Attack_03_02_Seq_Montage_Reimu",
    "/Game/Combo_Attack_03_03_Seq_Montage_Reimu",
    "/Game/Combo_Attack_03_04_Seq_Montage_Reimu",
]

cc = None
try:
    cc = cdo.get_editor_property("CombatComponent")
except:
    pass
if not cc:
    try:
        cc = unreal.find_object(None, cdo.get_path_name() + ".CombatComponent")
    except:
        pass

if cc:
    montages = []
    for path in REIMU_03_PATHS:
        m = unreal.load_asset(path)
        if m:
            montages.append(m)
            unreal.log(f"  Loaded: {path}")
        else:
            unreal.log_warning(f"  MISSING: {path}")
    if montages:
        cc.set_editor_property("ComboMontages", montages)
        unreal.log(f"  Set {len(montages)} ComboMontages (Combo_Attack_03_Reimu)")
    else:
        unreal.log_error("No Combo_Attack_03_Reimu montages found!")
else:
    unreal.log_error("CombatComponent not found!")

# ============================================================
# Part 3: Remove VRMMesh from Blueprint SCS
# ============================================================
unreal.log("=" * 60)
unreal.log("Part 3: Remove VRMMesh")
unreal.log("=" * 60)

removed = False
try:
    scs = bp.get_editor_property("SimpleConstructionScript")
    if scs:
        nodes = scs.get_all_nodes()
        for node in nodes:
            ct = node.get_editor_property("ComponentTemplate")
            if ct and ct.get_name() == "VRMMesh":
                scs.remove_node(node, True)
                removed = True
                unreal.log("  VRMMesh REMOVED from SCS!")
                break
except Exception as e:
    unreal.log_warning(f"  SCS access failed: {e}")

if not removed:
    unreal.log_warning("  >>> 請手動刪除 VRMMesh: 打開 BP_SekiroCharacter -> Components -> 右鍵 VRMMesh -> Delete <<<")

# Save
unreal.EditorAssetLibrary.save_asset(bp_path)
unreal.log(f"Saved {bp_path}")

# ============================================================
# Verify
# ============================================================
unreal.log("\n" + "=" * 60)
unreal.log("VERIFICATION")
unreal.log("=" * 60)
bp2 = unreal.load_asset(bp_path)
cdo2 = unreal.get_default_object(bp2.generated_class())
mesh2 = cdo2.get_editor_property("Mesh")
unreal.log(f"  bHiddenInGame: {mesh2.get_editor_property('bHiddenInGame')}")
cc2 = cdo2.get_editor_property("CombatComponent")
if cc2:
    cm = cc2.get_editor_property("ComboMontages")
    unreal.log(f"  ComboMontages: {len(cm)}")
    for i, m in enumerate(cm):
        p = m.get_path_name() if m else "None"
        ok = "Reimu" in p and "03" in p
        unreal.log(f"    [{i}] {p} {'OK' if ok else 'WRONG!'}")

unreal.log("=== ALL FIXES COMPLETE ===")
