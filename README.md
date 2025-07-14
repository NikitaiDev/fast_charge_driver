# fast_charge_driver

## Overview
The fast_charge_driver is a Linux kernel module that creates the device file /dev/fast_charge. This driver emulates an electric vehicle (EV) fast charging station, allowing user-space programs to simulate starting and stopping a charging session and to query the charging status.

## Supported Commands
The driver recognizes the following commands (via either write or ioctl):
| Command        | Description                                                               |
| -------------- | ------------------------------------------------------------------------- |
| `START_CHARGE` | Start the charging process (begin fast charge simulation).                |
| `STOP_CHARGE`  | Stop the charging process.                                                |
| `GET_STATUS`   | Get the current charging status (charging state and related information). |

## Dependencies
```bash
sudo apt-get update
sudo apt-get install -y build-essential linux-headers-$(uname -r) bash coreutils libc6-dev kmod python3 python3-full python3-pip python3-requests python3-venv
# For Debian; if you wish to install a non-Debian-packaged Python package
python3 -m venv .vevn 
source .venv/bin/activate
pip install requests
#
pip install pyserial
```

## Build
To build the module, run:
```bash
make
```

## Installation (Load the module)
To install (load) the fast_charge driver into the kernel, run:
```bash
sudo make insmod
```
This will insert the compiled module into the kernel (using insmod fast_charge.ko). Once loaded, the driver will register a character device and typically create the device node /dev/fast_charge (this may be done automatically via udev if the driver uses a device class). Tip: You may need root privileges to load the module. The make insmod command may use sudo internally or you can run sudo insmod fast_charge.ko manually.

## Removal (Unload the module)
To unload the driver from the kernel, run:
```bash
sudo make rmmod
```
This will remove the module from the kernel (using rmmod). After removal, the device /dev/fast_charge will no longer be available. You might need root privileges for this operation as well.

## Usage
### Sending Commands via Write/Read
You can send commands to the driver by writing the command name as text to /dev/fast_charge. For example, using a shell:
```bash
echo -n "S" > /dev/fast_charge  # START_CHARGE
echo -n "P" > /dev/fast_charge  # STOP_CHARGE
dd if=/dev/fast_charge bs=16 count=1 2>/dev/null | od -An -t u4  # GET_STATUS
```

### IOCTL Example (C Program)
Here is a simplified example in C showing how to use ioctl calls to interact with the driver:
```C
int fd = open("/dev/fast_charge", O_RDWR);
ioctl(fd, FAST_CHARGE_START);
ioctl(fd, FAST_CHARGE_STATUS, &status);
ioctl(fd, FAST_CHARGE_STOP);
close(fd);
```
## Test
### Write/Read
```bash
sudo ./tests/test_wr.sh
```

### IOCTL
```bash
make test && ./tests/test_ioctl
```
