TARGET := i686-elf
CC := $(TARGET)-gcc
AS := $(TARGET)-as
LD := $(TARGET)-ld
GRUB_MKRESCUE := grub-mkrescue
XORRISO := xorriso

CFLAGS := -std=c11 -ffreestanding -O2 -Wall -Wextra -Werror \
           -m32 -march=i686 -fno-pie -fno-stack-protector \
           -fno-builtin -nostdinc
LDFLAGS := -m elf_i386 -T linker.ld

BUILD := build
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/sooome-cooooool-os.iso

.PHONY: all clean run check

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/OS.o: OS.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/boot.o: boot.S | $(BUILD)
	$(AS) --32 $< -o $@

$(KERNEL): $(BUILD)/boot.o $(BUILD)/OS.o linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/boot.o $(BUILD)/OS.o
	grub-file --is-x86-multiboot2 $@

$(ISO): $(KERNEL) grub.cfg
	rm -rf $(BUILD)/isoe
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(BUILD)/iso

check: $(KERNEL)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M

clean:
	rm -rf $(BUILD)
