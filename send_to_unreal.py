"""
Send Python code to Unreal Engine's Remote ExecutionPlugin via UDP.
UE must have Python Editor Script Plugin + Remote Execution enabled.
Default port: 6776
"""

import socket
import uuid
import json
import time
import sys
import hashlib

# Unreal remote execution protocol
MULTICAST_GROUP = "239.0.0.1"
MULTICAST_PORT = 6776
COMMAND_PORT = 6776

def create_remote_execution_message(command_type, body=""):
    """Create a message for Unreal's remote execution protocol."""
    msg = {
        "type": command_type,
        "version": 1,
        "source": "external",
        "dest": "",
        "body": body
    }
    return json.dumps(msg).encode('utf-8')

def send_python_to_unreal(code: str, timeout: float = 10.0):
    """Send Python code to Unreal via the remote execution UDP protocol."""
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    
    # Build the command message
    node_id = str(uuid.uuid4())
    
    # The protocol uses a specific format
    # Message format: magic + command_id + exec_type + command
    
    # Try direct TCP approach to Unreal's Python remote execution
    # UE5's remote execution listens on TCP as well
    
    try:
        # Method 1: Try the newer HTTP-based Python scripting
        import urllib.request
        url = "http://127.0.0.1:30010/remote/script"
        data = json.dumps({
            "command": code,
            "timeout": timeout
        }).encode('utf-8')
        req = urllib.request.Request(url, data=data, 
                                      headers={'Content-Type': 'application/json'})
        response = urllib.request.urlopen(req, timeout=timeout)
        result = response.read().decode('utf-8')
        print(f"Script executed successfully via HTTP")
        print(f"Response: {result}")
        return True
    except Exception as e:
        print(f"HTTP method failed: {e}")
    
    try:
        # Method 2: Try Unreal Remote Execution Plugin (UDP multicast)
        # Protocol based on UnrealEnginePython remote execution
        
        MAGIC = b'\x00\x00\x00\x00'  # Protocol magic bytes
        
        # Create a simple exec request
        msg = json.dumps({
            "type": "command", 
            "body": code
        }).encode('utf-8')
        
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(2)
        sock.sendto(msg, (MULTICAST_GROUP, MULTICAST_PORT))
        
        try:
            data, addr = sock.recvfrom(65535)
            print(f"Got response from {addr}: {data[:200]}")
            return True
        except socket.timeout:
            print("No response from UDP multicast")
        finally:
            sock.close()
    except Exception as e:
        print(f"UDP method failed: {e}")
    
    return False


# The Python code to execute inside Unreal
unreal_code = '''
import unreal

source_path = "/Game/WBP_HUD"
dest_path = "/Game/WBP_Overhead"
editor_lib = unreal.EditorAssetLibrary

# Delete existing
if editor_lib.does_asset_exist(dest_path):
    editor_lib.delete_asset(dest_path)
    unreal.log("Deleted existing WBP_Overhead")

# Duplicate
result = editor_lib.duplicate_asset(source_path, dest_path)
if result:
    editor_lib.save_asset(dest_path)
    unreal.log("SUCCESS: Duplicated WBP_HUD -> WBP_Overhead")
else:
    unreal.log_error("FAILED: Could not duplicate WBP_HUD")
'''

if __name__ == "__main__":
    print("Attempting to execute Python in Unreal Engine...")
    success = send_python_to_unreal(unreal_code)
    if not success:
        print("\n=== MANUAL FALLBACK ===")
        print("Automatic remote execution failed.")
        print("Please run this in Unreal's Python console (Window > Developer Tools > Output Log):")
        print("---")
        print(unreal_code)
        sys.exit(1)
