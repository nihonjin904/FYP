"""
Re-apply BP_SekiroEnemy settings after Live Coding CDO reset.
Run in Unreal Editor: Output Log -> Python
"""
import unreal

bp_path = "/Game/BP_SekiroEnemy"
bp = unreal.load_asset(bp_path)
if not bp:
    unreal.log_error("Cannot load BP_SekiroEnemy")
    raise RuntimeError("BP not found")

cdo = unreal.get_default_object(bp.generated_class())

# Check ComboMontages
combat_comp = cdo.get_editor_property("CombatComponent")
if combat_comp:
    current = combat_comp.get_editor_property("ComboMontages")
    unreal.log(f"Current ComboMontages count: {len(current) if current else 0}")
    
    if not current or len(current) == 0:
        unreal.log("ComboMontages is EMPTY - re-setting...")
        COMBO_PATHS = [
            "/Game/Combo_Attack_01_01_Seq_Montage_Patchouli",
            "/Game/Combo_Attack_01_02_Seq_Montage_Patchouli",
            "/Game/Combo_Attack_01_03_Seq_Montage_Patchouli",
            "/Game/Combo_Attack_01_04_Seq_Montage_Patchouli",
        ]
        montages = []
        for p in COMBO_PATHS:
            m = unreal.load_asset(p)
            if m:
                montages.append(m)
                unreal.log(f"  Loaded: {p}")
        combat_comp.set_editor_property("ComboMontages", montages)
        unreal.log(f"Set {len(montages)} ComboMontages")
    else:
        unreal.log("ComboMontages OK - not empty")
        for i, m in enumerate(current):
            unreal.log(f"  [{i}]: {m.get_name() if m else 'None'}")

# Verify other montages
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

for prop, path in MONTAGE_MAP.items():
    val = cdo.get_editor_property(prop)
    if not val:
        m = unreal.load_asset(path)
        if m:
            cdo.set_editor_property(prop, m)
            unreal.log(f"Re-set {prop}")
    else:
        unreal.log(f"{prop} OK: {val.get_name()}")

# Verify WeaponSocketName  
wsn = cdo.get_editor_property("WeaponSocketName")
unreal.log(f"WeaponSocketName: {wsn}")
if str(wsn) != "右手首":
    cdo.set_editor_property("WeaponSocketName", "右手首")
    unreal.log("Re-set WeaponSocketName -> 右手首")

# Verify AnimClass
mesh_comp = cdo.get_editor_property("Mesh")
if mesh_comp:
    anim_class = mesh_comp.get_editor_property("AnimClass")
    unreal.log(f"AnimClass: {anim_class}")

unreal.EditorAssetLibrary.save_asset(bp_path)
unreal.log("=== Diagnostic complete and saved ===")
