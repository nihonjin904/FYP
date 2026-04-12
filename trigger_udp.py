import socket
import json

code = "import unreal\nunreal.SystemLibrary.execute_console_command('LiveCoding.Compile')"
msg = json.dumps({"type": "command", "body": code}).encode('utf-8')
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(5)
print("Sending UDP multicast to trigger Live Coding...")
sock.sendto(msg, ("239.0.0.1", 6776))

try:
    data, addr = sock.recvfrom(65535)
    print(f"Success! Got response from {addr}: {data[:200]}")
except socket.timeout:
    print("No response from UDP multicast (timeout). Live Coding might still have started, or the plugin is disabled.")
finally:
    sock.close()
