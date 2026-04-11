"""
Duplicate WBP_HUD -> WBP_Overhead using UE5's Remote Execution protocol.
Requires Python Scripting Plugin enabled in the running UE editor.
"""

import sys
sys.path.insert(0, r"C:\Program Files\Epic Games\UE_5.5\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python")

import remote_execution

# The Python code to execute inside Unreal
UNREAL_CODE = '''
import unreal

source_path = "/Game/WBP_HUD"
dest_path = "/Game/WBP_Overhead"
editor_lib = unreal.EditorAssetLibrary

# Check source
if not editor_lib.does_asset_exist(source_path):
    print("ERROR: Source not found: " + source_path)
else:
    # Delete existing WBP_Overhead
    if editor_lib.does_asset_exist(dest_path):
        editor_lib.delete_asset(dest_path)
        print("Deleted existing: " + dest_path)
    
    # Duplicate
    result = editor_lib.duplicate_asset(source_path, dest_path)
    if result:
        editor_lib.save_asset(dest_path)
        print("SUCCESS: Duplicated " + source_path + " -> " + dest_path)
        
        # Verify
        new_asset = editor_lib.load_asset(dest_path)
        if new_asset:
            print("VERIFIED: " + dest_path + " class=" + new_asset.get_class().get_name())
        else:
            print("WARNING: Could not load duplicated asset")
    else:
        print("ERROR: Duplication failed")
'''

def main():
    print("Connecting to Unreal Engine Remote Execution...")
    
    remote_exec = remote_execution.RemoteExecution()
    remote_exec.start()
    
    # Wait a bit for node discovery
    import time
    time.sleep(1)
    
    nodes = remote_exec.remote_nodes
    print(f"Found {len(nodes)} remote node(s)")
    
    if not nodes:
        print("ERROR: No Unreal Editor instances found.")
        print("Make sure Python Scripting Plugin is enabled in UE with Remote Execution enabled.")
        remote_exec.stop()
        return False
    
    print(f"Executing on node: {nodes[0]}")
    
    # Execute the code
    try:
        result = remote_exec.run_command(UNREAL_CODE)
        print(f"\n=== RESULT ===")
        print(f"Success: {result.get('success', 'unknown')}")
        
        output = result.get('output', '')
        if output:
            print(f"Output:\n{output}")
        
        command_result = result.get('result', '')
        if command_result:
            print(f"Return: {command_result}")
            
    except Exception as e:
        print(f"Execution error: {e}")
        remote_exec.stop()
        return False
    
    remote_exec.stop()
    return True

if __name__ == "__main__":
    success = main()
    if not success:
        print("\n=== FALLBACK: Manual Instructions ===")
        print("In UE Editor, go to: Edit > Editor Preferences > Python")
        print("Enable 'Remote Execution'")
        print("")
        print("Or paste this into the Output Log (Python console):")
        print("=" * 50)
        print(UNREAL_CODE)
        sys.exit(1)
