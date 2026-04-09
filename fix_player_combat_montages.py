"""
Retarget ALL player combat reaction montages to Reimu + set on BP_SekiroCharacter.
Same approach as setup_patchouli_bp.py Part 2.

Montages: AM_Attack_03, AM_BlockHit, AM_ParrySuccess, AM_Death,
          AM_Execution, AM_Hit, AM_ParryAttempt, AM_Stun, AM_Deflect,
          BlockEnd_Root_Montage, BlockLoop_Root_Montage,
          BlockStart_Root_Montage, BlockHit_Root_Montage

Run: File -> Execute Python Script
"""
import unreal

# ============================================================
# Part 1: Retarget combat reaction montages to Reimu
# ============================================================
unreal.log("=" * 60)
unreal.log("Part 1: Retarget combat reaction montages to Reimu")
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

# Same list as batch_retarget_patchouli.py (combat montages + katana animations)
COMBAT_SOURCES = [
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
    # Katana block animations
    "/Game/Katana_animation/BlockEnd_Root_Montage",
    "/Game/Katana_animation/BlockLoop_Root_Montage",
    "/Game/Katana_animation/BlockStart_Root_Montage",
    "/Game/Katana_animation/BlockHit_Root_Montage",
]

# Check which ones need retargeting
to_retarget = []
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

for path in COMBAT_SOURCES:
    name = path.split("/")[-1]
    reimu_path = f"/Game/{name}_Reimu"
    if unreal.EditorAssetLibrary.does_asset_exist(reimu_path):
        unreal.log(f"  Already exists: {reimu_path}")
    else:
        full_path = path + "." + name
        ad = asset_registry.get_asset_by_object_path(full_path)
        if ad.is_valid():
            to_retarget.append(ad)
            unreal.log(f"  Will retarget: {path}")
        else:
            unreal.log_warning(f"  Source not found: {path}")

if to_retarget:
    results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        assets_to_retarget=to_retarget,
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
    unreal.log("All combat montages already retargeted!")

# ============================================================
# Part 2: Set montage references on BP_SekiroCharacter
# (Same as setup_patchouli_bp.py Part 2 but for player)
# ============================================================
unreal.log("=" * 60)
unreal.log("Part 2: Set montage references on BP_SekiroCharacter")
unreal.log("=" * 60)

bp_path = "/Game/BP_SekiroCharacter"
bp = unreal.load_asset(bp_path)
cdo = unreal.get_default_object(bp.generated_class())

# Exact same mapping as setup_patchouli_bp.py but with _Reimu suffix
MONTAGE_MAP = {
    "AttackMontage":       "/Game/AM_Attack_03_Reimu",
    "ParryAttemptMontage": "/Game/AM_ParryAttempt_Reimu",
    "BlockLoopMontage":    "/Game/BlockLoop_Root_Montage_Reimu",
    "BlockHitMontage":     "/Game/AM_BlockHit_Reimu",
    "BlockEndMontage":     "/Game/BlockEnd_Root_Montage_Reimu",
    "ParrySuccessMontage": "/Game/AM_ParrySuccess_Reimu",
    "HitMontage":          "/Game/AM_Hit_Reimu",
    "ExecutionMontage":    "/Game/AM_Execution_Reimu",
    "StunMontage":         "/Game/AM_Stun_Reimu",
    "DeathMontage":        "/Game/AM_Death_Reimu",
}

for prop_name, asset_path in MONTAGE_MAP.items():
    montage = unreal.load_asset(asset_path)
    if montage:
        try:
            cdo.set_editor_property(prop_name, montage)
            unreal.log(f"  Set {prop_name} -> {asset_path}")
        except Exception as e:
            unreal.log_warning(f"  Failed to set {prop_name}: {e}")
    else:
        unreal.log_warning(f"  Cannot load: {asset_path}")

# Save
unreal.EditorAssetLibrary.save_asset(bp_path)
unreal.log(f"Saved {bp_path}")

# ============================================================
# Verify
# ============================================================
unreal.log("\n--- VERIFICATION ---")
for prop_name, expected in MONTAGE_MAP.items():
    try:
        m = cdo.get_editor_property(prop_name)
        actual = m.get_path_name() if m else "None"
        ok = "Reimu" in actual
        unreal.log(f"  {prop_name}: {actual} {'OK' if ok else 'WRONG!'}")
    except:
        unreal.log_warning(f"  {prop_name}: cannot read")

unreal.log("=== COMBAT MONTAGES FIX COMPLETE ===")
