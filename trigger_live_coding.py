import urllib.request
import json

url = "http://127.0.0.1:30010/remote/script"
print("Triggering Live Coding Compilation...")
try:
    code = "import unreal\nunreal.SystemLibrary.execute_console_command('LiveCoding.Compile')"
    data = json.dumps({"command": code, "timeout": 30.0}).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'})
    response = urllib.request.urlopen(req, timeout=30.0)
    print("Success: ", response.read().decode())
except Exception as e:
    print("Error:", e)
