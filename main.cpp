#include <cstdio>
#include <string>

struct square
{
    bool block_north = false;
    bool block_east = false;
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

    board[2][5].block_north = true;
    board[2][7].block_east = true;
    board[2][11].block_east = true;

    board[3][7].block_north = true;
    board[3][11].block_north = true;
    board[3][13].block_north = true;
    board[3][13].block_east = true;

    board[4][0].block_north = true;
    board[4][3].block_east = true;
    board[4][9].block_east = true;

    board[5][3].block_north = true;
    board[5][6].block_east = true;
    board[5][7].block_north = true;
    board[5][10].block_north = true;
    board[5][11].block_east = true;
    board[5][12].block_north = true;

    board[6][1].block_north = true;
    board[6][1].block_east = true;
    board[6][15].block_north = true;

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

    board[10][10].block_east = true;
    board[10][15].block_north = true;

    board[11][5].block_east = true;
    board[11][6].block_north = true;
    board[11][10].block_north = true;

    board[12][0].block_east = true;
    board[12][14].block_east = true;
    board[12][14].block_north = true;

    board[13][1].block_north = true;

    board[14][0].block_north = true;
    board[14][4].block_east = true;
    board[14][10].block_east = true;

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

            rows[row * 2 + 1][col * 3] = '.';
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
