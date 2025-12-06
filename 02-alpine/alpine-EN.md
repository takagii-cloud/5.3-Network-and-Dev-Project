# **Alpine: the graphical terminal**

This documentation aims to learn how to configure Alpine Linux in graphical mode.

------

### **Prerequisites**

- Alpine Linux ISO: `alpine_virt_3.22.2`
- Oracle VirtualBox in its latest version.

------

## **2. Alpine**

Alpine is a very lightweight graphical distribution. This lightweight footprint makes it possible to run several Alpine virtual machines simultaneously on a laptop.

We will learn how to install Alpine Linux with a graphical interface.

------

### **2.1 Creating the virtual machine**

Let’s create a virtual machine with Alpine Linux.

In VirtualBox:

1. New
2. Specify a virtual machine name: `AlpineLinux-og`
3. Select the Alpine Linux ISO
4. Choose hardware specifications

We keep the default specifications for our first virtual machine.

1. Choose virtual disk specifications, then confirm.

Our machine is now ready for its first boot.

------

### **2.2 Installing and configuring GRUB**

To allow choosing whether to boot in graphical mode (`GUI`) or console mode (`CLI`), we need to install GRUB.

------

#### **Installation**

Start the machine and log in: `localhost login: root`.
 Then launch the installation script with the simple command:

```shell
setup-alpine
```

- Choose a keyboard layout: `fr`
- Choose a variant: `fr-azerty`
- Choose a hostname: `alpine`
- Choose the interface to configure: `eth0`
- Configuration method: `dhcp`
- Manual network config: `n`
- Change root password: `taka`
- Timezone: `Europe/Paris`
- Proxy: `none`
- NTP: `busybox`
- APK Mirror: `(s)` then `1` for `dl-cdn.alpinelinux.org`
- User: `Adam`
  - username: `ataha`
  - password: `taki`
- SSH Server: `openssh`
- Disk & Install: `sda`
  - How would you like to use it: `sys`
- Erase the above disk(s) and continue (y/n): `y`

You can now shut down the virtual machine and eject the installation disk.

------

#### **System update**

```shell
apk update
apk upgrade
```

Enable community repositories by removing `#` in the file `/etc/apk/repositories`:
 `vi /etc/apk/repositories`

```shell
https://dl-cdn.alpinelinux.org/alpine/v3.22/main
https://dl-cdn.alpinelinux.org/alpine/v3.22/community
```

------

#### **Installing the GRUB bootloader**

1. Install GRUB:

```bash
apk add grub grub-bios
```

1. Install GRUB on the disk:

```bash
grub-install /dev/sda
```

1. Retrieve UUID and PARTUUID:

```shell
awk '$2=="/"{print $1}' /proc/mounts
```

`blkid -s UUID -o value /dev/sda1` : `1c71a0a4-069a-4d89-88a1-3874cf48240c` `6c0a2c39-3f59-4e03-86c0-01cb25951b11`

`blkid -s UUID -o value /dev/sda3` : `fb64b00f-e04c-4d56-9601-2cabdfd4e29d` `470b6fc3-8bb0-4766-96d5-e21396115f09`

`blkid -s PARTUUID -o value /dev/sda3` : `23b7e723-03` `2667a258-03`

------

#### **Adding entries in GRUB**

1. Modify the file: `vi /etc/grub.d/40_custom`

```bash
#!/bin/sh
exec tail -n +3 $0
# Custom GRUB entries

menuentry 'Alpine Linux (GUI)' {
    # Partition /boot = /dev/sda1
    search --no-floppy --fs-uuid --set=root 1c71a0a4-069a-4d89-88a1-3874cf48240c
    echo 'Graphical Mode...'
    linux /vmlinuz-virt root=UUID=fb64b00f-e04c-4d56-9601-2cabdfd4e29d ro modules=sd-mod,usb-storage,ext4 quiet rootfstype=ext4 softlevel=graphical
    echo 'Loading initial ramdisk...'
    initrd /initramfs-virt
}
```

1. Regenerate GRUB:

```shell
grub-mkconfig -o /boot/grub/grub.cfg
```

------

#### **Installing the graphical environment**

1. Interactive tool: `setup-desktop`

Choose `xfce`

------

#### **Creation of the GUI runlevel**

We don’t want LightDM and the graphical mode to start automatically in the "default" mode.

We create a separate runlevel named `graphical`, which will only be used if booted in graphical mode via GRUB.

```shell
mkdir -p /etc/runlevels/graphical
rc-update add -s default graphical
rc-update add lightdm graphical
rc-update add dbus default      
rc-update del lightdm default
```

------

#### **Replacing Dropbear with OpenSSH (optional)**

```shell
rc-update del dropbear
rc-service dropbear stop
apk del dropbear
apk add openssh
ssh-keygen -A
rc-update add sshd default
rc-service sshd start
```

------

### **2.3 Adding software tools**

We want to add software and tools necessary for the system and network analysis.

------

#### **Installing common tools**

```bash
apk add firefox-esr
apk add filezilla
apk add tcpdump
apk add wireshark
apk add putty
```

Add the user to the Wireshark group:
 `adduser ataha wireshark`

------

#### **Machine configuration**

1. Set keyboard to French: from the graphical interface.
2. Install `zsh`

```bash
apk update
apk add zsh shadow
```

1. Set `zsh` as the default shell for root:
    `chsh` → `/bin/zsh`
2. Set `zsh` as default shell for user:
    `chsh ataha` → `/bin/zsh`
3. Create `.zshrc` configuration: `vi /root/.zshrc`

```bash
# ----- Prompt -------------
PROMPT='%F{green}┌──(%B%F{blue}%n%F{white}@%F{blue}%m%b%F{green})-[%B%F{reset}%(6~.%-1~/…/%4~.%5~)%b%F{green}]
└─%B%(#.%F{red}#.%F{blue}$)%b%F{reset} '

# ----- Useful aliases -----
alias ll="ls -alF"
alias la="ls -A"
alias l="ls -CF"
```

1. Copy configuration for the user:

```shell
cp /root/.zshrc /home/ataha/.zshrc
chown ataha:ataha /home/ataha/.zshrc
```

------

### **2.4 Installing VirtualBox services and modules**

Let’s install the VirtualBox Guest Additions.

------

#### **VirtualBox Additions**

1. Install Guest Additions:

```shell
apk add virtualbox-guest-additions virtualbox-guest-additions-x11
```

1. Add Guest Additions to startup:

```shell
rc-update add virtualbox-guest-additions boot
rc-update virtualbox add virtualbox-drm boot
```

1. In VirtualBox:
    `General --> Advanced : Shared Clipboard & Drag'n'Drop : Bidirectional`

------

That’s it! All that remains is optimizing memory and disk size.