/*
 CLIsweeper - Minesweeper for the command line.
 Copyright (C) 2017  James Shiffer
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

typedef enum
{
    BLANK,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    MINE,
    UNDISCOVERED
} tile_state_t;

typedef struct
{
    tile_state_t type;
    bool flagged;
    bool questioned;
} tile_t;

typedef struct
{
    int width;
    int height;
} window_t;

typedef struct
{
    int x;
    int y;
} cursor_t;

typedef enum
{
    IN_PROGRESS,
    LOST,
    WON
} game_state_t;

typedef struct
{
    int rows;
    int cols;
    int mines;
    int moves;
    int flags;
    game_state_t state;
    tile_t** board;
    cursor_t cursor;
    window_t window;
} game_t;


tile_t get_tile_number(game_t* g, int x, int y)
{
    tile_t mines;
    mines.type = BLANK;

    if (x < g->cols - 1)
        if (g->board[x + 1][y].type == MINE)
            ++ mines.type;
    if (x > 0)
        if (g->board[x - 1][y].type == MINE)
            ++ mines.type;
    if (y < g->rows - 1)
        if (g->board[x][y + 1].type == MINE)
            ++ mines.type;
    if (y > 0)
        if (g->board[x][y - 1].type == MINE)
            ++ mines.type;
    if (x < g->cols - 1 && y < g->rows - 1)
        if (g->board[x + 1][y + 1].type == MINE)
            ++ mines.type;
    if (x > 0 && y < g->rows - 1)
        if (g->board[x - 1][y + 1].type == MINE)
            ++ mines.type;
    if (x < g->cols - 1 && y > 0)
        if (g->board[x + 1][y - 1].type == MINE)
            ++ mines.type;
    if (x > 0 && y > 0)
        if (g->board[x - 1][y - 1].type == MINE)
            ++ mines.type;

    return mines;
}

void place_mines(game_t* g)
{
    srand((unsigned) time(NULL));
    
    int spawn_protection = 1;
    
    int x, y = 0;
    for (int i = 0; i < g->mines; ++i)
    {
        do
        {
            x = rand() % g->cols;
            y = rand() % g->rows;
        } while ((abs(x - g->cursor.x) <= spawn_protection && abs(y - g->cursor.y) <= spawn_protection) || g->board[x][y].type == MINE);
        
        g->board[x][y].type = MINE;
    }
}

void draw_board(game_t* g)
{
    if (has_colors())
    {
        start_color();
        
        if (can_change_color())
        {
            init_color(0, 750, 750, 750);
            init_color(1, 43, 141, 977);
            init_color(2, 59, 492, 70);
            init_color(3, 980, 51, 105);
            init_color(4, 12, 47, 492);
            init_color(5, 496, 12, 39);
            init_color(6, 63, 500, 500);
            init_color(7, 0, 0, 0);
            init_color(8, 500, 500, 500);
            init_color(9, 500, 500, 500);
            init_color(10, 500, 500, 500);
            init_color(11, 1000, 0, 0);
        }
        
        for (int i = 0; i <= 11; ++i)
            init_pair(i, i, 0);
        init_pair(9, 10, 0);
        
    }
    
    if (has_colors()) attron(COLOR_PAIR(6));
    mvprintw(0, 0, "+============+\n| CLIsweeper |\n+============+");
    if (has_colors()) attroff(COLOR_PAIR(6));
    
    for (int i = 0; i < g->cols; ++i)
    {
        for (int j = 0; j < g->rows; ++j)
        {
            char tile;
            if (has_colors())
            {
                if (g->state == IN_PROGRESS && i == g->cursor.x && j == g->cursor.y)
                    attron(COLOR_PAIR(11));
                else
                    attron(COLOR_PAIR((int)g->board[i][j].type));
            }
            if (g->board[i][j].flagged && g->state == IN_PROGRESS)
            {
                if (has_colors()) attron(COLOR_PAIR(6));
                tile = 'F';
            }
            else if (g->board[i][j].questioned && g->state == IN_PROGRESS)
            {
                if (has_colors()) attron(COLOR_PAIR(6));
                tile = '?';
            }
            else
            {
                switch (g->board[i][j].type)
                {
                    case ONE:
                    case TWO:
                    case THREE:
                    case FOUR:
                    case FIVE:
                    case SIX:
                    case SEVEN:
                    case EIGHT:
                        tile = g->board[i][j].type + '0';
                        break;

                    case UNDISCOVERED:
                        if (g->state == WON || g->state == LOST)
                        {
                            tile_t t = get_tile_number(g, i, j);
                            
                            if (has_colors())
                            {
                                attroff(COLOR_PAIR(10));
                                attron(COLOR_PAIR((int)t.type));
                            }
                            
                            if (t.type == BLANK)
                            {
                                tile = ' ';
                            }
                            else
                            {
                                tile = (int)t.type + '0';
                            }
                        }
                        else
                        {
                            tile = '#';
                        }
                        break;

                    case MINE:
                        if (g->state == WON || g->state == LOST)
                        {
                            if (has_colors())
                            {
                                attroff(COLOR_PAIR(10));
                                attron(COLOR_PAIR(5));
                            }
                            
                            tile = '*';
                        }
                        else
                        {
                            tile = '#';
                        }
                        break;

                    case BLANK:
                    default:
                        if (has_colors() && g->state == IN_PROGRESS && i == g->cursor.x && j == g->cursor.y)
                            tile = 'o';
                        else
                            tile = ' ';
                }
            }
            mvprintw(j + 5, i * 2, "%c", tile);
            if (has_colors())
            {
                if ((i == g->cursor.x && j == g->cursor.y) || g->board[i][j].flagged || g->board[i][j].questioned)
                    attroff(COLOR_PAIR(6));
                else
                    attroff(COLOR_PAIR((int)g->board[i][j].type));
            }
        }
    }
    
    if (!has_colors())
    {
        mvprintw(g->rows + 5, 0, "x:%d y:%d ", g->cursor.x + 1, g->cursor.y + 1);
    }
    mvprintw(g->rows + 7, 0, "Flags Left: %d ", g->mines - g->flags);
    
}

void clear_board(game_t* g)
{
    for (int i = 0; i < g->cols; ++i)
    {
        for (int j = 0; j < g->rows; ++j)
        {
            g->board[i][j].type = UNDISCOVERED;
            g->board[i][j].flagged = false;
            g->board[i][j].questioned = false;
        }
    }
}

void reveal_surrounding_blanks(game_t* g, int x, int y)
{
    // This only applies to blank tiles
    if (get_tile_number(g, x, y).type != BLANK)
        return;
    
    if (x < g->cols - 1)
    {
        if (g->board[x + 1][y].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x + 1, y).type == BLANK)
            {
                g->board[x + 1][y].type = BLANK;
                reveal_surrounding_blanks(g, x + 1, y);
            }
            else if (g->board[x + 1][y].type != MINE)
            {
                g->board[x + 1][y].type = get_tile_number(g, x + 1, y).type;
            }
        }
    }
    if (x > 0)
    {
        if (g->board[x - 1][y].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x - 1, y).type == BLANK)
            {
                g->board[x - 1][y].type = BLANK;
                reveal_surrounding_blanks(g, x - 1, y);
            }
            else if (g->board[x - 1][y].type != MINE)
            {
                g->board[x - 1][y].type = get_tile_number(g, x - 1, y).type;
            }
        }
    }
    if (y < g->rows - 1)
    {
        if (g->board[x][y + 1].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x, y + 1).type == BLANK)
            {
                g->board[x][y + 1].type = BLANK;
                reveal_surrounding_blanks(g, x, y + 1);
            }
            else if (g->board[x][y + 1].type != MINE)
            {
                g->board[x][y + 1].type = get_tile_number(g, x, y + 1).type;
            }
        }
    }
    if (y > 0)
    {
        if (g->board[x][y - 1].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x, y - 1).type == BLANK)
            {
                g->board[x][y - 1].type = BLANK;
                reveal_surrounding_blanks(g, x, y - 1);
            }
            else if (g->board[x][y - 1].type != MINE)
            {
                g->board[x][y - 1].type = get_tile_number(g, x, y - 1).type;
            }
        }
    }
    
    if (x < g->cols - 1 && y < g->rows - 1)
    {
        if (g->board[x + 1][y + 1].type == UNDISCOVERED)
        {
            // Blanks do not reveal diagonally (?)
            if (get_tile_number(g, x + 1, y + 1).type == BLANK)
            {
                g->board[x + 1][y + 1].type = BLANK;
                reveal_surrounding_blanks(g, x + 1, y + 1);
            }
            else if (g->board[x + 1][y + 1].type != MINE)// && get_tile_number(g, x + 1, y + 1).type != BLANK)
            {
                g->board[x + 1][y + 1].type = get_tile_number(g, x + 1, y + 1).type;
            }
        }
    }
    if (x > 0 && y < g->rows - 1)
    {
        if (g->board[x - 1][y + 1].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x - 1, y + 1).type == BLANK)
            {
                g->board[x - 1][y + 1].type = BLANK;
                reveal_surrounding_blanks(g, x - 1, y + 1);
            }
            else if (g->board[x - 1][y + 1].type != MINE)
            {
                g->board[x - 1][y + 1].type = get_tile_number(g, x - 1, y + 1).type;
            }
        }
    }
    if (x < g->cols - 1 && y > 0)
    {
        if (g->board[x + 1][y - 1].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x + 1, y - 1).type == BLANK)
            {
                g->board[x + 1][y - 1].type = BLANK;
                reveal_surrounding_blanks(g, x + 1, y - 1);
            }
            else if (g->board[x + 1][y - 1].type != MINE)
            {
                g->board[x + 1][y - 1].type = get_tile_number(g, x + 1, y - 1).type;
            }
        }
    }
    if (x > 0 && y > 0)
    {
        if (g->board[x - 1][y - 1].type == UNDISCOVERED)
        {
            if (get_tile_number(g, x - 1, y - 1).type == BLANK)
            {
                g->board[x - 1][y - 1].type = BLANK;
                reveal_surrounding_blanks(g, x - 1, y - 1);
            }
            else if (g->board[x - 1][y - 1].type != MINE)
            {
                g->board[x - 1][y - 1].type = get_tile_number(g, x - 1, y - 1).type;
            }
        }
    }
}

game_state_t process_input(int c, game_t* g)
{
    switch (c)
    {
        case KEY_LEFT:
            if (g->cursor.x > 0)
                -- g->cursor.x;
            break;

        case KEY_RIGHT:
            if (g->cursor.x < g->cols - 1)
                ++ g->cursor.x;
            break;

        case KEY_UP:
            if (g->cursor.y > 0)
                -- g->cursor.y;
            break;

        case KEY_DOWN:
            if (g->cursor.y < g->rows - 1)
                ++ g->cursor.y;
            break;

        case 'f':
            if ((g->board[g->cursor.x][g->cursor.y].type == UNDISCOVERED || g->board[g->cursor.x][g->cursor.y].type == MINE) && !g->board[g->cursor.x][g->cursor.y].questioned)
            {
                if (g->board[g->cursor.x][g->cursor.y].flagged)
                {
                    -- g->flags;
                    g->board[g->cursor.x][g->cursor.y].flagged = false;
                }
                else
                {
                    if (g->flags < g->mines)
                    {
                        ++ g->flags;
                        g->board[g->cursor.x][g->cursor.y].flagged = true;
                    }
                }
            }
            break;
        
        case 'g':
            if ((g->board[g->cursor.x][g->cursor.y].type == UNDISCOVERED || g->board[g->cursor.x][g->cursor.y].type == MINE) && !g->board[g->cursor.x][g->cursor.y].flagged)
                g->board[g->cursor.x][g->cursor.y].questioned = !g->board[g->cursor.x][g->cursor.y].questioned;
            break;
            
        case ' ':
        {
            if (g->moves == 0)
                place_mines(g);
            
            ++ g->moves;
            
            // Do nothing if it's flagged/questioned
            if (g->board[g->cursor.x][g->cursor.y].flagged || g->board[g->cursor.x][g->cursor.y].questioned)
                return IN_PROGRESS;
            
            // If it's a mine, game is over
            if (g->board[g->cursor.x][g->cursor.y].type == MINE)
                return LOST;
            
            // If it's undiscovered, discover it
            tile_t tile = get_tile_number(g, g->cursor.x, g->cursor.y);
            tile.flagged = false;
            tile.questioned = false;
            if (g->board[g->cursor.x][g->cursor.y].type == UNDISCOVERED)
            {
                // If it's in a "blank cluster", reveal the whole cluster
                reveal_surrounding_blanks(g, g->cursor.x, g->cursor.y);
                g->board[g->cursor.x][g->cursor.y] = tile;
            }
            
            // Check if all tiles are discovered
            bool win = true;
            for (int i = 0; i < g->cols; ++i)
            {
                for (int j = 0; j < g->rows; ++j)
                {
                    if (g->board[i][j].type == UNDISCOVERED)
                        win = false;
                }
            }
            if (win) return WON;
            
            break;
        }

        case 'q':
            g->state = LOST;
            return true;
    }
    return false;
}

void init_game(game_t* g, int rows, int cols, int mines)
{
    g->state = IN_PROGRESS;
    g->moves = 0;
    g->flags = 0;
    g->rows = rows;
    g->cols = cols;
    g->mines = mines;
    
    g->board = (tile_t**) malloc(g->cols * sizeof(tile_t*));
    
    for (int i = 0; i < g->cols; ++i)
    {
        g->board[i] = (tile_t*) malloc(g->rows * sizeof(tile_t));
    }
    
    g->cursor.x = g->cols / 2 - 1;
    g->cursor.y = g->rows / 2 - 1;
    
    clear_board(g);
}

void deinit_game(game_t* g)
{
    for (int i = 0; i < g->cols; ++i)
    {
        free(g->board[i]);
    }
    
    free(g->board);
}

int main(int argc, char** argv)
{
    printf("CLIsweeper  Copyright (C) 2017  James Shiffer\nThis program comes with ABSOLUTELY NO WARRANTY; for details see LICENSE.\nThis is free software, and you are welcome to redistribute it under certain conditions; see LICENSE for details.\n\n");
    
    printf("Enter a difficulty (\"e\", \"m\", or \"h\"): ");
    char difficulty = 'e';
    difficulty = getchar();
    
    initscr();
    savetty();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    timeout(0);
    keypad(stdscr, true);
    curs_set(false);
    clear();

    game_t game;
    
    if (difficulty == 'h')
        init_game(&game, 16, 30, 99);
    else if (difficulty == 'm')
        init_game(&game, 16, 16, 40);
    else
        init_game(&game, 9, 9, 10);
    
    if (game.board == NULL) return 1;

    // Game loop
    while (game.state == IN_PROGRESS)
    {
        getmaxyx(stdscr, game.window.height, game.window.width);
        //clear();

        game.state = process_input(getch(), &game);

        draw_board(&game);

        refresh();
        usleep(20000);
    }
    
    draw_board(&game);
    
    if (game.state == WON)
        mvprintw(game.rows + 7, 0, "You win! :)\nPress Q to quit");
    else
        mvprintw(game.rows + 7, 0, "Game over :(\nPress Q to quit");
    
    while (getch() != 'q');
    
    deinit_game(&game);
    
    curs_set(true);
    endwin();
    return 0;
}
