# group8-TEE - ta_secret

## Description

ta_secret is an application that implements AES encription, a hard coded password validator and an access log.

The application has 3 main functions:

1. Encrypt a plain text file ```ta_secret -e <plain text file> <key> <pwd>```  
2. Decrypt a plain text file ```ta_secret -d <key> <pwd>```
3. View access log ```ta_secret -a <entry range begin> <entry range end> <pwd>```

## How to run ta_secret

OP-TEE using QEMU v8 must be built and running. A quick tutorial can be found [here.](https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v8) To avoid problems make sure to have all the [prerequisites](https://optee.readthedocs.io/en/latest/building/prerequisites.html) installed.


This complete project can be placed in new folder in ```.../optee/optee_examples```

Create ```root``` directory in ```.../optee/build/br-ext/board/qemu/overlay/``` and place your plain text file there so you don't have to create it every time you run the program. 

Now it's possible to run ```ta_secret```.

 ```cd .../optee/build make run``` 

 This will recompile every time want to run. You can use ``` make run-only``` after compiling the first time.

Finally, in the Normal World shell you can run any of the commands mentioned above in the description. The hard coded password is __Alfonso__




## Run the attacks
To perform the attacks, we have to add some flags to the Makefile (optee-qemu/build/Makefile) before compiling OPTEE. These flags are:

Enable Python inside OP-TEE. Place this line on top of the Makefile:

``EXPORT BR2_PACKAGE_PYTHON3=y``


To mount on QEMU the shared folder with the host:

``EXPORT QEMU_VIRTFS_AUTOMOUNT=y``

``EXPORT QEMU_VIRTFS_ENABLE=y``

``EXPORT QEMU_USERNET_ENABLE=y``

The first flag is to install python in the normal world, so we can run the python scripts for the attacks. The other three flags are to allow sharing folder memory with the host, so we can view and edit the attack files from the host OS.

We also access the file optee-qemu/optee-os/mk/config.mk and modify ``CFG_TA_ASLR ?= n`` to ``y``.

In case there are some conflicts with rust examples it is also possible to add the following:
``EXPORT BR2_PACKAGE_OPTEE_RUST_EXAMPLES_EXT=n``


In the directory Build run the command:

```make run```

Optionally, it is always possible to run the code by placing all the flags when calling make:

```make BR2_PACKAGE_PYTHON3=y QEMU_VIRTFS_AUTOMOUNT=y QEMU_VIRTFS_ENABLE=y QEMU_USERNET_ENABLE=y BR2_PACKAGE_OPTEE_RUST_EXAMPLES_EXT=n run```

Now lets create a share folder to have access to the all the files in the proyect:

``` mkdir -p /mnt/host ```
``` mount -t 9p -o trans=virtio host /mnt/host ```

Now go to the shared folder 

``` cd ../mnt/host/optee_examples/group8-TEE/test/ ```

In this folder we can run the attacks, using python3 for the python scripts and sh for the bash scripts.

```
sh raceCondition.sh 
python3 memory_mapping.py 
python3 timing.py
 ```

