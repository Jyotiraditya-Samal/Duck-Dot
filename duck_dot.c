#include <gba_video.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <stdlib.h>

// Game constants
#define GRID_MIN 3
#define GRID_MAX 6
#define TILE_SIZE 20
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

// Players
typedef enum { DUCK, DOT, NONE } Player;
Player current_player = DUCK;

// Grid state
int grid_size = GRID_MIN;
int grid[GRID_MAX][GRID_MAX]; // 0 = empty, 1 = Duck, 2 = Dot
int cursor_x = 0, cursor_y = 0;

// Colors (RGB15)
#define COLOR_BG 0x7C00  // Red
#define COLOR_GRID 0x7FFF // White
#define COLOR_DUCK 0x03E0 // Green
#define COLOR_DOT 0x001F  // Blue
#define COLOR_CURSOR 0x7FF0 // Cyan

// Draw a tile
void draw_tile(int x, int y, int color) {
  int screen_x = (SCREEN_WIDTH/2) - (grid_size*TILE_SIZE/2) + x*TILE_SIZE;
  int screen_y = (SCREEN_HEIGHT/2) - (grid_size*TILE_SIZE/2) + y*TILE_SIZE;
  
  for (int dy = 0; dy < TILE_SIZE; dy++) {
    for (int dx = 0; dx < TILE_SIZE; dx++) {
      MODE3_FB[screen_y + dy][screen_x + dx] = color;
    }
  }
}

// Check for 2×2 squares
int check_squares() {
  for (int y = 0; y < grid_size-1; y++) {
    for (int x = 0; x < grid_size-1; x++) {
      int player = grid[y][x];
      if (player != NONE &&
          grid[y][x+1] == player &&
          grid[y+1][x] == player &&
          grid[y+1][x+1] == player) {
        return player;
      }
    }
  }
  return NONE;
}

// Check rows/columns/diagonals
int check_lines() {
  // Rows and columns
  for (int i = 0; i < grid_size; i++) {
    int row = grid[i][0];
    int col = grid[0][i];
    for (int j = 1; j < grid_size; j++) {
      if (grid[i][j] != row) row = NONE;
      if (grid[j][i] != col) col = NONE;
    }
    if (row != NONE) return row;
    if (col != NONE) return col;
  }
  
  // Diagonals
  int diag1 = grid[0][0], diag2 = grid[0][grid_size-1];
  for (int i = 1; i < grid_size; i++) {
    if (grid[i][i] != diag1) diag1 = NONE;
    if (grid[i][grid_size-1-i] != diag2) diag2 = NONE;
  }
  return (diag1 != NONE) ? diag1 : diag2;
}

// Expand grid from a random corner
void expand_grid() {
  int corner = rand() % 4; // 0=TL, 1=TR, 2=BL, 3=BR
  grid_size++;
  
  // Shift grid based on corner
  for (int y = (corner >= 2) ? grid_size-1 : 0; 
       (corner >= 2) ? y > 0 : y < grid_size-1; 
       (corner >= 2) ? y-- : y++) {
    for (int x = (corner % 2 == 1) ? grid_size-1 : 0; 
         (corner % 2 == 1) ? x > 0 : x < grid_size-1; 
         (corner % 2 == 1) ? x-- : x++) {
      grid[y][x] = grid[y - (corner >= 2)][x - (corner % 2)];
    }
  }
}

// Main game loop
int main() {
  SetMode(MODE_3 | BG2_ENABLE);
  irqInit();
  irqEnable(IRQ_VBLANK);

  while (1) {
    scanKeys();
    u16 keys = keysDown();

    // Move cursor
    if (keys & KEY_LEFT && cursor_x > 0) cursor_x--;
    if (keys & KEY_RIGHT && cursor_x < grid_size-1) cursor_x++;
    if (keys & KEY_UP && cursor_y > 0) cursor_y--;
    if (keys & KEY_DOWN && cursor_y < grid_size-1) cursor_y++;

    // Place symbol
    if (keys & KEY_A && grid[cursor_y][cursor_x] == NONE) {
      grid[cursor_y][cursor_x] = current_player;
      
      // Check win
      int winner = check_lines();
      if (winner == NONE) winner = check_squares();
      
      if (winner != NONE) {
        // Reset game
        grid_size = GRID_MIN;
        for (int y = 0; y < GRID_MAX; y++)
          for (int x = 0; x < GRID_MAX; x++)
            grid[y][x] = NONE;
        current_player = DUCK;
      } else {
        // Check if grid is full
        int filled = 1;
        for (int y = 0; y < grid_size; y++)
          for (int x = 0; x < grid_size; x++)
            if (grid[y][x] == NONE) filled = 0;
        
        if (filled && grid_size < GRID_MAX) expand_grid();
        current_player = (current_player == DUCK) ? DOT : DUCK;
      }
    }

    // Draw grid
    for (int y = 0; y < grid_size; y++) {
      for (int x = 0; x < grid_size; x++) {
        int color = COLOR_BG;
        if (grid[y][x] == DUCK) color = COLOR_DUCK;
        else if (grid[y][x] == DOT) color = COLOR_DOT;
        draw_tile(x, y, color);
      }
    }

    // Draw cursor
    draw_tile(cursor_x, cursor_y, COLOR_CURSOR);
    VBlankIntrWait();
  }
  return 0;
}
