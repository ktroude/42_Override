# OverRide

`OverRide` is a 42 security project focused on binary exploitation of ELF executables in an i386 Linux environment.
The project is the continuation of `RainFall` and is designed to improve practical understanding of memory, insecure programming patterns, and binary exploitation.

> This repository is for the official 42 OverRide VM and for authorised educational work only.

---

## Table of contents

- [Project goal](#project-goal)
- [Repository status](#repository-status)
- [Important submission warning](#important-submission-warning)
- [Repository structure](#repository-structure)
- [File convention](#file-convention)
- [Environment](#environment)
- [VM install from Override.iso and access method from host machine](#VM-install-from-Override.iso-and-access-method-from-host-machine)
- [SSH forwarding with libvirt/QEMU](#ssh-forwarding-with-libvirtqemu)
- [Fix: ISO not detected after reboot](#fix-iso-not-detected-after-reboot)
- [Tools used](#tools-used)
- [Level summaries](#level-summaries)
- [How to review a level during evaluation](#how-to-review-a-level-during-evaluation)
- [Safety and scope](#safety-and-scope)
- [Final checklist](#final-checklist)

---

## Project goal

The goal of `OverRide` is to analyse each vulnerable binary, understand the vulnerability, exploit it in the official VM, and retrieve the `.pass` file of the next user.

The project trains practical knowledge of:

- ELF binary analysis;
- reverse engineering of compiled programs;
- i386 memory layout;
- stack behaviour;
- buffer overflows;
- format-string vulnerabilities;
- shellcode behaviour in a controlled lab;
- debugging with `gdb`;
- explaining an exploit clearly during peer evaluation.

---

## Repository status

This repository currently documents the following levels:

| Level | Status in this repository | Main idea |
|---|---:|---|
| `level00` | Done | Static analysis / hardcoded password |
| `level01` | Done | Stack buffer overflow and return address overwrite |
| `level02` | Done | Format-string stack leak |
| `level03` | Done | Reverse engineering of XOR-based password check |
| `level04` | Done | `gets()` overflow with shellcode adapted to the `ptrace` restriction |
| `level05` | Not documented here | Required for the mandatory part |
| `level06` | Not documented here | Required for the mandatory part |
| `level07` | Not documented here | Required for the mandatory part |
| `level08` | Not documented here | Required for the mandatory part |
| `level09` | Not documented here | Bonus level |

---

## Important submission warning

The folder `BinaryfromISO/` is useful for local analysis because it contains copied binaries from the OverRide VM.

However, the official subject says that the final submitted repository must not contain binaries. Everything present in the repository must be explainable during peer evaluation.

Before final submission, remove this folder if it is still present:

```text
BinaryfromISO/
```

A clean evaluated repository should normally keep only the solved level directories, each with this structure:

```text
levelXX/
├── flag
├── source
└── Ressources/
```

---

## Repository structure

Current archive structure:

```text
.
├── BinaryfromISO/
│   ├── Flag_of_level00-level05
│   ├── level00
│   ├── level01
│   ├── level02
│   ├── level03
│   └── level04
├── level00/
│   ├── flag
│   ├── source
│   └── Ressources/
│       └── notes.md
├── level01/
│   ├── flag
│   ├── source
│   └── Ressources/
│       ├── level01_python2_VM.py
│       ├── level01_python3_Hostmachine.py
│       └── notes.md
├── level02/
│   ├── flag
│   ├── source
│   └── Ressources/
│       ├── level02.py
│       └── notes.md
├── level03/
│   ├── flag
│   ├── source
│   └── Ressources/
│       └── notes.md
├── level04/
│   ├── flag
│   ├── source
│   └── Ressources/
│       ├── level04.py
│       └── notes.md
└── README.md
```

---

## File convention

Each solved level follows the expected OverRide structure.

### `flag`

Contains the password or proof obtained for the current solved level.

### `source`

Contains a readable C reconstruction of the exploited binary.
The goal is not to reproduce the original source perfectly, but to show the important logic and the vulnerability in a form that can be explained clearly.

### `Ressources/`

Contains notes, helper scripts, commands, analysis steps, and proof material used to justify the resolution during peer evaluation.

---

## Environment

The project is designed to be solved inside the official OverRide VM.

The subject indicates that the first login is:

```text
login:    level00
password: level00
```

The official VM exposes SSH on port `4242`.

Example from the subject:

```bash
ssh level00@<vm_ip> -p 4242
```

In this repository and in the notes, the local setup often uses host port forwarding:

```bash
ssh level00@127.0.0.1 -p 7777
```

This means:

```text
Host 127.0.0.1:7777  ->  VM port 4242
```

---

## VM install from Override.iso and access method from host machine

There are two steps to access the VM from the host machine.

### Step I - Install and boot the VM from `OverRide.iso`

Use this method when the OverRide VM has not been created yet.
The ISO is already the bootable project system, so the goal is mainly to create a VM that boots from `OverRide.iso`.

Example local files used in this setup:

```text
/home/thitran/sgoinfre/OverRide.iso
/home/thitran/sgoinfre/Overide.qcow2
```

> Linux filenames are case-sensitive. Keep the exact spelling `OverRide.iso` and `Overide.qcow2` if you use these paths.

#### Step 1 — Open Virtual Machine Manager

1. Open **Virtual Machine Manager**.
2. Click **Create a new virtual machine**.
3. Choose **Local install media (ISO image or CDROM)**.
4. Click **Forward**.

#### Step 2 — Select the OverRide ISO

1. Click **Browse**.
2. Select the ISO file:

```text
/home/thitran/sgoinfre/OverRide.iso
```

3. If Virt-Manager cannot detect the OS automatically, choose a generic Linux profile.
4. Click **Forward**.

#### Step 3 — Configure CPU and memory

A small configuration is enough for OverRide:

```text
Memory: 1024 MB or 2048 MB
CPU:    1 or 2 cores
```

Then click **Forward**.

#### Step 4 — Create the virtual disk

Create a small writable disk for the VM, for example:

```text
/home/thitran/sgoinfre/Overide.qcow2
Size: 5 GB
```

Then click **Forward**.

#### Step 5 — Name the VM

Choose a clear VM name, for example:

```text
ubuntu25.04
```

or:

```text
OverRide
```

If available, enable **Customize configuration before install**, then click **Finish**.

#### Step 6 — Check the boot order

Before starting the VM, open **Boot Options** and make sure:

1. **CD-ROM** is enabled.
2. **Hard Disk** is enabled.
3. **CD-ROM** is above **Hard Disk**.

The CD-ROM source must point to:

```text
/home/thitran/sgoinfre/OverRide.iso
```

This is important because the VM must boot from the OverRide ISO.

#### Step 7 — Start the VM

Click **Start**, then open the graphical console.

If everything is correct, the VM should boot to the OverRide login screen.
The first login is:

```text
login:    level00
password: level00
```

The graphical console is enough to verify that the VM boots correctly. For daily work, SSH is usually more comfortable; see Method 2 below.

### Step II — Connect from the host terminal using SSH

This is more comfortable for working with files, scripts, `gdb`, and copy/paste.

With the local setup used here, the final target is:

```text
Host 127.0.0.1:7777  ->  Guest port 4242
```

Then the connection command is:

```bash
ssh level00@127.0.0.1 -p 7777
```

For a normal VM IP without host forwarding, use:

```bash
ssh level00@<vm_ip> -p 4242
```

---

## SSH forwarding with libvirt/QEMU

This section documents the local setup where:

- libvirt version is `8.0.0`;
- the VM name is `ubuntu25.04`;
- the VM uses `<interface type="user">`;
- the newer `<backend type="passt">` method is unavailable;
- QEMU user-mode networking is used with `hostfwd`.

### Step 1 — Shut down the VM

The VM must be fully stopped and show:

```text
Shutoff
```

Do not edit the network configuration while the VM is running.

### Step 2 — Open the complete XML

In Virtual Machine Manager:

1. Open `ubuntu25.04`.
2. Click **Show virtual hardware details**.
3. Select **Overview**.
4. Open the **XML** tab.

Edit the complete domain XML, not only the network-device XML.

### Step 3 — Add the QEMU namespace

Change the first line from:

```xml
<domain type="kvm">
```

to:

```xml
<domain type="kvm"
        xmlns:qemu="http://libvirt.org/schemas/domain/qemu/1.0">
```

This namespace allows QEMU-specific command-line arguments in the domain XML.

### Step 4 — Remove the existing user interface block

Remove the existing block similar to this one:

```xml
<interface type="user">
  <mac address="52:54:00:b4:48:6f"/>
  <model type="virtio"/>
  <address type="pci"
           domain="0x0000"
           bus="0x01"
           slot="0x00"
           function="0x0"/>
</interface>
```

Do not keep both the old interface block and the replacement QEMU network configuration.

### Step 5 — Add the forwarding configuration

Add this block after `</devices>` and immediately before `</domain>`:

```xml
<qemu:commandline>
  <qemu:arg value="-netdev"/>
  <qemu:arg value="user,id=net0,hostfwd=tcp:127.0.0.1:7777-:4242"/>

  <qemu:arg value="-device"/>
  <qemu:arg value="virtio-net-pci,netdev=net0,mac=52:54:00:b4:48:6f"/>
</qemu:commandline>
```

The end of the XML should look like this:

```xml
    <rng model="virtio">
      <backend model="random">/dev/urandom</backend>
      <address type="pci"
               domain="0x0000"
               bus="0x06"
               slot="0x00"
               function="0x0"/>
    </rng>
  </devices>

  <qemu:commandline>
    <qemu:arg value="-netdev"/>
    <qemu:arg value="user,id=net0,hostfwd=tcp:127.0.0.1:7777-:4242"/>

    <qemu:arg value="-device"/>
    <qemu:arg value="virtio-net-pci,netdev=net0,mac=52:54:00:b4:48:6f"/>
  </qemu:commandline>
</domain>
```

Then click **Apply**.

The forwarding rule means:

```text
tcp:127.0.0.1:7777-:4242
    └──── host ────┘  └ guest port
```

### Step 6 — Check SSH inside the VM

For the official OverRide VM, SSH is expected on port `4242`.

For a custom Ubuntu VM, install and start SSH if needed:

```bash
sudo apt update
sudo apt install openssh-server
sudo systemctl enable --now ssh
```

Check the listening port:

```bash
sudo ss -lntp | grep ':4242'
```

Expected kind of output:

```text
LISTEN 0 128 0.0.0.0:4242
```

Find the current VM username if needed:

```bash
whoami
```

### Step 7 — Connect from the host

From the host machine:

```bash
ssh -p 7777 VM_USERNAME@127.0.0.1
```

For the OverRide first level, the usual command is:

```bash
ssh level00@127.0.0.1 -p 7777
```

---

## Fix: ISO not detected after reboot

This section documents a common local VM problem.

Example local files:

```text
/home/thitran/sgoinfre/OverRide.iso
/home/thitran/sgoinfre/Overide.qcow2
```

The first run may work because the VM boots directly from:

```text
/home/thitran/sgoinfre/OverRide.iso
```

On the second run, the CD-ROM may become empty:

```text
file   cdrom   sda   -
```

In that case, SeaBIOS tries the empty `Overide.qcow2` disk and reports that it is not a bootable disk.

### Step 1 — Stop the VM

If the VM is stuck at the BIOS error screen:

```bash
virsh -c qemu:///session destroy ubuntu25.04
```

### Step 2 — Insert the ISO persistently

Run:

```bash
virsh -c qemu:///session change-media \
  ubuntu25.04 \
  sda \
  /home/thitran/sgoinfre/OverRide.iso \
  --insert \
  --config
```

The important option is:

```text
--config
```

It updates the persistent VM configuration, so the ISO remains attached after shutting down and starting the VM again.

Verify the block devices:

```bash
virsh -c qemu:///session domblklist ubuntu25.04 --details
```

Expected result:

```text
Type   Device   Target   Source
---------------------------------------------------------------
file   disk     vda      /home/thitran/sgoinfre/Overide.qcow2
file   cdrom    sda      /home/thitran/sgoinfre/OverRide.iso
```

Linux filenames are case-sensitive, so keep the exact spelling:

```text
OverRide.iso
```

### Step 3 — Put the CD-ROM first in the boot order

In Virtual Machine Manager:

1. Open `ubuntu25.04`.
2. Click **Show virtual hardware details**.
3. Open **Boot Options**.
4. Enable both **CD-ROM** and **Hard Disk**.
5. Move **CD-ROM** above **Hard Disk**.
6. Click **Apply**.

Also select the CD-ROM device in the hardware list and confirm that its source is:

```text
/home/thitran/sgoinfre/OverRide.iso
```

### Step 4 — Start the VM

Start the VM:

```bash
virsh -c qemu:///session start ubuntu25.04
```

Then open the console in Virt-Manager, or run:

```bash
virt-viewer -c qemu:///session ubuntu25.04
```

The VM should boot back to the OverRide login screen.

### ISO versus QCOW2

| File | Purpose |
|---|---|
| `OverRide.iso` | Bootable OverRide system |
| `Overide.qcow2` | Empty writable virtual hard disk |

---

## Tools used

The notes and resources mention the following tools and methods:

- `ssh` to connect to the VM;
- `scp` to copy binaries from the VM for local analysis;
- `gdb` to inspect crashes and registers;
- `checksec`-style security checks;
- decompilation / binary analysis to reconstruct readable C code;
- Python 2 scripts inside the VM when Python 3 is unavailable;
- Python 3 helper scripts on the host machine.

---

## Level summaries

### level00 — hardcoded password

`level00/source` shows a simple authentication program.
The binary asks for an integer password, compares it with a constant value, and starts a shell when the comparison succeeds.

**Core weakness:** the expected password can be recovered by static analysis of the binary.

**Repository files:**

```text
level00/
├── flag
├── source
└── Ressources/
    └── notes.md
```

---

### level01 — stack buffer overflow

`level01/source` reconstructs an admin login program.
The username is checked against an expected value, then the password is read into a local buffer with an unsafe size.

**Core weakness:** a password read writes more bytes than the local stack buffer can safely hold, allowing control of the saved return address.

**Exploit idea:**

1. provide the valid username;
2. place controlled bytes in a predictable buffer;
3. overflow the password buffer;
4. overwrite the return address;
5. redirect execution to the controlled area.

**Important resource files:**

```text
level01/Ressources/level01_python2_VM.py
level01/Ressources/level01_python3_Hostmachine.py
level01/Ressources/notes.md
```

---

### level02 — format-string leak

`level02/source` reconstructs a login program that reads the next password from `/home/users/level03/.pass`.
If authentication fails, the program prints the username directly as a format string.

**Core weakness:**

```c
printf(username);
```

The safer form would be:

```c
printf("%s", username);
```

**Exploit idea:**

1. use the username field as a format string;
2. leak stack values with positional format specifiers;
3. identify the leaked chunks containing the next password;
4. reverse each leaked 64-bit chunk because the binary is little-endian;
5. reconstruct the password for the next level.

**Important resource file:**

```text
level02/Ressources/level02.py
```

---

### level03 — XOR reverse engineering

`level03/source` reconstructs a program that transforms an encrypted string with XOR and compares the result with an expected success string.

**Core weakness:** the required input can be calculated from the constant used by the program and the XOR key needed to decrypt the string.

**Exploit idea:**

1. inspect the encrypted string;
2. recover the XOR key that turns it into the expected text;
3. compute the correct numeric input;
4. run the binary with that value to obtain the next effective user.

**Repository files:**

```text
level03/
├── flag
├── source
└── Ressources/
    └── notes.md
```

---

### level04 — `gets()` overflow under `ptrace`

`level04/source` reconstructs a parent/child program.
The child reads input with `gets()`, while the parent traces the child and kills it if it tries to call `execve`.

**Core weakness:**

```c
gets(buf);
```

The buffer has a fixed size, but `gets()` has no length limit.

**Exploit idea:**

1. overflow the child stack buffer;
2. overwrite the saved return address;
3. jump into controlled code;
4. avoid `execve`, because the parent process blocks it;
5. read the next `.pass` file using direct syscalls.

**Important resource file:**

```text
level04/Ressources/level04.py
```

---

## How to review a level during evaluation

For each level, be ready to explain:

1. what the vulnerable program does;
2. where the vulnerability is;
3. why the exploit works;
4. how the offset, address, key, or leaked value was found;
5. how the next `.pass` file was read;
6. why the solution is not brute force.

Example review path:

```bash
cd levelXX
cat source
cat Ressources/notes.md
```

Then explain the important lines from `source` and the logic of the exploit from `Ressources/notes.md`.

---

## Safety and scope

This repository is only for the official 42 OverRide VM and for authorised educational work.
Do not use these techniques on systems, programs, accounts, or networks that you do not own or do not have explicit permission to test.

---

## Final checklist

Before submitting or defending the project:

- [ ] Verify that every documented level has `flag`, `source`, and `Ressources/`.
- [ ] Verify that every file can be explained clearly.
- [ ] Remove copied binaries from the final evaluated repository if required.
- [ ] Complete the mandatory levels `level00` through `level08`.
- [ ] Keep `level09` only for the bonus part.
- [ ] Make sure every group member can justify every solved challenge.
- [ ] Check that local VM notes are clearly separated from mandatory project files.
