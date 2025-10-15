# test_optee.py
import subprocess
import time
import pexpect

class OpteeQemuTest:
    def __init__(self, optee_path):
        self.optee_path = optee_path
        self.qemu_process = None
        
    def start_qemu(self):
        """Start QEMU with OP-TEE"""
        cmd = f"cd {self.optee_path}/build && make run-only"
        self.qemu_process = pexpect.spawn('/bin/bash', ['-c', cmd])
        
        # Wait for boot
        self.qemu_process.expect('buildroot login:', timeout=120)
        self.qemu_process.sendline('root')
        time.sleep(2)
        
    def run_optee_test(self, test_name):
        """Run a specific OP-TEE test"""
        self.qemu_process.sendline(f'optee_example_{test_name}')
        index = self.qemu_process.expect(['PASS', 'FAIL', pexpect.TIMEOUT], timeout=30)
        
        if index == 0:
            return True, "Test passed"
        elif index == 1:
            return False, "Test failed"
        else:
            return False, "Test timeout"
    
    def stop_qemu(self):
        """Stop QEMU"""
        if self.qemu_process:
            self.qemu_process.sendline('poweroff')
            self.qemu_process.expect(pexpect.EOF, timeout=30)

# Example usage
if __name__ == '__main__':
    test = OpteeQemuTest('/home/david/Documents/POLITO/Cybersecurity/Project/optee')
    
    try:
        print("Starting QEMU...")
        test.start_qemu()
        
        print("Running tests...")
        success, msg = test.run_optee_test('hello_world')
        print(f"Test result: {msg}")
        
    finally:
        test.stop_qemu()
