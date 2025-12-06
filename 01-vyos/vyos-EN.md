# VyOS: The Virtual Router

This documentation aims to teach how to configure the virtual router VyOS 1.4.3 [sagitta].

------

### Prerequisites

- VyOS ISO provided by the institution **[vyos-1.4.3-generic-amd64.iso]**
- Oracle VirtualBox in its latest version

------

## 1. VyOS

VyOS is a Debian-based Linux operating system. It provides the same services as a hardware router. If installed on a physical machine, it operates as a physical router. If installed in a virtual environment, it operates as a virtual router.

We will learn how to install VyOS and configure this virtual router.

------

### 1.1 Creating the Virtual Machine

Let’s create a virtual machine running VyOS. This system will be installed on the VM’s virtual disk.

On VirtualBox:

1. New

2. Set a VM name: `VyOS-og`

3. Select the VyOS ISO file

4. Choose the hardware specifications

   We keep the default hardware specifications for this first VM.

5. Define the virtual disk specifications and confirm the settings

6. Configuration → Network: Add **three network interfaces**

The VM is now ready for its first startup.

------

### 1.2 Automatically Setting the AZERTY Keyboard

Before starting the VM, and to avoid conflicts, we temporarily disable KVM modules:

```bash

sudo modprobe -r kvm_amd kvm
```

------

#### First Login

Login using:

- `vyos login: vyos`
- `Password: vyos`

------

#### Temporary Keyboard Layout

By default, the keyboard is in QWERTY. Change it temporarily using:

```bash

sudo loadkeys fr
```

------

#### Starting the Installation

Launch the VyOS installation:

```bash
install image
```

Choose boot option **1**: `/opt/vyatta/etc/config/config.boot`.
 Then shut down the VM using:

```bash
poweroff
```

Once powered off, eject the ISO used for installation.

------

#### Permanent Keyboard Configuration

To permanently configure the keyboard layout to AZERTY/French:

```bash
configure
set system option keyboard-layout fr
commit
save
```

Restart the VM to confirm that the modification has been applied.

The keyboard is now correctly set to French!

------

### 1.3 Automatic Homogenization of Network Interfaces

The purpose of this section is to configure automatic homogenization of network interface names. For example, the first VM interface should be `eth0`, the second `eth1`, and so on, logically incremented.

If we clone the original virtual router, the cloned VM must automatically generate new MAC addresses and must start again from `eth0`, `eth1`, etc.

------

#### First Clone Attempt

Let’s perform a first clone by generating new MAC addresses for all network interfaces.

After starting the cloned VM, we notice the following problem:

The network interfaces start at `eth4`. This is because the original router stopped at `eth3` (four interfaces). VyOS interprets new MAC addresses as **physical changes** and continues counting from the previous configuration.

We can now delete this test clone and all associated files.

------

#### Removing MAC Addresses From the Original VM

Since a cloned VM has different MAC addresses than the original VM, we must ensure that no interface keeps a static MAC address.

On the original VM, start the configuration mode:

```bash
configure
```

Then delete MAC addresses for each interface:

```bash

del interfaces ethernet eth0 hw-id
del interfaces ethernet eth1 hw-id
del interfaces ethernet eth2 hw-id
del interfaces ethernet eth3 hw-id
commit
save
exit
```

VyOS has a particular behavior: if a MAC address is modified or removed, it interprets this as a **physical hardware change**, and thus interface numbering may continue where it left off.

To prevent this, we must delete MAC addresses on the original VM **before cloning**.

------

#### New Clone Attempt

Now we clone the VM again with the following option enabled:

> **OS Installation Options: Generate new MAC addresses for all network interfaces**

After starting the clone, verify the interface numbering.

The interfaces of the cloned VM now start at `eth0` and are incremented properly.

------

We have successfully completed the first project: **VyOS as a Virtual Router**.