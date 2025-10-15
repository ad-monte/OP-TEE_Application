import pexpect
import time

# Connect to QEMU console (adjust host/port as needed)
# The default OP-TEE QEMU port is usually 54320
console_host = "127.0.0.1"
console_port = 5555

# Spawn a telnet session to QEMU console
child = pexpect.spawn(f"telnet {console_host} {console_port}", timeout=30)

# Send an empty line to trigger Buildroot's login prompt
child.sendline("")  # equivalent to pressing Enter

# Wait for the login prompt to appear
child.expect("login:")

# Log in as root
child.sendline("root")

# Wait for shell prompt
child.expect("#")

# Confirm connection
child.sendline("echo 'Connected to Buildroot shell'")
child.expect("#")

# Run your Trusted Application (CA)
child.sendline("my_ta sampletext.txt")
child.expect("#")

child.sendline("exit")

# Capture and display output
output = child.before.decode("utf-8", errors="ignore")
print("Output from my_ta client:\n", output)

# Exit cleanly
child.sendline("exit")
child.close()
