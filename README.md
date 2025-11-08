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


## Team Members
- Sebastián Felipe Alfonso Roa
- Daniel Rolando Martínez Teruel
- Alejandro David Montellano Zuna
