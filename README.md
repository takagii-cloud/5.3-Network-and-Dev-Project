# Integrative Projects – Semester 5

## 1. Educational Objectives

The integrative projects aim to:

- Familiarize students with the main tools used in the program, including VMware, VirtualBox, Git, development tools, and system/network administration tools.
- Apply concepts related to software development, operating systems, and networking acquired during semester 5 and previous studies.
- Develop autonomy and time management skills.
---

## 2. General Constraints

- Virtual machines must be built using **VMware** or **VirtualBox**.
- All software development must be done using **C**, with **socket programming** for networking.
- A **Git version manager** must be used.
- Projects are **individual** (no group work).

---

## 3. Mini-Projects

### 3.1 VyOS: Virtual Router

- Create a virtual machine running **VyOS 1.4.3**, fully installed on a virtual disk.
- Automatically set the **AZERTY keyboard layout**.
- Automatically rename network interfaces for cloned VMs:
  - 1 interface: `eth0`
  - 2 interfaces: `eth0`, `eth1`
  - etc.

---

### 3.2 Alpine: Lightweight Graphical Terminal

- Create a virtual machine running **Alpine Linux 3.22.2**.
- Install and configure **GRUB** allowing boot into console or graphical mode.
- Integrate at minimum:
  - OpenSSH, Filezilla, Web browser, tcpdump, wireshark, putty.
- Install **VMware Tools** to support mouse integration and automatic screen resolution.
- Optimize the system to reduce:
  - Disk usage (`df`)
  - Memory usage (`free`)

---

### 3.3 MicroCore: Lightweight Console System

- Create a MicroCore virtual machine (**TinyCore Linux 6.2**).
- Automatically set the **AZERTY keyboard layout**.
- Make `/home` and `/opt` persistent.
- Integrate at minimum:
  - IPv6
  - Linux `ip` command
  - tcpdump

---

### 3.4 ifshow: Local Network Interface Command

- Develop **ifshow** in C under Linux.

**Features:**

- `ifshow -i ifname`: display IPv4/IPv6 prefixes for a given interface.
- `ifshow -a`: list all local network interfaces with associated IPv4/IPv6 prefixes.

**Integration:**

- VyOS VM  
- Alpine VM  
- MicroCore VM

---

### 3.5 NetLab: Virtual Network

- Build a virtual network using all mini-projects (VyOS routers and Alpine/MicroCore terminals).
- Configure IPv4 routing for full communication between machines.
- Configure IPv6 routing with automatic address assignment on terminals.
- Network addresses are defined by a personal identifier **x** (given during class).

---

### 3.6 ifnetshow: Remote Interface Inspection

- Develop **ifnetshow** and its server agent using C socket programming.

**Features:**

- `ifnetshow -n addr -i ifname`: display prefixes of a remote interface.
- `ifnetshow -n addr -a`: display all interfaces and prefixes on a remote machine.

**Additional requirements:**

- The agent should be **persistent** if possible.
- Reuse code from `ifshow`.
- Integrate into **VyOS, Alpine, and MicroCore** systems.

---

### 3.7 neighborshow: Neighbor Discovery Command

- Develop **neighborshow** in C.
- Display all neighboring machines on the same physical network, without prior knowledge of their addresses.
- Requires an agent running on remote machines.
- System names must be retrieved dynamically using the Linux `hostname` command.
- Integrate into **VyOS, Alpine, and MicroCore**.

**Extension:**

- `neighborshow -hop n`: show reachable systems in *n* hops.

