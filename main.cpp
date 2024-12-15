#include <cstdio>
#include <string>
#include <optional>

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

enum shape_t
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

struct target
{
    target(color_t c, shape_t s) : color{c}, shape{s} {}

    color_t color;
    shape_t shape;
};

struct square
{
    bool block_north = false;
    bool block_east = false;

    std::optional<target> target;
};


static constexpr size_t k_board_width = 16;
static constexpr size_t k_board_height = 16;

// upper left is 0, 0. First coordinate is row, second is column
static square board[k_board_height][k_board_width];

void init_board()
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
    board[7][8].block_north = true;
    board[7][8].block_east = true;

    board[8][6].block_east = true;
    board[8][8].block_east = true;

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


void draw()
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

            if (sq.target) {
                rows[row * 2 + 1][col * 3] = color_to_char(sq.target->color);
                rows[row * 2 + 1][col * 3 + 1] = shape_to_char(sq.target->shape);
            } else {
                rows[row * 2 + 1][col * 3] = '.';
            }
        }
    }

    for (auto & row : rows) {
        printf("%s", row.c_str());
    }
}

int main()
{
    init_board();
    draw();
}
