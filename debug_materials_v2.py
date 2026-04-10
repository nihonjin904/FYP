"""
Deep Material Diagnostic v2
Compares ORIGINAL VRM materials vs CURRENT materials on both characters.
Focuses on: parent material properties, blend mode, two-sided, per-slot assignment.

Run: Unreal Editor > Tools > Execute Python Script > select this file
Output: C:/Unreal Projects/FYP/debug_materials_v2.txt
"""
import unreal

lines = []
def log(msg=""):
    lines.append(str(msg))
    unreal.log(str(msg))


def get_parent_chain(mi):
    """Walk up the parent chain and return info about each level."""
    chain = []
    current = mi
    while current is not None:
        info = {"name": current.get_name(), "class": current.get_class().get_name(), "path": current.get_path_name()}
        if isinstance(current, unreal.Material):
            try:
                info["blend_mode"] = str(current.get_editor_property('blend_mode'))
                info["shading_model"] = str(current.get_editor_property('shading_model'))
                info["two_sided"] = current.get_editor_property('two_sided')
            except:
                pass
            chain.append(info)
            break
        elif isinstance(current, unreal.MaterialInstanceConstant):
            try:
                current = current.parent
            except:
                break
        else:
            break
        chain.append(info)
    return chain


def dump_original_vrm_materials(folder, label):
    """List all original VRM MI's and their parent chain."""
    log(f"\n{'#'*60}")
    log(f"ORIGINAL VRM MATERIALS: {label}")
    log(f"Folder: {folder}")
    log(f"{'#'*60}")
    
    assets = unreal.EditorAssetLibrary.list_assets(folder, recursive=False)
    mi_assets = []
    for path in sorted(assets):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset and isinstance(asset, unreal.MaterialInstanceConstant):
            name = asset.get_name()
            # Skip our _Lit copies
            if "_Lit" in name:
                continue
            mi_assets.append(asset)
    
    for mi in mi_assets:
        log(f"\n--- {mi.get_name()} ---")
        chain = get_parent_chain(mi)
        for i, info in enumerate(chain):
            prefix = "  " * i
            log(f"{prefix}> {info['class']}: {info['name']}")
            log(f"{prefix}  Path: {info['path']}")
            if 'blend_mode' in info:
                log(f"{prefix}  Blend Mode: {info['blend_mode']}")
                log(f"{prefix}  Shading Model: {info['shading_model']}")
                log(f"{prefix}  Two Sided: {info['two_sided']}")
        
        # Read key texture parameters
        tex_params = ["gltf_tex_diffuse", "mtoon_tex_ShadeTexture", "MainTexture"]
        for pname in tex_params:
            try:
                tex = mi.get_texture_parameter_value(pname)
                if tex:
                    log(f"  Texture [{pname}]: {tex.get_name()}")
            except:
                pass
        
        scalar_params = ["PostLightScaleMat", "bOpaque", "bDivAdaptation"]
        for pname in scalar_params:
            try:
                val = mi.get_scalar_parameter_value(pname)
                log(f"  Scalar [{pname}]: {val}")
            except:
                pass


def dump_current_materials(folder, label):
    """List all our custom M_* materials and their properties."""
    log(f"\n{'#'*60}")
    log(f"CURRENT CUSTOM MATERIALS: {label}")
    log(f"{'#'*60}")
    
    assets = unreal.EditorAssetLibrary.list_assets(folder, recursive=True)
    for path in sorted(assets):
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset and isinstance(asset, unreal.Material):
            name = asset.get_name()
            if not name.startswith("M_"):
                continue
            log(f"\n--- {name} ---")
            log(f"  Path: {asset.get_path_name()}")
            try:
                log(f"  Blend Mode: {asset.get_editor_property('blend_mode')}")
                log(f"  Shading Model: {asset.get_editor_property('shading_model')}")
                log(f"  Two Sided: {asset.get_editor_property('two_sided')}")
            except Exception as e:
                log(f"  (Error: {e})")


def dump_bp_cdo_vs_original(bp_path, label, mesh_folder):
    """Compare BP CDO material assignments vs what the original VRM MI names suggest."""
    log(f"\n{'#'*60}")
    log(f"BP CDO vs ORIGINAL: {label}")
    log(f"{'#'*60}")
    
    bp = unreal.EditorAssetLibrary.load_asset(bp_path)
    if not bp:
        log("  BP not found!")
        return
    
    gen_class = bp.generated_class()
    cdo = unreal.get_default_object(gen_class) if gen_class else None
    if not cdo:
        log("  CDO not found!")
        return
    
    # Find mesh component
    mesh_comp = None
    skel_comps = cdo.get_components_by_class(unreal.SkeletalMeshComponent)
    for comp in skel_comps:
        if 'CharacterMesh0' in comp.get_name() or 'Mesh' in comp.get_name():
            mesh_comp = comp
            break
    if not mesh_comp and skel_comps:
        mesh_comp = skel_comps[0]
    
    if not mesh_comp:
        log("  No SkeletalMeshComponent!")
        return
        
    # Get skeletal mesh material slot names
    sk_mesh = None
    try:
        sk_mesh = mesh_comp.get_editor_property('skeletal_mesh_asset')
    except:
        try:
            sk_mesh = mesh_comp.skeletal_mesh
        except:
            pass
    
    slot_names = []
    if sk_mesh:
        try:
            materials = sk_mesh.get_editor_property('materials')
            for slot in materials:
                slot_names.append(str(slot.get_editor_property('material_slot_name')))
        except:
            pass
    
    num = mesh_comp.get_num_materials()
    log(f"\nSlot | Mesh Slot Name | Current Material | Type | Blend | Two-Sided")
    log(f"{'-'*90}")
    
    for i in range(num):
        slot_name = slot_names[i] if i < len(slot_names) else "?"
        mi = mesh_comp.get_material(i)
        if mi:
            mat_name = mi.get_name()
            mat_class = mi.get_class().get_name()
            
            # Get root material properties
            blend = "?"
            two_sided = "?"
            if isinstance(mi, unreal.Material):
                try:
                    blend = str(mi.get_editor_property('blend_mode')).split('.')[-1]
                    two_sided = mi.get_editor_property('two_sided')
                except:
                    pass
            elif isinstance(mi, unreal.MaterialInstanceConstant):
                # Walk to root material
                root = mi
                while isinstance(root, unreal.MaterialInstanceConstant) and root.parent:
                    root = root.parent
                if isinstance(root, unreal.Material):
                    try:
                        blend = str(root.get_editor_property('blend_mode')).split('.')[-1]
                        two_sided = root.get_editor_property('two_sided')
                    except:
                        pass
                mat_name += f" (parent: {root.get_name()})"
            
            log(f"  {i}  | {slot_name:14} | {mat_name:40} | {mat_class:10} | {blend:15} | {two_sided}")
        else:
            log(f"  {i}  | {slot_name:14} | (None)")
    
    # Also show what ORIGINAL VRM MI was for each slot name
    log(f"\n  Original VRM MI mapping (by slot name):")
    for i, sn in enumerate(slot_names):
        # Try to find matching original MI
        original_mi_path = f"{mesh_folder}/MI_{sn}"
        exists = unreal.EditorAssetLibrary.does_asset_exist(original_mi_path)
        if exists:
            orig = unreal.EditorAssetLibrary.load_asset(original_mi_path)
            if orig:
                root = orig
                while isinstance(root, unreal.MaterialInstanceConstant) and root.parent:
                    root = root.parent
                root_blend = "?"
                root_two = "?"
                if isinstance(root, unreal.Material):
                    try:
                        root_blend = str(root.get_editor_property('blend_mode')).split('.')[-1]
                        root_two = root.get_editor_property('two_sided')
                    except:
                        pass
                log(f"    Slot {i} ({sn}): MI_{sn} -> parent: {root.get_name()} | blend: {root_blend} | two-sided: {root_two}")
        else:
            log(f"    Slot {i} ({sn}): (no MI_{sn} found)")


# ============================================================
log("=" * 60)
log("DEEP MATERIAL DIAGNOSTIC v2")
log("=" * 60)

# 1. Original VRM materials
dump_original_vrm_materials("/Game/Characters/reimu/", "Reimu")
dump_original_vrm_materials("/Game/Characters/patchouli/", "Patchouli")

# 2. Current custom materials  
dump_current_materials("/Game/Characters/reimu/", "Reimu Custom")
dump_current_materials("/Game/Characters/patchouli/", "Patchouli Custom")

# 3. BP CDO comparison
dump_bp_cdo_vs_original("/Game/BP_SekiroCharacter", "Reimu (Player)", "/Game/Characters/reimu")
dump_bp_cdo_vs_original("/Game/BP_SekiroEnemy", "Patchouli (Enemy)", "/Game/Characters/patchouli")

# 4. Scene actor check
log(f"\n{'#'*60}")
log("SCENE ACTOR: Patchouli")
log(f"{'#'*60}")
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in actors:
    if "SekiroEnemy" in actor.get_name():
        skel_comps = actor.get_components_by_class(unreal.SkeletalMeshComponent)
        for comp in skel_comps:
            num = comp.get_num_materials()
            log(f"Actor: {actor.get_name()} | Component: {comp.get_name()} | Slots: {num}")
            for i in range(num):
                mi = comp.get_material(i)
                if mi:
                    log(f"  Slot {i}: {mi.get_name()} ({mi.get_class().get_name()})")

# Save
out = "C:/Unreal Projects/FYP/debug_materials_v2.txt"
with open(out, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
log(f"\nSaved to: {out}")
unreal.log_warning(f"Debug v2 saved to: {out}")
