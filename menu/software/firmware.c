#include <stdint.h>
#include "sdcard.h"

//#define debug

#define LED (*(volatile uint32_t*)0x02000000)

#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)

#define reg_game_menu (*(volatile uint32_t*)0x03000000)
#define reg_game_index (*(volatile uint32_t*)0x03000004)

#define MAX_GAMES 8

#define printf sdcard_error

#define USER_DATA 0x400000

void set_char(unsigned int y, unsigned int x, char ch)
{
	reg_game_menu = (y << 16) | (x << 8) | ch;
}

void set_chars(unsigned int y, char fn[], unsigned int len)
{
	for(int i=0; i<16;i++) 
		set_char(y, i, i < len ? fn[i] : ' ');
}

void putchar(char c)
{
    if (c == '\n')
        putchar('\r');
    reg_uart_data = c;
}

void print_hex(unsigned int val, int digits)
{
        for (int i = (4*digits)-4; i >= 0; i -= 4)
                reg_uart_data = "0123456789ABCDEF"[(val >> i) % 16];
}

void print(const char *p)
{
    while (*p)
        putchar(*(p++));
}

void delay() {
    for (volatile int i = 0; i < 25000; i++)
        ;
}

char getchar_prompt(char *prompt)
{
	int32_t c = -1;

	uint32_t cycles_begin, cycles_now, cycles;
	__asm__ volatile ("rdcycle %0" : "=r"(cycles_begin));

	if (prompt)
		print(prompt);

	while (c == -1) {
		__asm__ volatile ("rdcycle %0" : "=r"(cycles_now));
		cycles = cycles_now - cycles_begin;
		if (cycles > 12000000) {
			if (prompt)
				print(prompt);
			cycles_begin = cycles_now;
		}
		c = reg_uart_data;
	}
	return c;
}

void sdcard_error(char* msg, uint32_t r) {
  print(msg);
  print(" : ");
  print_hex(r, 8);
  print("\n");
}

void sdcard_error2(char* msg, uint32_t r1, uint32_t r2) {
  print(msg);
  print(" : ");
  print_hex(r1, 8);
  print(" , ");
  print_hex(r2, 8);
  print("\n");
}

int main() {
	// 9600 baud at 25MHz
	reg_uart_clkdiv = 2604;

	while (getchar_prompt("Press ENTER to continue..\n") != '\r') { /* wait */ }

	print("Game Menu\n");

        sdcard_init();

        LED = 0x30;

        print("sdcard init done\n");

	while (1) {
        	delay();
		delay();
		delay();
		LED = LED + 1;
    	}
}
