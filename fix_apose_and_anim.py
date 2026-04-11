import unreal

def log(msg):
    unreal.log_warning("[FixScript] " + str(msg))

def run_fixes():
    log("=== STARTING AUTOREPAIR FOR A-POSE & RETARGET ===")
    
    # 1. FIX A-POSE
    bp_path = "/Game/BP_SekiroEnemy"
    abp_new_path = "/Game/ABP_SekiroEnemy_New"
    
    bp_asset = unreal.load_asset(bp_path)
    abp_asset = unreal.load_asset(abp_new_path)
    
    if bp_asset and abp_asset:
        cdo = unreal.get_default_object(bp_asset.generated_class())
        if cdo:
            mesh_comp = cdo.get_editor_property("Mesh")
            if mesh_comp:
                mesh_comp.set_editor_property("AnimClass", abp_asset.generated_class())
                log("Successfully set AnimClass on BP_SekiroEnemy CDO.")
                unreal.EditorAssetLibrary.save_asset(bp_path)
    
    # Also update all placed instances in the world
    world = unreal.EditorLevelLibrary.get_editor_world()
    if world:
        actors = unreal.GameplayStatics.get_all_actors_of_class(world, bp_asset.generated_class())
        for actor in actors:
            mesh_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
            if mesh_comp:
                mesh_comp.set_editor_property("AnimClass", abp_asset.generated_class())
                log(f"Updated AnimClass on placed instance: {actor.get_actor_label()}")
                
    # 2. RETARGET A_Upward_Thrust
    RETARGETER_PATH = "/Game/boss_anim_retarget/RTG_NewRetargeter"
    SOURCE_ANIM = "/Game/boss_animation/A_Upward_Thrust"
    
    # We will use the same source mesh that the anim uses
    anim_seq = unreal.load_asset(SOURCE_ANIM)
    source_mesh = anim_seq.get_editor_property("skeleton").get_editor_property("preview_skeletal_mesh") if anim_seq else None
    
    # Target mesh for Patchouli
    TARGET_MESH = "/Game/Characters/patchouli/SK__\u9b54\u738b\u7523_\u30d1\u30c1\u30e5\u30ea\u30fc\u30fb\u30ce\u30fc\u30ec\u30c3\u30b8"
    target_mesh = unreal.load_asset(TARGET_MESH)
    
    retargeter = unreal.load_asset(RETARGETER_PATH)
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_ad = asset_registry.get_asset_by_object_path(SOURCE_ANIM + "." + SOURCE_ANIM.split("/")[-1])
    
    if retargeter and anim_ad.is_valid() and source_mesh and target_mesh:
        results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
            assets_to_retarget=[anim_ad],
            source_mesh=source_mesh,
            target_mesh=target_mesh,
            ik_retarget_asset=retargeter,
            search="",
            replace="",
            prefix="",
            suffix="",
            include_referenced_assets=True
        )
        
        for r in results:
            old_name = str(r.package_name)
            new_name = "/Game/boss_anim_retarget/" + str(r.asset_name)
            if old_name != new_name:
                unreal.EditorAssetLibrary.rename_asset(old_name, new_name)
            log(f"Retargeted and moved to: {new_name}")
    else:
        log("ERROR: Could not load assets for retargeting. Make sure Paths are correct.")
    
    log("=== AUTOREPAIR COMPLETED! Please compile C++ and test in editor. ===")

if __name__ == "__main__":
    run_fixes()
