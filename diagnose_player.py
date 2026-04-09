"""
Diagnostic: Compare BP_SekiroEnemy (working) vs BP_SekiroCharacter (A-pose).
Run: File -> Execute Python Script
"""
import unreal

def dump_bp(bp_path, label):
    unreal.log("=" * 60)
    unreal.log(f"=== {label}: {bp_path} ===")
    unreal.log("=" * 60)

    bp = unreal.load_asset(bp_path)
    if not bp:
        unreal.log_error(f"Cannot load {bp_path}")
        return

    cdo = unreal.get_default_object(bp.generated_class())

    # --- 1. CDO Mesh (GetMesh() equivalent) ---
    unreal.log("--- CDO Mesh (CharacterMesh0 / GetMesh()) ---")
    try:
        mesh = cdo.get_editor_property("Mesh")
        if mesh:
            unreal.log(f"  Name: {mesh.get_name()}")
            try:
                sk = mesh.get_editor_property("SkeletalMeshAsset")
                unreal.log(f"  SkeletalMesh: {sk.get_path_name() if sk else 'NONE!'}")
            except:
                try:
                    sk = mesh.get_editor_property("SkeletalMesh")
                    unreal.log(f"  SkeletalMesh: {sk.get_path_name() if sk else 'NONE!'}")
                except:
                    unreal.log("  SkeletalMesh: CANNOT READ")
            try:
                ac = mesh.get_editor_property("AnimClass")
                unreal.log(f"  AnimClass: {ac.get_path_name() if ac else 'NONE!'}")
            except:
                unreal.log("  AnimClass: CANNOT READ")
            try:
                am = mesh.get_editor_property("AnimationMode")
                unreal.log(f"  AnimationMode: {am}")
            except:
                pass
            try:
                vis = mesh.get_editor_property("bVisible")
                hig = mesh.get_editor_property("bHiddenInGame")
                unreal.log(f"  bVisible={vis}, bHiddenInGame={hig}")
            except:
                pass
            try:
                rl = mesh.get_editor_property("RelativeLocation")
                rr = mesh.get_editor_property("RelativeRotation")
                unreal.log(f"  RelLoc={rl}, RelRot={rr}")
            except:
                pass
        else:
            unreal.log("  Mesh: NULL!")
    except Exception as e:
        unreal.log_error(f"  Cannot read Mesh: {e}")

    # --- 2. VRMMesh (via find_object on CDO subobject) ---
    unreal.log("--- VRMMesh component ---")
    cdo_path = cdo.get_path_name()
    vrm = None
    try:
        vrm = unreal.find_object(None, cdo_path + ".VRMMesh")
    except:
        pass
    if vrm:
        unreal.log(f"  FOUND VRMMesh: {vrm.get_class().get_name()}")
        try:
            sk = vrm.get_editor_property("SkeletalMeshAsset")
            unreal.log(f"  SkeletalMesh: {sk.get_path_name() if sk else 'None'}")
        except:
            try:
                sk = vrm.get_editor_property("SkeletalMesh")
                unreal.log(f"  SkeletalMesh: {sk.get_path_name() if sk else 'None'}")
            except:
                pass
        try:
            ac = vrm.get_editor_property("AnimClass")
            unreal.log(f"  AnimClass: {ac.get_path_name() if ac else 'None'}")
        except:
            pass
        try:
            vis = vrm.get_editor_property("bVisible")
            hig = vrm.get_editor_property("bHiddenInGame")
            unreal.log(f"  bVisible={vis}, bHiddenInGame={hig}")
        except:
            pass
    else:
        unreal.log("  VRMMesh: NOT FOUND (good - matches Patchouli pattern)")

    # --- 3. Capsule ---
    unreal.log("--- CapsuleComponent ---")
    try:
        cap = cdo.get_editor_property("CapsuleComponent")
        if cap:
            hh = cap.get_editor_property("CapsuleHalfHeight")
            r = cap.get_editor_property("CapsuleRadius")
            unreal.log(f"  HalfHeight={hh}, Radius={r}")
    except:
        pass

    # --- 4. WeaponSocketName ---
    unreal.log("--- WeaponSocketName ---")
    try:
        wsn = cdo.get_editor_property("WeaponSocketName")
        unreal.log(f"  WeaponSocketName='{wsn}'")
    except:
        unreal.log("  (not found)")

    # --- 5. ComboMontages ---
    unreal.log("--- ComboMontages ---")
    try:
        cc = cdo.get_editor_property("CombatComponent")
        if cc:
            cm = cc.get_editor_property("ComboMontages")
            unreal.log(f"  Count: {len(cm)}")
            for i, m in enumerate(cm):
                unreal.log(f"    [{i}] {m.get_path_name() if m else 'None'}")
        else:
            unreal.log("  CombatComponent: NULL")
    except:
        unreal.log("  (cannot read)")

    # --- 6. Key assets ---
    unreal.log("--- Related assets ---")
    suffix = "_Patchouli" if "Enemy" in bp_path else "_Reimu"
    for n in [f"ABP_SekiroCharacter{suffix}", f"BS_SekiroMovement{suffix}",
              f"Combo_Attack_01_01_Seq_Montage{suffix}"]:
        e = unreal.EditorAssetLibrary.does_asset_exist(f"/Game/{n}")
        unreal.log(f"  /Game/{n}: {'OK' if e else 'MISSING!'}")

    unreal.log("")

dump_bp("/Game/BP_SekiroEnemy", "ENEMY (working Patchouli)")
dump_bp("/Game/BP_SekiroCharacter", "PLAYER (broken Reimu)")
unreal.log("=== DIAGNOSTIC COMPLETE ===")
