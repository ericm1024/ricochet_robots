#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <random>
#include <string>
#include <unordered_set>

#include <unistd.h>
#include <termios.h>

enum class color_t : uint8_t
{
    BLUE,
    RED,
    GREEN,
    YELLOW,
    RAINBOW,
    INVALID_COLOR,
};
using enum color_t;

static char to_char(color_t c)
{
    switch (c) {
    case BLUE: return 'b';
    case RED: return 'r';
    case GREEN: return 'g';
    case YELLOW: return 'y';
    case RAINBOW: return 'r';
    case INVALID_COLOR: assert(false); return 'i';
    }
}

static char const * to_str(color_t c)
{
    switch (c) {
    case BLUE: return "blue";
    case RED: return "red";
    case GREEN: return "green";
    case YELLOW: return "yellow";
    case RAINBOW: return "rainbow";
    case INVALID_COLOR: assert(false); return "invalid_color";
    }
}

enum class shape_t : uint8_t
{
    CRESCENT,
    GEAR,
    PLANET,
    STAR,
    HOLE,
    INVALID_SHAPE,
};
using enum shape_t;

static char to_char(shape_t s)
{
    switch (s) {
    case CRESCENT: return 'c';
    case GEAR: return 'g';
    case PLANET: return 'p';
    case STAR: return 's';
    case HOLE: return 'h';
    case INVALID_SHAPE: assert(false); return 'i';
    }
}

enum class direction_t : uint8_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};
using enum direction_t;

static char const * to_str(direction_t dir)
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
    target() = default;
    target(color_t c, shape_t s) : color{c}, shape{s} {}

    bool operator==(target const &) const = default;

    color_t color = INVALID_COLOR;
    shape_t shape = INVALID_SHAPE;
};

namespace std
{
    template <> struct hash<target>
    {
        size_t operator()(target const & t) const
        {
            return static_cast<size_t>(t.color) | (static_cast<size_t>(t.shape) << 8);
        }
    };
}

struct robot
{
    bool operator==(robot const &) const = default;

    color_t color;
    uint8_t row;
    uint8_t col;
};

static constexpr size_t k_num_robots = 4;

using robot_array = std::array<robot, k_num_robots>;
static_assert(sizeof(robot_array) == 12);

// https://stackoverflow.com/a/7666577/3775803
static size_t hash(unsigned char const * str, size_t len)
{
    unsigned char const * end = str + len;
    size_t hash = 5381;

    while (str != end) {
        hash = ((hash << 5) + hash) + *str++; /* hash * 33 + c */
    }

    return hash;
}

namespace std
{
    template <> struct hash<robot_array>
    {
        size_t operator()(robot_array const & r) const
        {
            static_assert(std::has_unique_object_representations_v<robot_array>);
            return ::hash(reinterpret_cast<unsigned char const *>(r.data()), sizeof(r));
        }
    };
}

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

struct move
{
    move() = default;
    move(uint8_t i, direction_t d) : robot_idx{i}, dir{d} {}

    uint8_t robot_idx;
    direction_t dir;
};

struct moves_vec
{
    moves_vec() = default;

    template <typename ... Ts>
    void emplace_back(Ts && ... args)
    {
        assert(count < std::size(moves));
        moves[count++] = move(std::forward<Ts>(args)...);
    }

    void clear()
    {
        count = 0;
    }

    bool empty() const
    {
        return count == 0;
    }

    size_t size() const
    {
        return count;
    }

    move const * begin() const { return moves; }
    move const * end() const { return moves + count; }

private:
    move moves[32];
    size_t count = 0;
};

struct game_state
{
    game_state();
    game_state(game_state const &) = default;

    void draw();
    void move_robot(robot * r, direction_t dir);

    void valid_moves(moves_vec & vec) const;

    bool turn_over() const;

    void play(move mv)
    {
        move_robot(&robots[mv.robot_idx], mv.dir);
    }

    void select_target(std::unordered_set<target> * targets_used = nullptr);

private:
    void init_board();
    void select_starting_squares();

    bool can_move(robot const * r, direction_t dir) const;
    void move_single(robot * r, direction_t dir);

    // upper left is 0, 0. First coordinate is row, second is column
    square board[k_board_height][k_board_width];

public:
    robot_array robots;

    target target_square;
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

    for (size_t i = 0; i < k_num_robots; ++i) {
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

void game_state::select_target(std::unordered_set<target> * targets_used)
{
    std::uniform_int_distribution<unsigned> row_dis(0, k_board_width - 1);
    std::uniform_int_distribution<unsigned> col_dis(0, k_board_height - 1);
    while (true) {
        uint8_t row = row_dis(rng);
        uint8_t col = col_dis(rng);

        square & sq = board[row][col];
        if (sq.target) {
            target tg = *sq.target;
            if (!targets_used || !targets_used->contains(tg)) {
                target_square = tg;
                if (targets_used) {
                    targets_used->insert(tg);
                }
                break;
            }
        }
    }
}

game_state::game_state()
{
    init_board();
    select_starting_squares();
    select_target();
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
                char c = std::toupper(to_char(r->color));
                rows[row * 2 + 1][col * 3] = c;
                rows[row * 2 + 1][col * 3 + 1] = c;
            } else if (sq.target && *sq.target == target_square) {
                rows[row * 2 + 1][col * 3] = to_char(sq.target->color);
                rows[row * 2 + 1][col * 3 + 1] = to_char(sq.target->shape);
            } else if (sq.allowable_starting_square) {
                rows[row * 2 + 1][col * 3] = '.';
            }
        }
    }

    for (auto & row : rows) {
        printf("%s", row.c_str());
    }
}

bool game_state::can_move(robot const * r, direction_t dir) const
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
    int8_t r_idx = r - robots.data();

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

void game_state::valid_moves(moves_vec & vec) const
{
    vec.clear();

    for (uint8_t i = 0; i < k_num_robots; ++i) {
        robot const * r = &robots[i];
        for (direction_t d : {UP, DOWN, LEFT, RIGHT}) {
            if (can_move(r, d)) {
                vec.emplace_back(i, d);
            }
        }
    }

    assert(!vec.empty());
}

bool game_state::turn_over() const
{
    for (robot const & r : robots) {
        square const & sq = board[r.row][r.col];
        if (sq.target && *sq.target == target_square &&
            (sq.target->color == r.color || sq.target->color == RAINBOW)) {
            return true;
        }
    }
    return false;
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

static void play_game()
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

                printf("\n\nmove %s\n\n", to_str(dir));

                game.move_robot(robot_to_move, dir);
                game.draw();
            }
        }
    }
}

struct solutions
{
    void add(moves_vec solution)
    {
        if (solution.size() < move_count) {
            move_count = solution.size();
            options.clear();
        }
        options.push_back(solution);
    }

    size_t move_count = std::numeric_limits<size_t>::max();
    std::vector<moves_vec> options;
};

static void do_solve(game_state const & game,
                     std::unordered_map<robot_array, size_t> & states_achieved,
                     moves_vec const & current_moves, solutions & sols)
{
    // can't improve down this route.
    if (current_moves.size() == sols.move_count) {
        return;
    }

    // probably too deep?
    if (current_moves.size() > 15) {
        return;
    }

    assert(current_moves.size() < sols.move_count);

    moves_vec moves;
    game.valid_moves(moves);

    bool solution_found = false;
    for (move mv : moves) {
        game_state copy{game};
        copy.play(mv);
        if (copy.turn_over()) {
            moves_vec solution = current_moves;
            solution.emplace_back(mv);

            sols.add(solution);
            solution_found = true;

            //printf("solution of size %zu found\n", solution.size());
        }
    }

    if (solution_found) {
        return;
    }

    for (move mv : moves) {
        game_state copy{game};
        copy.play(mv);
        size_t moves_used = current_moves.size() + 1;
        auto [it, did_insert] = states_achieved.emplace(copy.robots, moves_used);
        if (did_insert || it->second > moves_used) {
            it->second = moves_used;

            moves_vec next_moves = current_moves;
            next_moves.emplace_back(mv);
            do_solve(copy, states_achieved, next_moves, sols);
        }
    }
}

static void solve()
{
    game_state game;

    std::unordered_set<target> targets_used;
    targets_used.insert(game.target_square);

    while (true) {
        game.draw();

        std::unordered_map<robot_array, size_t> states_achieved;
        states_achieved.emplace(game.robots, 0);

        moves_vec current_moves;
        solutions sols;
        do_solve(game, states_achieved, current_moves, sols);

        assert(sols.options.size() > 0);

        printf("found %zu solution%s in %zu moves\n",
               sols.options.size(), sols.options.size() > 1 ? "s" : "", sols.move_count);

        int i_sol = 0;
        for (moves_vec const & moves : sols.options) {
            printf("solution %d\n", ++i_sol);
            for (move const & mv : moves) {
                printf("%s moves %s\n", to_str(game.robots[mv.robot_idx].color), to_str(mv.dir));
            }
            printf("\n");
        }

        int input = 0;
        while (true) {
            printf("select solution: ");
            input = getchar();
            if (input == EOF) {
                exit(0);
            }
            input = (input - '0') - 1;
            if (input < 0 || input >= (int)std::size(sols.options)) {
                printf("\ninvalid selection\n");
            } else {
                break;
            }
        }

        for (move const & mv : sols.options[input]) {
            game.play(mv);
        }

        game.select_target(&targets_used);
        printf("%zu targets used, selected target %c%c\n", targets_used.size(), to_char(game.target_square.color), to_char(game.target_square.shape));
    }
}

static void usage(char ** argv)
{
    fprintf(stderr, "usage: %s [play|solve]\n", argv[0]);
    exit(1);
}

int main(int argc, char ** argv)
{
    // seed 3945310130, eventually gets to 6 solutions in 7 moves
    //
    // fails to find green up, left, blue up, left, down, right, up

    unsigned seed = 3945310130;//std::random_device{}();
    printf("seed is %u\n", seed);
    rng.seed(seed);

    if (argc != 2) {
        usage(argv);
    } else if (strcmp(argv[1], "play") == 0) {
        play_game();
    } else if (strcmp(argv[1], "solve") == 0) {
        solve();
    } else {
        fprintf(stderr, "unknown arg %s\n", argv[1]);
        usage(argv);
    }
}
