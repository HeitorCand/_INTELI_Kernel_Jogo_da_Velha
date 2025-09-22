Jogo da Forca no Kernel
===============================

Este é um micro kernel simples que implementa um jogo interativo da Forca. O kernel inicializa diretamente no jogo com complexidade mínima de código.

## Video
![Video Kernel](https://drive.google.com/file/d/1O6fkfyYf2OtCu8uRt2BiFsXEnBdv4z_5/view?usp=drive_link)


## Funcionalidades

- **Bootloader Simples**: Bootloader em Assembly que inicializa o kernel
- **Display Básico**: Display em modo texto com posicionamento simples  
- **Entrada do Teclado**: Tratamento essencial de interrupções do teclado
- **Jogo da Forca**: Implementação simplificada da forca com:
  - 5 palavras simples para adivinhar (KERNEL, SYSTEM, MEMORY, PROGRAM, COMPUTER)
  - Desenho básico da forca em ASCII
  - Exibição simples da palavra com sublinhados
  - Rastreamento básico de letras
  - Detecção de vitória/derrota
  - Reinício do jogo com a tecla '0'

## Instruções do Jogo

1. **Objetivo**: Adivinhe a palavra oculta digitando letras
2. **Entrada**: Digite qualquer letra (a-z) para fazer um palpite
3. **Chances**: Você tem 6 palpites errados antes de perder
4. **Reiniciar**: Pressione 'R' para começar um novo jogo
5. **Vitória**: Adivinhe todas as letras antes de esgotar as chances

## Implementação Técnica

### Componentes Simplificados
- `kernel.asm`: Bootloader em Assembly 
- `kernel.c`: kernel com lógica básica do jogo
- `keyboard_map.h`: Mapeamento do teclado
- `link.ld`: Script do linker

### Funções Principais
- `init_game()`: Inicializar estado do jogo
- `draw_game()`: Exibir estado atual do jogo
- `process_guess()`: Lidar com palpites de letras
- `keyboard_handler_main()`: Processar entrada do teclado

## Requisitos

### Ambiente de Desenvolvimento
- **NASM**: Netwide Assembler para compilar código assembly
- **Cross Compiler**: x86_64-elf-gcc para geração de código x86 de 32 bits
- **QEMU**: Emulador de sistema i386 para testes

### Instalação no macOS
```bash
# Instalar dependências usando Homebrew
brew install nasm
brew install x86_64-elf-gcc
brew install qemu
```

## Instruções de Compilação

### Opção 1: Usando o script de build (recomendado)
```bash
./build.sh
```

### Opção 2: Compilação manual
```bash
# Compilar bootloader em assembly
nasm -f elf32 kernel.asm -o kasm.o

# Compilar código C do kernel
x86_64-elf-gcc -m32 -c kernel.c -o kc.o -ffreestanding -fno-stack-protector

# Linkar para criar o kernel final
x86_64-elf-ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o
```

## Executando o Jogo

### No Emulador QEMU
```bash
qemu-system-i386 -kernel kernel
```

## Exibição do Jogo

O jogo mostra:
- Título: "HANGMAN GAME"
- Desenho visual da forca (atualiza com palpites errados)
- Palavra atual com letras adivinhadas reveladas
- Contador de palpites errados (X/6)
- Lista de todas as letras tentadas
- Instruções e status do jogo

## Base Original

Baseado no [mkeykernel](http://github.com/arjun024/mkernel) por Arjun Sreedharan, estendido com funcionalidade abrangente de jogo e capacidades de exibição aprimoradas.
