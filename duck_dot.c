#include <gba.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Screen and Grid Config ---
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   160
#define TILE_SIZE       20
#define GRID_MIN        3
#define GRID_MAX        6
#define HUD_HEIGHT_TOP  10
#define HUD_HEIGHT_BOTTOM 10

// --- Color Definitions ---
#define RGB15(r,g,b) ((r)|((g)<<5)|((b)<<10))
#define COLOR_BG       RGB15(0,0,0)
#define COLOR_DUCK     RGB15(31,31,0)
#define COLOR_DOT      RGB15(31,31,31)
#define COLOR_CURSOR   RGB15(0,31,31)
#define COLOR_TEXT     RGB15(31,31,31)

// --- HUD ---
#define HUD_PLAYER_Y 2
#define MSG_DURATION 60
#define MSG_Y (SCREEN_HEIGHT - 8 - 2)

// --- Game Types ---
typedef enum { NONE = 0, DUCK = 1, DOT = 2 } Player;

// --- Global Variables ---
Player grid[GRID_MAX][GRID_MAX];
int grid_size = GRID_MIN;
Player current_player = DUCK;

int cursor_x = 0;
int cursor_y = 0;
int prev_cursor_x = 0;
int prev_cursor_y = 0;

char message_buffer[64] = {0};
int message_timer = 0;

// --- Video Memory ---
volatile u16* videoBuffer = (volatile u16*)0x06000000;

// --- Helper Functions ---
int get_grid_origin_x() { 
    return (SCREEN_WIDTH - grid_size * TILE_SIZE) / 2; 
}

int get_grid_origin_y() { 
    return HUD_HEIGHT_TOP + (SCREEN_HEIGHT - HUD_HEIGHT_TOP - grid_size * TILE_SIZE) / 2; 
}

// Full 8x8 ASCII font for GBA (printable characters 0x20 - 0x7E)
const u8 font8x8[128][8] = {
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['!'] = {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    ['"'] = {0x36,0x36,0x24,0x00,0x00,0x00,0x00,0x00},
    ['#'] = {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00},
    ['$'] = {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},
    ['%'] = {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
    ['&'] = {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},
    ['\''] = {0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00},
    ['('] = {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    [')'] = {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    ['*'] = {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    ['+'] = {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    [','] = {0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00},
    ['-'] = {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    ['/'] = {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
    ['0'] = {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    ['1'] = {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['2'] = {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00},
    ['3'] = {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    ['4'] = {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00},
    ['5'] = {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    ['6'] = {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
    ['7'] = {0x7E,0x66,0x0C,0x18,0x18,0x18,0x18,0x00},
    ['8'] = {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    ['9'] = {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    [':'] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
    [';'] = {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00},
    ['<'] = {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00},
    ['='] = {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
    ['>'] = {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00},
    ['?'] = {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00},
    ['@'] = {0x3C,0x66,0x6E,0x6E,0x6C,0x60,0x3E,0x00},
    ['A'] = {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00},
    ['B'] = {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    ['C'] = {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
    ['D'] = {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    ['E'] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00},
    ['F'] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},
    ['G'] = {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},
    ['H'] = {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    ['I'] = {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    ['J'] = {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00},
    ['K'] = {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
    ['L'] = {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    ['M'] = {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    ['N'] = {0x66,0x76,0x7E,0x6E,0x66,0x66,0x66,0x00},
    ['O'] = {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    ['P'] = {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    ['Q'] = {0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00},
    ['R'] = {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00},
    ['S'] = {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
    ['T'] = {0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00},
    ['U'] = {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    ['V'] = {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    ['W'] = {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    ['X'] = {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    ['Y'] = {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00},
    ['Z'] = {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
    ['['] = {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
    ['\\'] = {0x40,0x60,0x30,0x18,0x0C,0x06,0x02,0x00},
    [']'] = {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
    ['^'] = {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00},
    ['_'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    ['`'] = {0x18,0x0C,0x00,0x00,0x00,0x00,0x00,0x00},
    ['a'] = {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00},
    ['b'] = {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00},
    ['c'] = {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00},
    ['d'] = {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00},
    ['e'] = {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00},
    ['f'] = {0x1C,0x36,0x30,0x7C,0x30,0x30,0x30,0x00},
    ['g'] = {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x7C},
    ['h'] = {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00},
    ['i'] = {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    ['j'] = {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x6C,0x38},
    ['k'] = {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00},
    ['l'] = {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    ['m'] = {0x00,0x00,0x6C,0x7E,0x6B,0x6B,0x63,0x00},
    ['n'] = {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00},
    ['o'] = {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00},
    ['p'] = {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60},
    ['q'] = {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06},
    ['r'] = {0x00,0x00,0x6C,0x76,0x60,0x60,0x60,0x00},
    ['s'] = {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00},
    ['t'] = {0x30,0x30,0x7C,0x30,0x30,0x36,0x1C,0x00},
    ['u'] = {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00},
    ['v'] = {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00},
    ['w'] = {0x00,0x00,0x63,0x6B,0x7F,0x36,0x36,0x00},
    ['x'] = {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00},
    ['y'] = {0x00,0x00,0x66,0x66,0x3E,0x06,0x3C,0x00},
    ['z'] = {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00},
    ['{'] = {0x0E,0x0C,0x0C,0x38,0x0C,0x0C,0x0E,0x00},
    ['|'] = {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
    ['}'] = {0x70,0x30,0x30,0x1C,0x30,0x30,0x70,0x00},
    ['~'] = {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00},
};

// --- Basic Drawing Functions ---
static inline void put_pixel(int x, int y, u16 color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        videoBuffer[y * SCREEN_WIDTH + x] = color;
}

void draw_square(int x, int y, int size, u16 color) {
    for (int dy = 0; dy < size; dy++)
        for (int dx = 0; dx < size; dx++)
            put_pixel(x + dx, y + dy, color);
}

void draw_circle(int cx, int cy, int radius, u16 color) {
    for (int y = -radius; y <= radius; y++)
        for (int x = -radius; x <= radius; x++)
            if (x * x + y * y <= radius * radius)
                put_pixel(cx + x, cy + y, color);
}

void draw_char(int x, int y, char c, u16 color) {
    for (int row = 0; row < 8; row++) {
        u8 row_bits = font8x8[(int)c][row];
        for (int col = 0; col < 8; col++)
            if (row_bits & (1 << (7 - col)))
                put_pixel(x + col, y + row, color);
    }
}

void draw_string(int x, int y, const char *str, u16 color) {
    int start_x = x;
    while (*str) {
        if (*str == '\n') { 
            y += 8; 
            x = start_x; 
        } else {
            draw_char(x, y, *str, color); 
            x += 8;
        }
        str++;
    }
}

// --- HUD Functions ---
void show_message(const char* text) {
    strncpy(message_buffer, text, sizeof(message_buffer)-1);
    message_buffer[sizeof(message_buffer)-1] = 0;
    message_timer = MSG_DURATION;
}

void draw_hud() {
    // Clear top HUD (player turn)
    for (int y = 0; y < HUD_HEIGHT_TOP; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            put_pixel(x, y, COLOR_BG);

    // Draw current player turn
    const char* turn_text = (current_player == DUCK) ? "DUCK'S TURN" : "DOT'S TURN";
    draw_string(2, HUD_PLAYER_Y, turn_text, COLOR_TEXT);

    // Draw bottom HUD message if active
    if (message_timer > 0) {
        draw_string(2, MSG_Y, message_buffer, COLOR_TEXT);
        message_timer--;
    } else {
        // Clear bottom HUD after message
        for (int y = MSG_Y; y < SCREEN_HEIGHT; y++)
            for (int x = 0; x < SCREEN_WIDTH; x++)
                put_pixel(x, y, COLOR_BG);
    }
}

// --- Grid Drawing Functions ---
void draw_tile(int grid_x, int grid_y, Player player, int highlight) {
    int screen_x = get_grid_origin_x() + grid_x * TILE_SIZE;
    int screen_y = get_grid_origin_y() + grid_y * TILE_SIZE;

    // Clear interior leaving 1-pixel border
    draw_square(screen_x + 1, screen_y + 1, TILE_SIZE - 2, COLOR_BG);

    if (player == DUCK) draw_square(screen_x + 3, screen_y + 3, TILE_SIZE - 6, COLOR_DUCK);
    else if (player == DOT) draw_circle(screen_x + TILE_SIZE / 2, screen_y + TILE_SIZE / 2, TILE_SIZE / 3, COLOR_DOT);

    // Draw highlight border
    if (highlight) {
        for (int i = 1; i < TILE_SIZE - 1; i++) {
            put_pixel(screen_x + i, screen_y + 1, COLOR_CURSOR);            // top
            put_pixel(screen_x + i, screen_y + TILE_SIZE - 2, COLOR_CURSOR); // bottom
            put_pixel(screen_x + 1, screen_y + i, COLOR_CURSOR);            // left
            put_pixel(screen_x + TILE_SIZE - 2, screen_y + i, COLOR_CURSOR); // right
        }
    }
}

void clear_tile_highlight(int grid_x, int grid_y) {
    draw_tile(grid_x, grid_y, grid[grid_y][grid_x], 0);
}

void draw_grid_outline() {
    int origin_x = get_grid_origin_x();
    int origin_y = get_grid_origin_y();

    for (int i = 0; i < grid_size * TILE_SIZE; i++) {
        put_pixel(origin_x + i, origin_y, COLOR_DOT);
        put_pixel(origin_x + i, origin_y + grid_size * TILE_SIZE - 1, COLOR_DOT);
        put_pixel(origin_x, origin_y + i, COLOR_DOT);
        put_pixel(origin_x + grid_size * TILE_SIZE - 1, origin_y + i, COLOR_DOT);
    }
}

// --- Win Checking ---
int check_squares() {
    for (int y = 0; y < grid_size - 1; y++)
        for (int x = 0; x < grid_size - 1; x++) {
            int p = grid[y][x];
            if (p != NONE &&
                grid[y][x+1] == p &&
                grid[y+1][x] == p &&
                grid[y+1][x+1] == p)
                return p;
        }
    return NONE;
}

int check_lines() {
    for (int i = 0; i < grid_size; i++) {
        int row_val = grid[i][0];
        int col_val = grid[0][i];

        for (int j = 1; j < grid_size; j++) {
            if (grid[i][j] != row_val) row_val = NONE;
            if (grid[j][i] != col_val) col_val = NONE;
        }

        if (row_val != NONE) return row_val;
        if (col_val != NONE) return col_val;
    }

    int diag1 = grid[0][0], diag2 = grid[0][grid_size - 1];
    for (int i = 1; i < grid_size; i++) {
        if (grid[i][i] != diag1) diag1 = NONE;
        if (grid[i][grid_size - 1 - i] != diag2) diag2 = NONE;
    }

    return (diag1 != NONE) ? diag1 : diag2;
}

// --- Reset Functions ---
void reset_grid() {
    int origin_x = (SCREEN_WIDTH - GRID_MAX * TILE_SIZE) / 2;
    int origin_y = HUD_HEIGHT_TOP + (SCREEN_HEIGHT - HUD_HEIGHT_TOP - GRID_MAX * TILE_SIZE) / 2;

    // Clear full grid area
    for (int y = 0; y < GRID_MAX * TILE_SIZE; y++)
        for (int x = 0; x < GRID_MAX * TILE_SIZE; x++)
            put_pixel(origin_x + x, origin_y + y, COLOR_BG);

    // Reset logical grid
    for (int y = 0; y < GRID_MAX; y++)
        for (int x = 0; x < GRID_MAX; x++)
            grid[y][x] = NONE;

    grid_size = GRID_MIN;
    cursor_x = cursor_y = prev_cursor_x = prev_cursor_y = 0;
    current_player = (rand() % 2) ? DUCK : DOT;

    // Draw border
    draw_grid_outline();
}

// --- Flash Winner ---
void flash_winner(Player winner) {
    for (int f = 0; f < 4; f++) {
        for (int y = 0; y < grid_size; y++)
            for (int x = 0; x < grid_size; x++) {
                if (grid[y][x] == winner) {
                    int screen_x = get_grid_origin_x() + x * TILE_SIZE;
                    int screen_y = get_grid_origin_y() + y * TILE_SIZE;
                    u16 color = (f % 2 == 0) ? COLOR_BG : (winner == DUCK ? COLOR_DUCK : COLOR_DOT);
                    draw_square(screen_x + 3, screen_y + 3, TILE_SIZE - 6, color);
                }
            }
        VBlankIntrWait(); VBlankIntrWait();
    }
}

// --- Main Loop ---
int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);
    SetMode(MODE_3 | BG2_ENABLE);

    reset_grid();

    draw_tile(cursor_x, cursor_y, grid[cursor_y][cursor_x], 1);
    draw_hud();

    while (1) {
        scanKeys();
        u16 keys_pressed = keysDown();
        int cursor_moved = 0;

        // Move cursor
        if (keys_pressed & KEY_LEFT && cursor_x > 0) { cursor_x--; cursor_moved = 1; }
        if (keys_pressed & KEY_RIGHT && cursor_x < grid_size - 1) { cursor_x++; cursor_moved = 1; }
        if (keys_pressed & KEY_UP && cursor_y > 0) { cursor_y--; cursor_moved = 1; }
        if (keys_pressed & KEY_DOWN && cursor_y < grid_size - 1) { cursor_y++; cursor_moved = 1; }

        if (cursor_moved) {
            clear_tile_highlight(prev_cursor_x, prev_cursor_y);
            draw_tile(cursor_x, cursor_y, grid[cursor_y][cursor_x], 1);
            prev_cursor_x = cursor_x;
            prev_cursor_y = cursor_y;
        }

        // Place symbol
        if (keys_pressed & KEY_A && grid[cursor_y][cursor_x] == NONE) {
            grid[cursor_y][cursor_x] = current_player;
            draw_tile(cursor_x, cursor_y, current_player, 1);

            int winner = check_lines();
            if (winner == NONE) winner = check_squares();

            if (winner != NONE) {
                flash_winner(winner);
                show_message((winner == DUCK) ? "Duck Wins! Resetting..." : "Dot Wins! Resetting...");
                reset_grid();
                draw_tile(cursor_x, cursor_y, grid[cursor_y][cursor_x], 1);
            } else {
                int filled = 1;
                for (int y = 0; y < grid_size; y++)
                    for (int x = 0; x < grid_size; x++)
                        if (grid[y][x] == NONE) filled = 0;

                if (filled) {
                    if (grid_size < GRID_MAX) {
                        grid_size++;
                        int origin_x = get_grid_origin_x();
                        int origin_y = get_grid_origin_y();

                        // Clear new grid area
                        for (int y = 0; y < grid_size * TILE_SIZE; y++)
                            for (int x = 0; x < grid_size * TILE_SIZE; x++)
                                put_pixel(origin_x + x, origin_y + y, COLOR_BG);

                        draw_grid_outline();

                        // Redraw existing tiles
                        for (int y = 0; y < grid_size - 1; y++)
                            for (int x = 0; x < grid_size - 1; x++)
                                draw_tile(x, y, grid[y][x], (x == cursor_x && y == cursor_y));
                    } else {
                        show_message("Full Grid! Resetting...");
                        reset_grid();
                        draw_tile(cursor_x, cursor_y, grid[cursor_y][cursor_x], 1);
                    }
                } else {
                    current_player = (current_player == DUCK) ? DOT : DUCK;
                }
            }
        }

        draw_hud();
        VBlankIntrWait();
    }
}

