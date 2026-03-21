NAME	= kfs
CC	= i686-elf-gcc
AS	= i686-elf-as

SRCDIR	= src
INCDIR	= includes
LDSCRIPT = $(SRCDIR)/linker.ld

CPPFLAGS = -I$(INCDIR)
CFLAGS	= -std=gnu99 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra
LDFLAGS	= -ffreestanding -nostdlib -nodefaultlibs -lgcc

SRCS_C	= $(SRCDIR)/kernel.c $(SRCDIR)/terminal.c $(SRCDIR)/keyboard.c $(SRCDIR)/printk.c $(SRCDIR)/bash.c $(SRCDIR)/helpers.c
SRCS_S	= $(SRCDIR)/boot.s
OBJDIR	= obj
OBJS	= $(addprefix $(OBJDIR)/,$(SRCS_S:.s=.o) $(SRCS_C:.c=.o))
DEPS	= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CC) -T $(LDSCRIPT) -o $(NAME) $(LDFLAGS) $(OBJS)
	@echo "Compilation finished. Output: $(NAME)"

$(OBJDIR)/%.o: %.c
	@echo "\tCC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR)/%.o: %.s
	@echo "\tAS $<"
	@mkdir -p $(dir $@)
	@$(AS) $< -o $@

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

re: fclean all

iso: re
	@echo "Creating ISO image..."
	@mkdir -p iso/boot/grub
	@cp $(NAME) iso/boot/
	@printf 'set timeout=5\nset default=0\n\nmenuentry "etaquet $(NAME)" {\n\tmultiboot /boot/$(NAME)\n\tboot\n}\n' > iso/boot/grub/grub.cfg
	@grub-mkrescue -o $(NAME).iso iso
	@echo "ISO image created: $(NAME).iso"

launch: iso
	@echo "Launching QEMU..."
	@qemu-system-i386 -cdrom $(NAME).iso

.PHONY: all re clean fclean iso launch
