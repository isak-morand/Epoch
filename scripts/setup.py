import os
import subprocess
import platform
import argparse

def main():
    parser = argparse.ArgumentParser(description="Generate Premake project files.")
    parser.add_argument('--target', choices=['editor', 'runtime'], required=True, help='Project target: editor or runtime')
    args = parser.parse_args()

    target = args.target.lower()

    # Detect the current OS
    current_os = platform.system().lower()   # "windows", "linux", "darwin", etc.

    # Build a map of menu choices → API flags, based on OS
    api_map = {}
    menu_items = []

    # On Windows, allow DX11, DX12, Vulkan
    if current_os == "windows":
        api_map["1"] = "dx11"
        api_map["2"] = "dx12"
        api_map["3"] = "vulkan"
        menu_items = [
            ("1", "DirectX 11"),
            ("2", "DirectX 12"),
            ("3", "Vulkan"),
        ]

    # On Linux, omit DX11/DX12 (only Vulkan is supported)
    elif current_os == "linux":
        api_map["1"] = "vulkan"
        menu_items = [
            ("1", "Vulkan"),
        ]

# Print the menu
    print("Select rendering API for project generation:")
    for key, label in menu_items:
        print(f"  {key}) {label}")

    choice = input(
        f"Enter choice [{menu_items[0][0]}-{menu_items[-1][0]}]: "
    ).strip()

    # Map to an API, or default to the first in menu_items
    api = api_map.get(choice, api_map[menu_items[0][0]])
    print(f"\nGenerating project for API: {api.upper()}\n")

    # Locate premake executable
    premake_exe = os.path.join("vendor", "Premake", "premake5.exe")

    if not os.path.isfile(premake_exe):
        print(f"Error: Could not find premake executable at: {premake_exe}")
        exit()

    # Build the premake command
    cmd = [premake_exe, "vs2022", f"--render-api={api}", f"--target={target}"]

    # Execute the command
    subprocess.call(cmd)

if __name__ == "__main__":
    main()
