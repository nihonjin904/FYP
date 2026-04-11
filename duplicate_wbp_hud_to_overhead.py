"""
Duplicate WBP_HUD to replace WBP_Overhead for enemy overhead display.

Steps:
1. Delete existing WBP_Overhead
2. Duplicate WBP_HUD as WBP_Overhead
3. Save the new asset

After running this script, the WBP_Overhead will have the exact same
UI layout as WBP_HUD. The BP_SekiroEnemy event graph already calls:
- UpdateHealth(Current, Max)      -> for enemy health bar
- UpdateEnemyPosture(Current, Max) -> for enemy posture bar

Since WBP_HUD (now WBP_Overhead) inherits from SekiroWidgetBase which
has BlueprintImplementableEvent for both, the Blueprint implementations
in the duplicated widget will work as-is.
"""

import unreal

# Paths
source_path = "/Game/WBP_HUD"
dest_path = "/Game/WBP_Overhead"

editor_lib = unreal.EditorAssetLibrary

# Step 1: Check source exists
if not editor_lib.does_asset_exist(source_path):
    unreal.log_error(f"Source asset not found: {source_path}")
    raise RuntimeError(f"Source asset not found: {source_path}")

print(f"[OK] Source found: {source_path}")

# Step 2: Delete existing WBP_Overhead if it exists
if editor_lib.does_asset_exist(dest_path):
    print(f"[INFO] Deleting existing: {dest_path}")
    success = editor_lib.delete_asset(dest_path)
    if not success:
        # Force delete
        unreal.log_warning(f"Normal delete failed, trying force delete for {dest_path}")
        loaded = editor_lib.load_asset(dest_path)
        if loaded:
            unreal.EditorAssetLibrary.delete_loaded_asset(loaded)
            print(f"[OK] Force deleted: {dest_path}")
        else:
            unreal.log_error(f"Cannot delete {dest_path}")
    else:
        print(f"[OK] Deleted existing: {dest_path}")

# Step 3: Duplicate WBP_HUD -> WBP_Overhead
print(f"[INFO] Duplicating {source_path} -> {dest_path}")
result = editor_lib.duplicate_asset(source_path, dest_path)

if result:
    print(f"[OK] Successfully duplicated!")
    
    # Step 4: Save the new asset
    editor_lib.save_asset(dest_path)
    print(f"[OK] Saved: {dest_path}")
    
    # Verify
    new_asset = editor_lib.load_asset(dest_path)
    if new_asset:
        print(f"[OK] Verified: {dest_path} exists and loaded")
        print(f"     Class: {new_asset.get_class().get_name()}")
    else:
        unreal.log_error(f"Failed to load duplicated asset: {dest_path}")
else:
    unreal.log_error(f"Failed to duplicate {source_path} -> {dest_path}")
    raise RuntimeError("Duplication failed")

print("\n=== NEXT STEPS ===")
print("The WBP_Overhead now has the same UI as WBP_HUD.")
print("BP_SekiroEnemy's OverheadWidget already references WBP_Overhead.")
print("The event graph calls UpdateHealth + UpdateEnemyPosture on it.")
print("")
print("IMPORTANT: Open WBP_Overhead in UE Editor and verify:")
print("1. The Event UpdateHealth implementation updates the health ProgressBar")
print("2. The Event UpdateEnemyPosture implementation updates the posture ProgressBar")
print("3. If WBP_HUD had separate player/enemy sections, hide the 'player' section")
