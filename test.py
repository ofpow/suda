#!/usr/bin/python3
import sys
import os
import subprocess

def record():
    for entry in os.scandir("./tests"):
        if entry.is_file() and entry.path.endswith(".suda"):
            print(f"[CMD] suda {entry.path} > {entry.path[:-len('.suda')] + '.txt'}")
            output = subprocess.run(["./suda", entry.path], capture_output=True, text=True)
            if output.returncode == 0:
                with open((entry.path[:-len(".suda")] + ".txt"), "w") as f:
                    f.write(output.stdout)
            else:
                print(f"[ERR] {entry.path} exited with non-zero exit code")

def run_tests():
    sucesses, failures = 0, 0
    cmd = ["./suda"]

    for entry in os.scandir("./tests"):
        if entry.is_file() and entry.path.endswith(".suda"):
            if os.path.isfile(entry.path[:-len(".suda")] + ".txt"):
                with open (entry.path[:-len(".suda")] + ".txt", "r") as f:
                    cmd.append(entry.path)
                    if (subprocess.run(cmd, capture_output=True, text=True).stdout == f.read()):
                        sucesses += 1
                    else:
                        print(f"[FAIL] test {entry.path} failed")
                        failures += 1
                    cmd.pop()
            else:
                print(f"[ERR] test {entry.path} has no expected output")

    print(f"BYTECODE:  {sucesses} tests succeded and {failures} tests failed")

if __name__ == "__main__":
    print("[CMD] gcc -o suda suda.c")
    subprocess.run(["gcc", "-o", "suda", "suda.c"])
    if len(sys.argv) != 2:
        run_tests()
    elif sys.argv[1] == 'record':
        record()
