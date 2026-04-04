"""
Check and fix BP_SekiroEnemy's AnimClass and SkeletalMesh assignment.
Also check what AnimInstance the enemy actually has at runtime.
"""
import unreal

# --- Check BP_SekiroEnemy's default settings ---
bp = unreal.load_asset("/Game/BP_SekiroEnemy")
if not bp:
    print("ERROR: Could not load BP_SekiroEnemy")
else:
    print("BP_SekiroEnemy loaded OK")
    cdo = unreal.get_default_object(bp.generated_class())
    
    # Check mesh
    mesh = cdo.get_editor_property("Mesh") if hasattr(cdo, "Mesh") else None
    
    # Try getting the SkeletalMesh component
    mesh_comp = cdo.find_component_by_class(unreal.SkeletalMeshComponent)
    if mesh_comp:
        print(f"  SkeletalMesh: {mesh_comp.skeletal_mesh}")
        print(f"  AnimClass: {mesh_comp.anim_class}")
        
        # Check what ABP is assigned
        current_anim = mesh_comp.anim_class
        target_abp = unreal.load_asset("/Game/ABP_SekiroCharacter_Patchouli")
        if target_abp:
            target_class = target_abp.generated_class()
            if current_anim != target_class:
                print(f"  MISMATCH! Current: {current_anim}, Expected: {target_class}")
                print("  Fixing AnimClass...")
                mesh_comp.set_editor_property("anim_class", target_class)
                unreal.EditorAssetLibrary.save_asset("/Game/BP_SekiroEnemy")
                print("  Fixed and saved!")
            else:
                print("  AnimClass is already correct!")
        else:
            print("  ERROR: Could not load ABP_SekiroCharacter_Patchouli")
    else:
        print("  No SkeletalMeshComponent found on CDO")

# --- Also check what animations exist for Patchouli ---
print("\n--- Patchouli Animations ---")
patchouli_anims = unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_path(
    "/Game", recursive=True
)
for asset_data in patchouli_anims:
    name = str(asset_data.asset_name)
    if "Patchouli" in name and "AnimSequence" in str(asset_data.asset_class_path):
        print(f"  {asset_data.object_path}")
