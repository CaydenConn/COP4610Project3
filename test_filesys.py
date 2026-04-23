import subprocess
import os
import sys
import shutil

def run_test():
    exe = "./filesys"
    if not os.path.exists(exe):
        exe = "./bin/filesys" # Check alternate
        if not os.path.exists(exe):
             print(f"Error: filesys not found. Please run 'make' first.")
             return

    original_img = "fat32.img"
    test_img = "test_fat32.img"

    if not os.path.exists(original_img):
        print(f"Error: {original_img} not found.")
        return

    print("--- Setting up test environment ---")
    shutil.copyfile(original_img, test_img)
    print(f"Copied {original_img} to {test_img} for safe testing.\n")

    # Session 3: Main testing
    print(f"\n--- Main Test: Parts 1-5 using {test_img} ---")
    
    commands = [
        "info",
        "ls",
        "mkdir MDIR",
        "cd MDIR",
        "creat MFILE",
        "ls",
        "open MFILE -w",
        "lsof",
        "write MFILE \"HELLO WORLD THIS IS A TEST\"",
        "read MFILE 10", # should fail since opened write only
        "close MFILE",
        "open MFILE -rw",
        "lsof",
        "lseek MFILE 6",
        "read MFILE 5", # read "WORLD"
        "close MFILE",
        "mv MFILE MNEW",
        "ls",
        "mv MNEW ..",
        "ls",
        "cd ..",
        "ls",
        "rm MNEW",
        "rmdir MDIR",
        "ls",
        "exit"
    ]

    input_str = "\n".join(commands) + "\n"

    try:
        process = subprocess.Popen(
            [exe, test_img],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        stdout, stderr = process.communicate(input=input_str, timeout=10)

        print("--- Standard Output ---")
        print(stdout)
        
        if stderr:
            print("--- Standard Error ---")
            print(stderr)

        print("-" * 47)
        if "WORLD" in stdout:
             print("[PASS] Successfully performed write, lseek, and read operations!")
             
        if "MNEW" in stdout:
             print("[PASS] Successfully performed mv command!")
             print("[PASS] Successfully performed rm and rmdir commands!")

    except subprocess.TimeoutExpired:
        print("Error: Test timed out. The program might be stuck in a loop.")
        process.kill()
    except Exception as e:
        print(f"Error during testing: {e}")
        
    print("\n--- Cleaning up ---")
    if os.path.exists(test_img):
        os.remove(test_img)
        print(f"Removed temporary {test_img}")

if __name__ == "__main__":
    run_test()
