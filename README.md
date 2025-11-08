# group8-TEE - ta_secret

## Description

ta_secret is an application that implements AES encription, a hard coded password validator and an access log.

The app has 3 main functions

1. Encrypt a plain text file ```ta_secret -e <plain text file> key <pwd>```  
2. Decrypt a plain text file ```ta_secret -d key <pwd> ```
3. View access log ```ta_secret -a <entry range begin> <entry range end> <pwd>``` 


## How to run ta_secret

OP-TEE using QEMU v8 must be built and running. A quick tutorial can be found [here.](https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v8) To avoid problems make sure to have all the [prerequisites](https://optee.readthedocs.io/en/latest/building/prerequisites.html) installed.


This complete project can be placed in new folder in ```.../optee/optee_examples```

Create root directory in ```.../optee/build/br-ext/board/qemu/overlay/root``` and place your plain text file there so you don't have to create it every time you run the program. You may need to create the ```root``` directory in the ```overlay``` folder.

Now it's possible to run ```ta_secret```.

 ```cd .../optee/build make run``` 

 This will recompile every time want to run. You can use ``` make run-only``` after compiling the first time.

Finally, in the Normal World shell you can run any of the commands mentioned above in the description. The hard coded password is "Alfonso"




## Run the attacks

To run the attacks you must use Telnet to connect. Run qemu the following way 

```make run-only QEMU_EXTRA_ARGS="-serial tcp:127.0.0.1:5555,server,nowait" ```

In a different terminal, run ```python timing.py``` or ```python3 timing.py``` depending on your python instllation.


### Run attact - race condition

In the directory Build run the command:

```make BR2_PACKAGE_OPTEE_RUST_EXAMPLES_EXT=n QEMU_VIRTFS_ENABLE=y QEMU_USERNET_ENABLE=y run```

Now lets create a share folder to have access to the all the files in the proyect:

``` mkdir -p /mnt/host ```
``` mount -t 9p -o trans=virtio host /mnt/host ```

Now go to the shared folder 

``` cd ../mnt/host/optee_examples/group8-TEE/test/ ```
``` sh raceCondition.sh ```

## Design and implementation

### Trusted application desing 

#### ta/light_crypto_TA.c
- ta2tee_algo_id / ta2tee_key_size / ta2tee_mode_id
  - Map TA-level constants to OP-TEE enums; validate inputs and return TEE_ERROR_BAD_PARAMETERS on invalid values.
- alloc_resources(session, param_types, params)
  - Expects VALUE_INPUT x3: {algo, key_size, mode}.
  - Stores selections in the per-session aes_cipher, allocates TEE_Operation and TEE transient key, loads a dummy key so the op can be reset later.
- set_aes_key(session, param_types, params)
  - Expects MEMREF_INPUT: raw key.
  - Validates size, populates transient object, resets operation, and sets the key on the operation.
- reset_aes_iv(session, param_types, params)
  - Expects MEMREF_INPUT: IV.
  - Re-initializes the cipher via TEE_CipherInit.
- cipher_buffer(session, param_types, params)
  - Expects MEMREF_INPUT (in) + MEMREF_OUTPUT (out).
  - Checks sizes and op state, then calls TEE_CipherUpdate to encrypt/decrypt.

#### ta/pass_validation_TA.c
- password_validation(param_types, params)
  - Expects MEMREF_INPUT (password) + VALUE_OUTPUT (result).
  - Compares the input to a static password and writes 1/0 to params[1].value.a.
  - Logs basic traces for debug.

#### ta/secret_manag_TA.c
- updateLog(param_types, params)
  - Expects MEMREF_INPUT (message) + VALUE_INPUT (aux/id).
  - Opens (or creates) a persistent object (TEE_STORAGE_PRIVATE) using a fixed ID.
  - Maintains a ring buffer: message[10][100], timestamp[10][100], and a monotonically increasing log_count.
  - Reads existing struct (if present), selects row = log_count % 10, bounded-copy of the message, stamps time (TEE_GetSystemTime), increments log_count, then Truncate + Seek + Write the full struct back.
- get_log_entry(param_types, params)
  - Expects MEMREF_OUTPUT (message out) + VALUE_INPUT (index) + MEMREF_OUTPUT (timestamp out).
  - Opens the same object (read-only), reads the struct, selects the requested row, copies message and timestamp into caller buffers (bounded), and sets returned sizes.



### Client application desing 
Entry point and session lifecycle
- main() validates arguments, then calls:
  - prepare_tee_session(): TEEC_InitializeContext + TEEC_OpenSession(TEEC_LOGIN_PUBLIC)
  - terminate_tee_session(): TEEC_CloseSession + TEEC_FinalizeContext
- One session (struct test_ctx) is used for the duration of the command.

Commands and control flow
- Encrypt: ta_secret -e <file> <key> <pwd>
  1) password_validation(pwd)
  2) updateLog("Encrypting")
  3) encrypt_file(file, key)
- Decrypt: ta_secret -d <key> <pwd>
  1) password_validation(pwd)
  2) updateLog("Decrypting")
  3) decrypt_file(key)
- Access log: ta_secret -a <from> <to> <pwd>
  1) Password check via vuln_cmp(pwd, "Alfonso") == 0 (intentional weakness)
  2) updateLog("Accessing log")
  3) for i in [from..to]: getLog(msg, ts, i) and print:
     - Message as ASCII
     - Timestamp as 100-byte hex dump



## Vunerabilities by funtions

### TEE_Result updateLog()

This function is responsible for updating a log that contains only 10 entries and is stored in a persistent object. The function does not perform any synchronization to handle the object in case of multiple instantiations of the same TA, which can lead to race conditions. There are also additional TA and flag configurations that have been intentionally set up to create a clear vulnerability.

The function ``TEE_OpenPersistentObject()`` returns a handle used to access and modify the persistent object. The handle can be set to allow read-only or write-only operations by using the corresponding flags. It is also possible to set flags that allow creating another handle with read-and-write access. In this case, all the flags were used, leaving a vulnerability to race conditions, as shown here:

``` 
uint32_t flags =     TEE_DATA_FLAG_ACCESS_READ |
                     TEE_DATA_FLAG_ACCESS_WRITE |
                     TEE_DATA_FLAG_ACCESS_WRITE_META |
					 TEE_DATA_FLAG_SHARE_READ |
                     TEE_DATA_FLAG_SHARE_WRITE;

res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,OBJ_ID, OBJ_ID_LEN,flags, &obj);
```

Additionally, there is a configuration of the trusted application that allows this race condition to happen. It was necessary to modify the TA flags in the file ``user_ta_header_defines.h: TA_FLAG_SINGLE_INSTANCE`` was removed to allow multiple instances of the TA, and ``TA_FLAG_MULTI_SESSION`` was added to allow multiple sessions to be open at the same time. As a result, the modified line was:

``` 
#define TA_FLAGS    (TA_FLAG_SINGLE_INSTANCE | TA_FLAG_MULTI_SESSION | TA_FLAG_INSTANCE_KEEP_ALIVE) 
```

On the other hand, the effectiveness of this race condition vulnerability depends on the degree of real parallelism and the number of threads and cores the system has.

### TEE_Result get_log_entry()

This function returns a single row from the log; its input is the index into the log array. The main vulnerability lies in the input validation of the index, which should be limited to values between 0 and 9. Data can be easily leaked by scanning stack memory with both positive and negative indices.

``` int32_t entry_index = params[1].value.a; ```

Because it opens and reads the persistent object without synchronization, a race condition can occur when there are multiple instances—especially if it is important to read the log immediately after a specific command.

### TEE_Result password_validation()

This function simply validates the password, which is stored in a static variable. That means it can be easily extracted and will remain in memory for the lifetime of the process. Instead, it should be stored in protected memory; alternatively, it could be hashed and stored in secure persistent storage.

``` static char pass[] = "Alfonso"; ```

Additionally, the string validation uses strcmp, which compares characters sequentially and returns when a mismatch or the end of the string is reached. Using strcmp makes the code vulnerable to timing-analysis attacks: the more initial characters are correct, the longer the comparison takes.

```validated = (strcmp((const char*)input_str, pass_str)==0) ? 1 : 0; ```



## Testing 





## Race condition attack 

In this case the target is the application's log, which is not synchronized when multiple instances of the TA try to read from and write to the persistent object that contains the log.

The attack first performs a baseline test that sends nine bash commands serially to encrypt a file. This first test verifies that the log correctly registers each operation and that no race condition occurs when commands run sequentially. The second test sends nine bash commands to decrypt a file, but this time each command is started in the background so they run in parallel.

As a result, the commands sent serially are all registered in the log as expected, because the log is accessed only once at a time. By contrast, test 2 recorded only 4 of the 9 decryption commands, which indicates the log was read by two TA instances concurrently and only the last writer's updates survived.


## Team Members
- Sebastián Felipe Alfonso Roa
- Daniel Rolando Martínez Teruel
- Alejandro David Montellano Zuna
