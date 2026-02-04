import os
import re
from pathlib import Path

Import("env")

def get_version():
    cargo_toml_path = Path(env.subst("$PROJECT_DIR")).parent / "host" / "Cargo.toml"
    if not cargo_toml_path.exists():
        print(f"Warning: Cargo.toml not found at {cargo_toml_path}")
        return "0.0.0-unknown"
    
    with open(cargo_toml_path, "r") as f:
        content = f.read()
        match = re.search(r'^version\s*=\s*"([^"]+)"', content, re.MULTILINE)
        if match:
            return match.group(1)
    
    return "0.0.0-error"

version = get_version()
print(f"--- Synchronized Firmware Version from Host: {version} ---")
# Pass the version as a string literal
env.Append(BUILD_FLAGS=[f'-DFIRMWARE_VERSION=\\"{version}\\"'])