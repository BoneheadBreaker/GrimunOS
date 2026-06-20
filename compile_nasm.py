import subprocess
import os

# Create the output directory if missing
if not os.path.exists("Binaries"):
    os.makedirs("Binaries")

# Fetch standard Windows Environment Variables dynamically
# os.environ.get('USERPROFILE') resolves to "C:\Users\<Current_User>"
# os.environ.get('APPDATA') resolves to "C:\Users\<Current_User>\AppData\Roaming"
user_profile = os.environ.get('USERPROFILE', '')
appdata_roaming = os.environ.get('APPDATA', '')

# Build dynamic fallback paths using os.path.join (safely handles backslashes)
nasm_path = os.path.join(user_profile, "AppData", "Local", "NASM", "nasm.exe")

# Check Program Files
if not os.path.exists(nasm_path):
    nasm_path = r"C:\Program Files\NASM\nasm.exe"

# Check Roaming AppData folder dynamically
if not os.path.exists(nasm_path) and appdata_roaming:
    nasm_path = os.path.join(appdata_roaming, "NASM", "nasm.exe")

# Let Windows search your System PATH environment variables
if not os.path.exists(nasm_path):
    nasm_path = "nasm"

print(f"[Python Assembler] Compiling boot sector files using target tool...")
print(f"  -> Using NASM path: {nasm_path}")

try:
    # Assemble boot.asm
    subprocess.run([nasm_path, "boot.asm", "-f", "bin", "-o", "Binaries/boot.bin"], check=True)
    print("  -> Successfully created Binaries/boot.bin")

    # Assemble entry.asm
    subprocess.run([nasm_path, "entry.asm", "-f", "elf32", "-o", "Binaries/entry.o"], check=True)
    print("  -> Successfully created Binaries/entry.o")
except Exception as e:
    print(f"\n[Error] Python couldn't run NASM path. Diagnostic trace: {e}")
    print("Please make sure you know the exact folder where nasm.exe is installed!")
