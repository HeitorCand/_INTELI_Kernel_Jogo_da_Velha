#include "keyboard_map.h"

/* Constantes de exibição */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT *COLUMNS_IN_LINE *LINES

/* Constantes de hardware */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

/* Constantes do jogo */
#define MAX_WORD_LENGTH 10
#define MAX_WRONG_GUESSES 6

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* Variáveis globais */
unsigned int current_loc = 0;
char *vidptr = (char *)0xb8000;

/* Estado  do jogo */
char words[5][MAX_WORD_LENGTH] = {"KERNEL", "SYSTEM", "MEMORY", "PROGRAM", "COMPUTER"};
char current_word[MAX_WORD_LENGTH];
char guessed_word[MAX_WORD_LENGTH];
char guessed_letters[26];
int word_length;
int wrong_guesses;
int game_over;
int won;
int word_index = 0;

struct IDT_entry
{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* Configurar interrupção de teclado */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/* Inicializar PIC */
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);
	write_port(0x21, 0x20);
	write_port(0xA1, 0x28);
	write_port(0x21, 0x00);
	write_port(0xA1, 0x00);
	write_port(0x21, 0x01);
	write_port(0xA1, 0x01);
	write_port(0x21, 0xff);
	write_port(0xA1, 0xff);

	/* Carregar IDT */
	idt_address = (unsigned long)IDT;
	idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;
	load_idt(idt_ptr);
}

void kb_init(void)
{
	write_port(0x21, 0xFD); /* Habilitar interrupção de teclado */
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0')
	{
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void kprint_at(const char *str, int row, int col)
{
	current_loc = (row * COLUMNS_IN_LINE + col) * BYTES_FOR_EACH_ELEMENT;
	kprint(str);
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE)
	{
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
	current_loc = 0;
}

/* Funções  de string */
int string_length(const char *str)
{
	int len = 0;
	while (str[len] != '\0')
		len++;
	return len;
}

void string_copy(char *dest, const char *src)
{
	int i = 0;
	while (src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

/* Funções do jogo */
void init_game(void)
{
	int i;

	/* Selecionar palavra */
	string_copy(current_word, words[word_index]);
	word_index = (word_index + 1) % 5;
	word_length = string_length(current_word);

	/* Inicializar palavra adivinhada com sublinhados */
	for (i = 0; i < word_length; i++)
	{
		guessed_word[i] = '_';
	}
	guessed_word[word_length] = '\0';

	/* Limpar letras adivinhadas */
	for (i = 0; i < 26; i++)
	{
		guessed_letters[i] = 0;
	}

	wrong_guesses = 0;
	game_over = 0;
	won = 0;
}

void draw_hangman(void)
{
	kprint_at("+---+", 2, 50);
	kprint_at("|   |", 3, 50);

	if (wrong_guesses >= 1)
		kprint_at("O   |", 4, 50);
	else
		kprint_at("    |", 4, 50);

	if (wrong_guesses >= 3)
		kprint_at("/|\\ |", 5, 50);
	else if (wrong_guesses >= 2)
		kprint_at("/|  |", 5, 50);
	else
		kprint_at("    |", 5, 50);

	if (wrong_guesses >= 5)
		kprint_at("/ \\ |", 6, 50);
	else if (wrong_guesses >= 4)
		kprint_at("/   |", 6, 50);
	else
		kprint_at("    |", 6, 50);

	kprint_at("    |", 7, 50);
	kprint_at("=====", 8, 50);
}

void draw_game(void)
{
	int i;
	char temp[4];

	clear_screen();

	/* Título */
	kprint_at("=== Jogo da Forca ===", 0, 25);

	/* Desenhar forca */
	draw_hangman();

	/* Mostrar palavra */
	kprint_at("Palavra: ", 10, 5);
	current_loc = (10 * COLUMNS_IN_LINE + 15) * BYTES_FOR_EACH_ELEMENT;
	for (i = 0; i < word_length; i++)
	{
		vidptr[current_loc++] = guessed_word[i];
		vidptr[current_loc++] = 0x07;
		vidptr[current_loc++] = ' ';
		vidptr[current_loc++] = 0x07;
	}

	/* Mostrar tentativas erradas */
	kprint_at("Erros: ", 12, 5);
	temp[0] = '0' + wrong_guesses;
	temp[1] = '/';
	temp[2] = '0' + MAX_WRONG_GUESSES;
	temp[3] = '\0';
	kprint(temp);

	/* Mostrar letras adivinhadas */
	kprint_at("Letras: ", 14, 5);
	for (i = 0; i < 26; i++)
	{
		if (guessed_letters[i])
		{
			vidptr[current_loc++] = 'A' + i;
			vidptr[current_loc++] = 0x07;
			vidptr[current_loc++] = ' ';
			vidptr[current_loc++] = 0x07;
		}
	}

	/* Instruções */
	kprint_at("Digite uma letra para adivinhar. Pressione 0 para reiniciar.", 18, 5);

	/* Status do jogo */
	if (game_over)
	{
		if (won)
		{
			kprint_at("*** VOCE VENCEU! ***", 20, 30);
		}
		else
		{
			kprint_at("*** FIM DE JOGO ***", 20, 30);
			kprint_at("A palavra era: ", 21, 25);
			kprint(current_word);
		}
	}
}

void process_guess(char letter)
{
	int i, found = 0;
	char upper_letter = letter;

	/* Converter para maiúscula */
	if (letter >= 'a' && letter <= 'z')
	{
		upper_letter = letter - 32;
	}

	/* Verificar se já foi adivinhada */
	if (guessed_letters[upper_letter - 'A'])
	{
		return;
	}

	/* Marcar como adivinhada */
	guessed_letters[upper_letter - 'A'] = 1;

	/* Verificar se a letra está na palavra */
	for (i = 0; i < word_length; i++)
	{
		if (current_word[i] == upper_letter)
		{
			guessed_word[i] = upper_letter;
			found = 1;
		}
	}

	if (!found)
	{
		wrong_guesses++;
		if (wrong_guesses >= MAX_WRONG_GUESSES)
		{
			game_over = 1;
		}
	}
	else
	{
		/* Verificar se a palavra está completa */
		won = 1;
		for (i = 0; i < word_length; i++)
		{
			if (guessed_word[i] == '_')
			{
				won = 0;
				break;
			}
		}
		if (won)
			game_over = 1;
	}
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode, pressed_char;

	write_port(0x20, 0x20); /* EOI */

	status = read_port(KEYBOARD_STATUS_PORT);
	if (status & 0x01)
	{
		keycode = read_port(KEYBOARD_DATA_PORT);
		if (keycode < 0)
			return;

		pressed_char = keyboard_map[(unsigned char)keycode];

		/* Reiniciar jogo */
		if (pressed_char == '0' || pressed_char == '0')
		{
			init_game();
			draw_game();
			return;
		}

		/* Processar adivinhação de letra */
		if (!game_over && ((pressed_char >= 'a' && pressed_char <= 'z') ||
						   (pressed_char >= 'A' && pressed_char <= 'Z')))
		{
			process_guess(pressed_char);
			draw_game();
		}
	}
}

void kmain(void)
{
	idt_init();
	kb_init();
	init_game();
	draw_game();
	while (1)
		;
}