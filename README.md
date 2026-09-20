# sooome cooooool os

A tiny graphical x86 operating-system starter written in freestanding C.
It currently boots through GRUB2 and draws a framebuffer desktop. The same ISO
can run in QEMU, VirtualBox, or on a legacy-BIOS physical x86 machine.

## What works

- Multiboot2 boot protocol
- 32-bit x86 kernel entry point and stack
- GRUB-provided 32-bit RGB framebuffer
- A simple desktop, window, taskbar, labels, and cursor
- No libc, filesystem, or host OS dependency at runtime

This is an early OS foundation, not yet a complete daily-use operating system.
Keyboard, mouse, processes, storage, networking, and UEFI boot support are
natural next milestones.

## Build on Arch Linux

Install the build and emulator tools:

```sh
sudo pacman -S base-devel grub xorriso qemu-system-i386
```

The Makefile expects an `i686-elf-gcc` cross compiler. A cross compiler avoids
accidentally linking against the host Linux runtime. Build one using the
OSDev toolchain instructions, then make sure `i686-elf-gcc`, `i686-elf-as`,
and `i686-elf-ld` are on `PATH`.

Build and run:

```sh
make
make run
```

The ISO is written to `build/sooome-cooooool-os.iso`.

## Build online with GitHub

You can build the ISO without installing Linux or any compiler on Windows.
Upload this folder to a GitHub repository, then:

1. Open the repository on GitHub.
2. Select the **Actions** tab.
3. Select **Build bootable ISO** in the left sidebar.
4. Select **Run workflow**, choose the `main` branch, and run it.
5. Open the completed workflow run and download the
   `sooome-cooooool-os` artifact from the **Artifacts** section.

The workflow is stored in
`.github/workflows/build-iso.yml`. It uses GitHub's temporary Ubuntu runner,
then deletes the runner after the build. The ISO is not written to your local
disk unless you choose to download it.

To upload the folder using GitHub's website, create a new repository, choose
**Add file > Upload files**, drag in the project files including the `.github`
folder, and commit them to the `main` branch.

## Run in VirtualBox

1. Run `make` on an Arch Linux environment (a native Arch install, WSL2 with
   the required Linux tools, or a Linux VM).
2. Create a new VirtualBox VM:
   - Type: Other
   - Version: Other/Unknown (32-bit)
   - Memory: 128 MB or more
   - Firmware: BIOS, not EFI
3. Attach `build/sooome-cooooool-os.iso` as the optical disk.
4. Start the VM. Enable 3D acceleration only if the VM has display issues
   without it; the kernel itself only needs the GRUB framebuffer.

## Real hardware warning

Only test on hardware you can safely re-image. Use a spare USB drive and write
the ISO with a tool such as `dd` or an imaging utility. Do not overwrite the
disk containing your existing operating system. Current boot support targets
legacy BIOS through GRUB; UEFI support will need a separate EFI loader.
