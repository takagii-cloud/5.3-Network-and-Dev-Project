# **Microcore: the console-mode terminal**

This documentation is intended to teach how to configure Microcore.

------

### **Prerequisites**

- Tinycore 6.2 ISO
- VirtualBox in its latest version

------

## **3. Microcore**

MicroCore is a very lightweight Linux distribution running in console mode. We will learn how to install it, configure a non-persistent system, optimize it, and use it as a virtual console.

------

#### **3.1 Install the Virtual Machine**

Let’s create a virtual machine integrating the Microcore system.

In VirtualBox:

1. New
2. Specify a name for the virtual machine: `TinyCore-og`
3. Select the Microcore ISO: `TinyCore-6.2.iso`
4. Choose the hardware specifications

Our machine is ready for its first startup.

------

#### **First startup**

Choose the option `Boot TinyCore`.

------

You log in without a password during the first connection. Then download the keymaps:

```shell
tce-load -wi kmaps
```

Then change the keyboard layout:

```shell
sudo loadkmap < /usr/share/kmap/azerty/fr.kmap
```

------

#### **Persistent installation**

Install the TinyCore installer:

```shell
tce-load -wi tc-install
```

Then launch it:

```shell
sudo tc-install.sh
```

- Install from: `r`
- Enter boot directory: `/mnt/sr0/boot`
- Select install type: `f`
- Select target for installation of core: `1. Whole Disk`
- Select disk for core: `1`
- Would you like to install a bootloader: `y`
- Select formatting option: **leave the line empty**

------

### **3.2 Automatic integration of the AZERTY keyboard**

1. Install the `kmaps` package:

```shell
tce-load -wi kmaps
```

1. Load the French keymap:

```shell
sudo loadkmap < /usr/share/kmap/azerty/fr-latin1.kmap
```

------

### **3.3 Make `/home` and `/opt` persistent**

1. Add `/home` to persistence:

```shell
echo "/home" | sudo tee -a /opt/.filetool.lst
```

1. Add `/opt` to persistence:

```shell
echo "/opt" | sudo tee -a /opt/.filetool.lst
```

1. Save the data:

```shell
filetool.sh -b
```

------

### **3.4 Install `IPv6`, `iproute2` and `tcpdump`**

1. Install `iproute2`:

```shell
tce-load -wi iproute2
```

2. Install `tcpdump`:

```shell
tce-load -wi tcpdump
```

3. Install IPv6:

```shell
tce-load -wi ipv6-3.16.6-tinycore
sudo modprobe ipv6
```

4. IPv6 verification

Finally, save everything with:

```shell
filetool.sh -b 
```