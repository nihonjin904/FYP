"""
Debug Script: Dump complete material state for both characters.
Run in Unreal Editor: Tools > Execute Python Script > select this file
Output saved to: C:/Unreal Projects/FYP/debug_materials_output.txt
"""
import unreal

output_lines = []

def log(msg=""):
    output_lines.append(str(msg))
    unreal.log(str(msg))

def dump_material_interface(mi, indent="  "):
    """Dump a MaterialInterface's full info."""
    if mi is None:
        log(f"{indent}(None)")
        return
    
    log(f"{indent}Name: {mi.get_name()}")
    log(f"{indent}Class: {mi.get_class().get_name()}")
    log(f"{indent}Path: {mi.get_path_name()}")
    
    # Check if it's a MaterialInstance
    if isinstance(mi, unreal.MaterialInstanceConstant):
        parent = mi.parent
        if parent:
            log(f"{indent}Parent: {parent.get_name()} ({parent.get_path_name()})")
            # Check parent's parent recursively
            if isinstance(parent, unreal.MaterialInstanceConstant) and parent.parent:
                log(f"{indent}Parent's Parent: {parent.parent.get_name()} ({parent.parent.get_path_name()})")
            elif isinstance(parent, unreal.Material):
                mat = parent
                log(f"{indent}Parent Shading Model: {mat.get_editor_property('shading_model')}")
                log(f"{indent}Parent Blend Mode: {mat.get_editor_property('blend_mode')}")
                log(f"{indent}Parent Two Sided: {mat.get_editor_property('two_sided')}")
        
        # Dump scalar parameters
        log(f"{indent}--- Scalar Parameters ---")
        try:
            scalar_names = ["PostLightScaleMat", "bOpaque", "TexturePow", "TextureSaturate", 
                           "bDivAdaptation", "Roughness", "Metallic", "Opacity"]
            for pname in scalar_names:
                result = mi.get_scalar_parameter_value(pname)
                if result is not None:
                    log(f"{indent}  {pname} = {result}")
        except Exception as e:
            log(f"{indent}  (Error reading scalars: {e})")
        
        # Dump vector parameters
        log(f"{indent}--- Vector Parameters ---")
        try:
            vec_names = ["gltf_basecolor", "BaseColor"]
            for pname in vec_names:
                try:
                    result = mi.get_vector_parameter_value(pname)
                    if result is not None:
                        log(f"{indent}  {pname} = R={result.r:.3f} G={result.g:.3f} B={result.b:.3f} A={result.a:.3f}")
                except:
                    pass
        except Exception as e:
            log(f"{indent}  (Error reading vectors: {e})")
        
        # Dump texture parameters
        log(f"{indent}--- Texture Parameters ---")
        try:
            tex_names = ["gltf_tex_diffuse", "mtoon_tex_ShadeTexture", "DiffuseTexture", "BaseColorTexture"]
            for pname in tex_names:
                try:
                    result = mi.get_texture_parameter_value(pname)
                    if result is not None:
                        log(f"{indent}  {pname} = {result.get_name()} ({result.get_path_name()})")
                except:
                    pass
        except Exception as e:
            log(f"{indent}  (Error reading textures: {e})")
    
    elif isinstance(mi, unreal.Material):
        mat = mi
        try:
            log(f"{indent}Shading Model: {mat.get_editor_property('shading_model')}")
            log(f"{indent}Blend Mode: {mat.get_editor_property('blend_mode')}")
            log(f"{indent}Two Sided: {mat.get_editor_property('two_sided')}")
        except Exception as e:
            log(f"{indent}(Error reading material props: {e})")


def dump_skeletal_mesh_materials(skel_mesh_comp, label):
    """Dump all material slots on a SkeletalMeshComponent."""
    log(f"\n{'='*60}")
    log(f"COMPONENT: {label}")
    log(f"{'='*60}")
    
    if skel_mesh_comp is None:
        log("  (Component is None)")
        return
    
    num_materials = skel_mesh_comp.get_num_materials()
    log(f"Total Material Slots: {num_materials}")
    
    sk_mesh = skel_mesh_comp.get_editor_property('skeletal_mesh_asset') if hasattr(skel_mesh_comp, 'skeletal_mesh_asset') else None
    if sk_mesh is None:
        try:
            sk_mesh = skel_mesh_comp.skeletal_mesh
        except:
            try:
                sk_mesh = skel_mesh_comp.get_editor_property('skeletal_mesh')
            except:
                pass
    
    if sk_mesh:
        log(f"Skeletal Mesh: {sk_mesh.get_name()} ({sk_mesh.get_path_name()})")
        # Get material slot names from the mesh
        try:
            mat_slot_names = sk_mesh.get_editor_property('materials')
            if mat_slot_names:
                log(f"Mesh Material Slot Names:")
                for i, slot in enumerate(mat_slot_names):
                    slot_name = slot.get_editor_property('material_slot_name') if slot else "?"
                    log(f"  Slot {i}: {slot_name}")
        except Exception as e:
            log(f"  (Could not read slot names: {e})")
    
    log(f"\nMaterials on Component:")
    for i in range(num_materials):
        log(f"\n--- Slot {i} ---")
        mi = skel_mesh_comp.get_material(i)
        dump_material_interface(mi)


def dump_blueprint_cdo(bp_path, label):
    """Load a Blueprint and dump its CDO's CharacterMesh0 materials."""
    log(f"\n{'#'*60}")
    log(f"BLUEPRINT CDO: {label}")
    log(f"Path: {bp_path}")
    log(f"{'#'*60}")
    
    bp = unreal.EditorAssetLibrary.load_asset(bp_path)
    if bp is None:
        log("  (Blueprint not found!)")
        return
    
    gen_class = bp.generated_class()
    if gen_class is None:
        log("  (No generated class)")
        return
    
    cdo = unreal.get_default_object(gen_class)
    if cdo is None:
        log("  (No CDO)")
        return
    
    # Find CharacterMesh0
    mesh_comp = None
    try:
        mesh_comp = cdo.mesh
    except:
        try:
            mesh_comp = cdo.get_editor_property('mesh')
        except:
            pass
    
    if mesh_comp is None:
        # Try to find by component name
        components = cdo.get_components_by_class(unreal.SkeletalMeshComponent)
        for comp in components:
            if 'CharacterMesh0' in comp.get_name() or 'Mesh' in comp.get_name():
                mesh_comp = comp
                break
        if mesh_comp is None and len(components) > 0:
            mesh_comp = components[0]
    
    if mesh_comp:
        dump_skeletal_mesh_materials(mesh_comp, f"{label} CDO CharacterMesh0")
    else:
        log("  (Could not find SkeletalMeshComponent)")


def dump_scene_actor(actor_name, label):
    """Find an actor in the level and dump its materials."""
    log(f"\n{'#'*60}")
    log(f"SCENE ACTOR: {label}")
    log(f"Actor Name: {actor_name}")
    log(f"{'#'*60}")
    
    # Find actor
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    target = None
    for actor in actors:
        if actor.get_name() == actor_name or actor.get_actor_label() == actor_name:
            target = actor
            break
    
    if target is None:
        # Try partial match
        for actor in actors:
            if actor_name in actor.get_name():
                target = actor
                log(f"  (Partial match: {actor.get_name()})")
                break
    
    if target is None:
        log(f"  (Actor '{actor_name}' not found in level)")
        return
    
    log(f"Actor Class: {target.get_class().get_name()}")
    
    # Get all SkeletalMeshComponents
    skel_comps = target.get_components_by_class(unreal.SkeletalMeshComponent)
    for comp in skel_comps:
        dump_skeletal_mesh_materials(comp, f"{label} > {comp.get_name()}")
    
    # Also check StaticMeshComponents (WeaponMesh)
    static_comps = target.get_components_by_class(unreal.StaticMeshComponent)
    for comp in static_comps:
        comp_name = comp.get_name()
        if 'Weapon' in comp_name:
            log(f"\n--- WeaponMesh ({comp_name}) ---")
            num_mats = comp.get_num_materials()
            for i in range(num_mats):
                mi = comp.get_material(i)
                if mi:
                    log(f"  Slot {i}: {mi.get_name()} ({mi.get_path_name()})")


# ============================================================
# MAIN
# ============================================================
log("=" * 60)
log("MATERIAL DEBUG REPORT")
log("=" * 60)

# 1. Reimu (Player) - BP CDO
dump_blueprint_cdo("/Game/BP_SekiroCharacter", "Reimu (BP_SekiroCharacter)")

# 2. Patchouli (Enemy) - BP CDO
dump_blueprint_cdo("/Game/BP_SekiroEnemy", "Patchouli (BP_SekiroEnemy)")

# 3. Patchouli Scene Actor
dump_scene_actor("BP_SekiroEnemy_C_0", "Patchouli Scene Actor")

# 4. List all Reimu MIs and Materials
log(f"\n{'#'*60}")
log("ALL REIMU ASSETS")
log(f"{'#'*60}")
reimu_assets = unreal.EditorAssetLibrary.list_assets("/Game/Characters/reimu/", recursive=True)
for asset_path in sorted(reimu_assets):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        log(f"  {asset.get_class().get_name()}: {asset.get_name()} -> {asset_path}")

# 5. List all Patchouli MIs and Materials  
log(f"\n{'#'*60}")
log("ALL PATCHOULI ASSETS")
log(f"{'#'*60}")
patch_assets = unreal.EditorAssetLibrary.list_assets("/Game/Characters/patchouli/", recursive=True)
for asset_path in sorted(patch_assets):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset:
        log(f"  {asset.get_class().get_name()}: {asset.get_name()} -> {asset_path}")

# Save to file
output_path = "C:/Unreal Projects/FYP/debug_materials_output.txt"
with open(output_path, "w", encoding="utf-8") as f:
    f.write("\n".join(output_lines))

log(f"\n\nOutput saved to: {output_path}")
unreal.log_warning(f"Debug output saved to: {output_path}")
