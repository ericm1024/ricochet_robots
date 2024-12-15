#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <random>
#include <string>

#include <unistd.h>
#include <termios.h>

enum class color_t
{
    BLUE,
    RED,
    GREEN,
    YELLOW,
    RAINBOW,
};
using enum color_t;

static char color_to_char(color_t c)
{
    switch (c) {
    case BLUE: return 'b';
    case RED: return 'r';
    case GREEN: return 'g';
    case YELLOW: return 'y';
    case RAINBOW: return 'r';
    }
}

enum class shape_t
{
    CRESCENT,
    GEAR,
    PLANET,
    STAR,
    HOLE,
};
using enum shape_t;

static char shape_to_char(shape_t s)
{
    switch (s) {
    case CRESCENT: return 'c';
    case GEAR: return 'g';
    case PLANET: return 'p';
    case STAR: return 's';
    case HOLE: return 'h';
    }
}

enum class direction_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};
using enum direction_t;

static char const * dir_to_str(direction_t dir)
{
    switch (dir) {
    case UP: return "UP";
    case DOWN: return "DOWN";
    case LEFT: return "LEFT";
    case RIGHT: return "RIGHT";
    }
}

struct target
{
    target(color_t c, shape_t s) : color{c}, shape{s} {}

    color_t color;
    shape_t shape;
};

struct robot
{
    color_t color;
    uint8_t row;
    uint8_t col;
};

struct square
{
    bool block_north = false;
    bool block_east = false;
    bool allowable_starting_square = true;

    std::optional<target> target;

    int8_t robot_idx = -1;
};

static constexpr size_t k_board_width = 16;
static constexpr size_t k_board_height = 16;

struct game_state
{
    game_state();
    game_state(game_state const &) = default;

    void draw();

    void move_robot(robot * r, direction_t dir);

private:
    void init_board();
    void select_starting_squares();
    bool can_move(robot * r, direction_t dir);
    void move_single(robot * r, direction_t dir);

    // upper left is 0, 0. First coordinate is row, second is column
    square board[k_board_height][k_board_width];

public:
    robot robots[4];
};

static std::mt19937 rng{std::random_device{}()};

void game_state::init_board()
{
    board[0][2].block_east = true;
    board[0][11].block_east = true;

    board[1][4].block_east = true;
    board[1][5].target.emplace(BLUE, CRESCENT);

    board[2][5].block_north = true;
    board[2][7].block_east = true;
    board[2][11].block_east = true;
    board[2][7].target.emplace(RAINBOW, HOLE);
    board[2][11].target.emplace(RED, PLANET);

    board[3][7].block_north = true;
    board[3][11].block_north = true;
    board[3][13].block_north = true;
    board[3][13].block_east = true;
    board[3][13].target.emplace(YELLOW, CRESCENT);

    board[4][0].block_north = true;
    board[4][3].block_east = true;
    board[4][9].block_east = true;
    board[4][3].target.emplace(RED, STAR);
    board[4][10].target.emplace(GREEN, STAR);

    board[5][3].block_north = true;
    board[5][5].block_east = true;
    board[5][6].block_north = true;
    board[5][10].block_north = true;
    board[5][11].block_east = true;
    board[5][12].block_north = true;
    board[5][6].target.emplace(GREEN, PLANET);
    board[5][12].target.emplace(BLUE, GEAR);

    board[6][1].block_north = true;
    board[6][1].block_east = true;
    board[6][15].block_north = true;
    board[6][1].target.emplace(YELLOW, GEAR);

    board[7][6].block_east = true;
    board[7][7].block_north = true;
    board[7][7].allowable_starting_square = false;
    board[7][8].block_north = true;
    board[7][8].block_east = true;
    board[7][8].allowable_starting_square = false;

    board[8][6].block_east = true;
    board[8][8].block_east = true;
    board[8][7].allowable_starting_square = false;
    board[8][8].allowable_starting_square = false;

    board[9][3].block_north = true;
    board[9][3].block_east = true;
    board[9][7].block_north = true;
    board[9][8].block_north = true;
    board[9][11].block_east = true;
    board[9][12].block_north = true;
    board[9][3].target.emplace(YELLOW, STAR);
    board[9][12].target.emplace(BLUE, STAR);

    board[10][10].block_east = true;
    board[10][15].block_north = true;
    board[10][10].target.emplace(YELLOW, PLANET);

    board[11][5].block_east = true;
    board[11][6].block_north = true;
    board[11][10].block_north = true;
    board[11][6].target.emplace(BLUE, PLANET);

    board[12][0].block_east = true;
    board[12][14].block_east = true;
    board[12][14].block_north = true;
    board[12][1].target.emplace(GREEN, GEAR);
    board[12][14].target.emplace(RED, GEAR);

    board[13][1].block_north = true;

    board[14][0].block_north = true;
    board[14][4].block_east = true;
    board[14][10].block_east = true;
    board[14][4].target.emplace(RED, CRESCENT);
    board[14][11].target.emplace(GREEN, CRESCENT);

    board[15][4].block_north = true;
    board[15][6].block_east = true;
    board[15][11].block_north = true;
    board[15][13].block_east = true;
}

void game_state::select_starting_squares()
{
    robots[0].color = BLUE;
    robots[1].color = RED;
    robots[2].color = GREEN;
    robots[3].color = YELLOW;

    std::uniform_int_distribution<unsigned> row_dis(0, k_board_width - 1);
    std::uniform_int_distribution<unsigned> col_dis(0, k_board_height - 1);

    for (size_t i = 0; i < 4; ++i) {
        while (true) {
            uint8_t row = row_dis(rng);
            uint8_t col = col_dis(rng);

            square & sq = board[row][col];
            if (!sq.target && sq.robot_idx == -1 && sq.allowable_starting_square) {
                robots[i].row = row;
                robots[i].col = col;
                sq.robot_idx = i;
                break;
            }
        }
    }
}

game_state::game_state()
{
    init_board();
    select_starting_squares();
}

void game_state::draw()
{
    std::string rows[k_board_height * 2];
    for (auto & row : rows) {
        row.resize(k_board_width * 3 + 1, ' ');
        row.back() = '\n';
    }

    for (unsigned row = 0; row < k_board_height; ++row) {
        for (unsigned col = 0; col < k_board_width; ++col) {
            square const & sq = board[row][col];

            if (sq.block_north) {
                rows[row * 2][col * 3] = '_';
                rows[row * 2][col * 3 + 1] = '_';
            }
            if (sq.block_east) {
                rows[row * 2 + 1][col * 3 + 2] = '|';
            }

            if (sq.robot_idx != -1) {
                robot * r = &robots[sq.robot_idx];
                char c = std::toupper(color_to_char(r->color));
                rows[row * 2 + 1][col * 3] = c;
                rows[row * 2 + 1][col * 3 + 1] = c;
            } else if (sq.target) {
                rows[row * 2 + 1][col * 3] = color_to_char(sq.target->color);
                rows[row * 2 + 1][col * 3 + 1] = shape_to_char(sq.target->shape);
            } else if (sq.allowable_starting_square) {
                rows[row * 2 + 1][col * 3] = '.';
            }
        }
    }

    for (auto & row : rows) {
        printf("%s", row.c_str());
    }
}

bool game_state::can_move(robot * r, direction_t dir)
{
    switch (dir) {
    case UP:
        return r->row > 0
            && !board[r->row][r->col].block_north
            && board[r->row - 1][r->col].robot_idx == -1;
    case DOWN:
        return r->row < k_board_height - 1
            && !board[r->row + 1][r->col].block_north
            && board[r->row + 1][r->col].robot_idx == -1;
    case LEFT:
        return r->col > 0
            && !board[r->row][r->col - 1].block_east
            && board[r->row][r->col - 1].robot_idx == -1;
    case RIGHT:
        return r->col < k_board_width - 1
            && !board[r->row][r->col].block_east
            && board[r->row][r->col + 1].robot_idx == -1;
    }
}

void game_state::move_single(robot * r, direction_t dir)
{
    int8_t r_idx = r - robots;

    int delta_row = 0;
    int delta_col = 0;
    switch (dir) {
    case UP:
        delta_row = -1;
        break;
    case DOWN:
        delta_row = 1;
        break;
    case LEFT:
        delta_col = -1;
        break;
    case RIGHT:
        delta_col = 1;
        break;
    }

    assert(board[r->row][r->col].robot_idx == r_idx);
    board[r->row][r->col].robot_idx = -1;
    r->row += delta_row;
    r->col += delta_col;
    board[r->row][r->col].robot_idx = r_idx;
}

void game_state::move_robot(robot * r, direction_t dir)
{
    while (can_move(r, dir)) {
        move_single(r, dir);
    }
}

// from chatgpt
void set_raw_mode(int fd)
{
    struct termios term;
    tcgetattr(fd, &term); // Get current terminal settings
    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echoing
    term.c_cc[VMIN] = 1; // Minimum characters to read
    term.c_cc[VTIME] = 0; // Timeout
    tcsetattr(fd, TCSAFLUSH, &term); // Set terminal settings
}

// from chatgpt
void reset_mode()
{
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); // Restore canonical mode and echoing
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &term);
}

int main()
{
    set_raw_mode(STDOUT_FILENO);
    atexit(reset_mode);

    game_state game;
    game.draw();

    robot * robot_to_move = &game.robots[0];

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == 27) { // Escape character (for arrow keys)
            ch = getchar(); // Should be '['
            if (ch == '[') {
                ch = getchar();
                direction_t dir;
                switch (ch) {
                case 'A':
                    dir = UP;
                    break;
                case 'B':
                    dir = DOWN;
                    break;
                case 'C':
                    dir = RIGHT;
                    break;
                case 'D':
                    dir = LEFT;
                    break;
                default:
                    printf("Unknown arrow key\n");
                    exit(1);
                }

                printf("\n\nmove %s\n\n", dir_to_str(dir));

                game.move_robot(robot_to_move, dir);
                game.draw();
            }
        }
    }
}
