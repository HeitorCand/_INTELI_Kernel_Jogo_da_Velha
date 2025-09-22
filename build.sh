echo "Construindo o Jogo da Forca Kernel..."

# Limpar builds anteriores
rm -f kasm.o kc.o kernel

# Compilar arquivo assembly
echo "Compilando kernel.asm..."
nasm -f elf32 kernel.asm -o kasm.o

# Compilar arquivo C usando cross-compiler
echo "Compilando kernel.c..."
x86_64-elf-gcc -m32 -c kernel.c -o kc.o -ffreestanding -fno-stack-protector

# Linkar arquivos objeto
echo "Linkando kernel..."
x86_64-elf-ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o

echo "Build concluído! Kernel criado."
echo "Para rodar o jogo: qemu-system-i386 -kernel kernel"