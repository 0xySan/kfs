NAME	= kfs
CC		= gcc
AS		= nasm

SRCDIR	= src
INCDIR	= includes
LDSCRIPT = $(SRCDIR)/linker.ld

CFLAGS	= -m32 -std=gnu99 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra -I$(INCDIR)
LDFLAGS	= -m elf_i386
ASFLAGS	= -f elf32
# GRUBFLAGS = --compress=gz

SRCS_C	= $(SRCDIR)/kernel.c $(SRCDIR)/terminal.c $(SRCDIR)/keyboard.c $(SRCDIR)/printk.c $(SRCDIR)/bash.c $(SRCDIR)/helpers.c $(SRCDIR)/pfa.c
SRCS_S	= $(SRCDIR)/boot.s
OBJDIR	= obj
OBJS	= $(addprefix $(OBJDIR)/,$(SRCS_S:.s=.o) $(SRCS_C:.c=.o))
DEPS	= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	@echo -e "\tLD $@"
	@ld -T $(LDSCRIPT) -o $(NAME) $(LDFLAGS) $(OBJS)
	@echo "Compilation finished. Output: $(NAME)"

$(OBJDIR)/%.o: %.c
	@echo -e "\tCC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR)/%.o: %.s
	@echo -e "\tAS $<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

-include $(DEPS)

clean:
	@rm -rf $(OBJDIR)
	@rm -f *.iso

fclean: clean
	@echo "Cleaning executable..."
	@rm -f $(NAME)
	@echo "Executable removed."
	@echo "Cleaning ISO directory..."
	@rm -rf iso
	@echo "ISO directory removed."

re:
	@$(MAKE) fclean
	@$(MAKE) all

iso: re
	@echo "Creating ISO image..."
	@mkdir -p iso/boot/grub
	@cp $(NAME) iso/boot/
	@printf 'set timeout=5\nset default=0\n\nmenuentry "etaquet $(NAME)" {\n\tmultiboot /boot/$(NAME)\n\tboot\n}\n' > iso/boot/grub/grub.cfg
	@grub-mkrescue $(GRUBFLAGS) -o $(NAME).iso iso
	@echo "ISO image created: $(NAME).iso"

run: iso
	@echo "Launching QEMU..."
	@qemu-system-i386 -cdrom $(NAME).iso

dev: iso
	@qemu-system-i386 -drive format=raw,file=$(NAME).iso -s -S

.PHONY: all re clean fclean iso run dev
