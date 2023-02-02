#!/usr/bin/python3
import sys
import os
import subprocess

sucesses, failures = 0, 0

if __name__ == "__main__":
    print("[CMD] gcc -o suda suda.c")
    subprocess.run(["gcc", "-o", "suda", "suda.c"])
    if len(sys.argv) != 2:
        for entry in os.scandir("./tests"):
            if entry.is_file() and entry.path.endswith(".suda"):
                print(f"[TEST] {entry.path}")
                if os.path.isfile(entry.path[:-len(".suda")] + ".txt"):
                    with open (entry.path[:-len(".suda")] + ".txt", "r") as f:
                        if (subprocess.run(["./suda", entry.path], capture_output=True, text=True).stdout == f.read()):
                            sucesses += 1
                        else:
                            print(f"[FAIL] test {entry.path} failed")
                            failures += 1
                else:
                    print(f"[ERR] test {entry.path} has no expected output")
        print(f"{sucesses} tests succeded and {failures} tests failed")
    elif sys.argv[1] == 'record':
        for entry in os.scandir("./tests"):
            if entry.is_file() and entry.path.endswith(".suda"):
                print(f"[CMD] suda {entry.path} > {entry.path[:-len('.suda')] + '.txt'}")
                output = subprocess.run(["./suda", entry.path], capture_output=True, text=True)
                if output.returncode == 0:
                    with open((entry.path[:-len(".suda")] + ".txt"), "w") as f:
                        f.write(output.stdout)
                else:
                    print(f"[ERR] {entry.path} exited with non-zero exit code")