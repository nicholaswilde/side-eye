import os
import subprocess
import re

def run_command(command, cwd=None):
    result = subprocess.run(command, shell=True, capture_output=True, text=True, cwd=cwd)
    if result.returncode != 0:
        return None
    return result.stdout

def main():
    firmware_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    print(f"Generating coverage for firmware in {firmware_dir}...")

    # Run tests
    print("Running native tests...")
    run_command("pio test -e native", cwd=firmware_dir)

    # Find object files
    pio_build_dir = os.path.join(firmware_dir, ".pio", "build", "native")
    
    # We want to run gcov on all source/test object files
    coverage_data = []
    
    for root, dirs, files in os.walk(pio_build_dir):
        for file in files:
            if file.endswith(".o"):
                obj_file = os.path.join(root, file)
                # Find corresponding .gcda file
                gcda_file = obj_file.replace(".o", ".gcda")
                if os.path.exists(gcda_file):
                    # Run gcov
                    output = run_command(f"gcov {obj_file}", cwd=firmware_dir)
                    if output:
                        # Parse gcov output line by line
                        current_file = None
                        for line in output.splitlines():
                            file_match = re.match(r"File '(.*?)'", line)
                            if file_match:
                                current_file = file_match.group(1)
                            
                            cov_match = re.match(r"Lines executed:(.*?)% of (\d+)", line)
                            if cov_match and current_file:
                                coverage_data.append({
                                    "file": current_file,
                                    "coverage": float(cov_match.group(1)),
                                    "lines": int(cov_match.group(2))
                                })

    # Filter and summarize
    print("\n--- Firmware Coverage Summary ---")
    print(f"{'Filename':<35} | {'Lines':<10} | {'Coverage':<10}")
    print("-" * 60)
    
    total_lines = 0
    covered_lines = 0
    
    seen_files = {}
    for entry in coverage_data:
        fname = entry["file"]
        # Only keep the highest coverage entry for each file if there are multiple
        if fname not in seen_files or entry["coverage"] > seen_files[fname]["coverage"]:
            seen_files[fname] = entry

    for fname in sorted(seen_files.keys()):
        entry = seen_files[fname]
        
        # Skip library files
        if ".pio/libdeps" in fname or "Unity" in fname or "unity" in fname:
            continue
            
        print(f"{fname:<35} | {entry['lines']:<10} | {entry['coverage']:>8.2f}%")
        
        total_lines += entry["lines"]
        covered_lines += entry["lines"] * (entry["coverage"] / 100.0)

    if total_lines > 0:
        total_percent = (covered_lines / total_lines) * 100
        print("-" * 60)
        print(f"{'TOTAL':<35} | {total_lines:<10} | {total_percent:>8.2f}%")
    else:
        print("No coverage data found.")

if __name__ == "__main__":
    main()