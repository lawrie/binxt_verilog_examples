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

char games[MAX_GAMES][9];
uint32_t first_clusters[MAX_GAMES];
uint32_t file_sizes[MAX_GAMES];

int num_games;

uint8_t buffer[512];
uint32_t cluster_begin_lba;
uint8_t sectors_per_cluster;
uint32_t fat[128];
uint32_t fat_begin_lba;

void read_files() {
    uint8_t filename[13], first_file[13];
    filename[8] = '.';
    filename[12] = 0;

    for(int i=0; buffer[i];i+=32) {
        if (buffer[i] != 0xe5) {
            if (buffer[i+11] != 0x0f) {
                for(int j=0;j<8;j++) filename[j] = buffer[i+j];
                for(int j=0;j<3;j++) filename[9+j] = buffer[i+8+j];
                uint8_t attrib = buffer[i+0x0b];
                uint16_t first_cluster_hi = *((uint16_t *) &buffer[i+0x14]);
                uint16_t first_cluster_lo = *((uint16_t *) &buffer[i+0x1a]);
                uint32_t first_cluster = (first_cluster_hi << 16) + first_cluster_lo;
                uint32_t file_size = *((uint32_t *) &buffer[i+0x1c]);
                if ((attrib & 0x1f) == 0 && num_games < MAX_GAMES) {
                  for(int j=0;j<8;j++) games[num_games][j] = filename[j];
                  print(filename);
                  set_chars(num_games, filename, 12);
                  print(" ");
                  print_hex(first_cluster, 8);
                  print(" ");
                  print_hex(file_size, 8);
                  print("\n");
                  games[num_games][8] = 0;
                  first_clusters[num_games] = first_cluster;
                  file_sizes[num_games] = file_size;
                  num_games++;
                }
            }
        }
    }

}

int main() {
	// 9600 baud at 25MHz
	reg_uart_clkdiv = 2604;

	while (getchar_prompt("Press ENTER to continue..\n") != '\r') { /* wait */ }

	print("Game Menu\n");

        sdcard_init();

        LED = 0x30;

        print("sdcard init done\n");


        // Read the master boot record
        sdcard_read(buffer, 0);

#ifdef debug
        if (buffer[510] == 0x55 && buffer[511] == 0xAA) print("MBR is valid.\n\n");
#endif

        uint8_t *part = &buffer[446];

#ifdef debug
        printf("Boot flag: %d\n", part[0]);
        printf("Type code: 0x%x\n", part[4]);
#endif

	uint16_t *lba_begin = (uint16_t *) &part[8];

        uint32_t part_lba_begin = lba_begin[0];

#ifdef debug
        printf("LBA begin: %ld\n", part_lba_begin);
#endif

        // Read the volume id
        sdcard_read(buffer, part_lba_begin);

#ifdef debug
        if (buffer[510] == 0x55 && buffer[511] == 0xAA) print("Volume ID is valid\n\n");
#endif

        uint16_t num_rsvd_sectors = *((uint16_t *) &buffer[0x0e]);

#ifdef debug
        printf("Number of reserved sectors: %d\n", num_rsvd_sectors);
#endif

	uint8_t num_fats = buffer[0x10];
        uint32_t sectors_per_fat = *((uint32_t *) &buffer[0x24]);
        sectors_per_cluster = buffer[0x0d];
        uint32_t root_dir_first_cluster = *((uint32_t *) &buffer[0x2c]);

#ifdef debug
        printf("Number of FATs: %d\n", num_fats);
        printf("Sectors per FAT: %ld\n", sectors_per_fat);
        printf("Sectors per cluster: %d\n", sectors_per_cluster);
        printf("Root dir first cluster: %ld\n", root_dir_first_cluster);
#endif

        fat_begin_lba = part_lba_begin + num_rsvd_sectors;

#ifdef debug
        printf("fat begin lba: %ld\n", fat_begin_lba);
#endif

        // Assumes 2 FATs
        cluster_begin_lba = part_lba_begin + num_rsvd_sectors + (sectors_per_fat << 1);

#ifdef debug
        printf("cluster begin lba: %ld\n", cluster_begin_lba);
#endif

        // Assumes 8 sectors per cluster
        uint32_t root_dir_lba = cluster_begin_lba + ((root_dir_first_cluster - 2) << 3);

#ifdef debug
        printf("root dir lba: %ld\n", root_dir_lba);
#endif

        // Read the root directory
        sdcard_read(buffer, root_dir_lba);

        read_files();

	char led;
	while (1) {
        	delay();
		delay();
		delay();
		led = led + 1;
		LED = led;
    	}
}
