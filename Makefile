NAME	= kfs
CC	= i686-elf-gcc
AS	= i686-elf-as

CFLAGS	= -std=gnu99 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Wall -Wextra
LDFLAGS	= -ffreestanding -nostdlib -nodefaultlibs -lgcc

SRCS_C	= kernel.c terminal.c keyboard.c printk.c
SRCS_S	= boot.s
OBJS	= $(SRCS_S:.s=.o) $(SRCS_C:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CC) -T linker.ld -o $(NAME) $(LDFLAGS) $(OBJS)
	@echo "Compilation finished. Output: $(NAME)"

%.o: %.c
	@echo "Compiling $<..."
	@$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.s
	@echo "Assembling $<..."
	@$(AS) $< -o $@

clean:
	@echo "Cleaning object files..."
	@rm -f $(OBJS)
	@rm -f *.iso
	@echo "Object files removed."

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
	@printf 'set timeout=5\nset default=0\n\nmenuentry "$(NAME)" {\n\tmultiboot /boot/$(NAME)\n\tboot\n}\n' > iso/boot/grub/grub.cfg
	@grub-mkrescue -o $(NAME).iso iso
	@echo "ISO image created: $(NAME).iso"

launch: iso
	@echo "Launching QEMU..."
	@qemu-system-i386 -cdrom $(NAME).iso

.PHONY: all re clean fclean iso launch
