#include "board.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "iterator"



Board::Board(int size)
    : board_size(size),
    board(size, std::vector<Cell_state>(size, Cell_state::Empty)) {
    if (size < 2) {
        throw std::invalid_argument("Board size cannot be less than 2.");
    }
    if (size == 5) {
        // Initialize the board with the specific pattern
        for (size_t row = 0; row < 5; ++row) {
            for (size_t col = 0; col < static_cast<size_t>(size); ++col) {
                if (row <= 1) {
                    board[row][col] = Cell_state::X;
                }
                else if (row >= 3) {
                    board[row][col] = Cell_state::O;
                }
            }
        }
        board[2][0] = Cell_state::O;
        board[2][1] = Cell_state::X;
        board[2][2] = Cell_state::Empty;
        board[2][3] = Cell_state::O;
        board[2][4] = Cell_state::X;
    }
    else if (size == 9) {
        for (size_t row = 0; row < 5; ++row) {
            for (size_t col = 0; col < static_cast<size_t>(size); ++col) {
                if (row <= 1) {
                    board[row][col] = Cell_state::X;
                }
                else if (row >= 3) {
                    board[row][col] = Cell_state::O;
                }
            }
        }
        board[2][0] = Cell_state::O;
        board[2][1] = Cell_state::X;
        board[2][2] = Cell_state::O;
        board[2][3] = Cell_state::X;
        board[2][4] = Cell_state::Empty;
        board[2][5] = Cell_state::O;
        board[2][6] = Cell_state::X;
        board[2][7] = Cell_state::O;
        board[2][8] = Cell_state::X;
    }

}


int Board::get_board_size() const { return board_size; }

bool Board::is_within_bounds(int move_x, int move_y) const {
  return move_x >= 0  && move_x < 5 && move_y >= 0 && move_y < board_size;
}

bool Board::is_in_vector(int x, int y, const std::vector<std::array<int, 2>>& vector) const {
    return std::find(vector.begin(), vector.end(), std::array<int, 2>{x, y}) != vector.end();
}

std::vector<int> Board::all_direction(int x, int y) const {
    
    static const std::vector<int> all_dir = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    static const std::vector<int> no_diag = { 2, 4, 6, 8, 5 };
    static const std::vector<int> all_dir_no = { 1, 2, 3, 4, 6, 7, 8, 9 };
    static const std::vector<int> no_diag_no = { 2, 4, 8, 6 };

    const bool diag_not_available = (x + y) % 2 == 1;
    const bool after_capturing_move = !path.empty();

    return diag_not_available ? (after_capturing_move ? no_diag : no_diag_no)
        : (after_capturing_move ? all_dir : all_dir_no);
}


bool has_taking_moves(const std::vector<std::array<int, 4>>& valid_moves) {
    return std::any_of(valid_moves.begin(), valid_moves.end(), [](const std::array<int, 4>& move) {
        return move[3] > 0;
        });
}

void remove_on_vector(std::vector<std::array<int, 4>>& valid_moves, int threshold) {
    valid_moves.erase(
        std::remove_if(valid_moves.begin(), valid_moves.end(),
            [threshold](const std::array<int, 4>& move) {
                return move[3] < threshold;
            }),
        valid_moves.end()
    );
}
void Board::print_valid_moves(std::vector<std::array<int, 4>> moves) const {
    int index = 1;
    for (const auto& move : moves) {
        // Print the row as a number and the column as an alphabet
        char column = 'a' + move[1];  // assuming move[1] is the column index (0-based)
        int row = move[0] + 1;  // assuming move[0] is the row index (0-based)

        // Determine direction based on the provided value
        std::string direction;
        switch (move[2]) {
        case 1: direction = "Move downleft "; break;
        case 2: direction = "Move down     "; break;
        case 3: direction = "Move downright"; break;
        case 4: direction = "Move left     "; break;
        case 5: direction = "Stay     "; break;
        case 6: direction = "Move right    "; break;
        case 7: direction = "Moveupleft   "; break;
        case 8: direction = "Move up       "; break;
        case 9: direction = "Move upright  "; break;
        default: direction = "unknown"; break;
        }

        // Determine move type
        std::string move_type;
        switch (move[3]) {
        case -1: move_type = ""; break;
        case 0: move_type = ""; break;
        case 1: move_type = "and take backward"; break;
        case 2: move_type = "and take forward"; break;
        default: move_type = "unknown"; break;
        }

        std::cout << index << "-"
            << "From (" << row << ", " << column << ") "
            << direction << " "
            << move_type << "\n";
        index++;
    }
}


std::vector<std::array<int, 4>> Board::get_valid_moves(Cell_state player) const {

    std::vector<std::array<int, 4>> valid_moves;
    std::vector<std::array<int, 2>> valid_positions;


    if (!path.empty()) {
        valid_positions.emplace_back(path.back());
    }
    else {
        if (get_board_size() == 5) {
            valid_positions = coordinates;
        }
        else {
            valid_positions = coordinates_9;
        }
        
    }

    for (const auto&[x,y] : valid_positions) {

        if (board[x][y] != player) {
            continue; 
        }

        for (const auto& dir : all_direction(x, y)) {
            if (dir == 5) {
                valid_moves.emplace_back(std::array<int, 4>{x, y, dir, 0});
                continue;
            }

            int dest_x = x + offset_x[dir], dest_y = y + offset_y[dir];

            if (!is_within_bounds(dest_x, dest_y) ||
               (std::array<int, 2> {dest_x, dest_y} == restricted_move) ||
                is_in_vector(dest_x, dest_y, path) ||
                board[dest_x][dest_y] != Cell_state::Empty)
                continue;

            int tarf_x = dest_x + offset_x[dir], tarf_y = dest_y + offset_y[dir];
            int tarb_x = dest_x - 2 * offset_x[dir], tarb_y = dest_y - 2 * offset_y[dir];

            bool took = false;

            if (is_within_bounds(tarf_x, tarf_y) &&
                board[tarf_x][tarf_y] != Cell_state::Empty &&
                board[tarf_x][tarf_y] != player) {
                valid_moves.emplace_back(std::array<int, 4>{x, y, dir, 2});
                took = true;
            }
            if (is_within_bounds(tarb_x, tarb_y) &&
                board[tarb_x][tarb_y] != Cell_state::Empty &&
                board[tarb_x][tarb_y] != player) {
                valid_moves.emplace_back(std::array<int, 4>{x, y, dir, 1});
                took = true;
            }
            if (!took) {
                valid_moves.emplace_back(std::array<int, 4>{x, y, dir, -1});
            }
            
        }
    }

    // Process taking moves based on `path`
    if (path.empty()) {
        if (has_taking_moves(valid_moves)) {
            remove_on_vector(valid_moves, 1);
        }
    }
    else {
        remove_on_vector(valid_moves, 0);
    }

    return valid_moves;
}

void Board::make_move(int move_x, int move_y, int dir, int tar, Cell_state player) {
  // Check if the move is valid. If not, throw an exception.
    int dest_x = move_x + offset_x[dir];
    int dest_y = move_y + offset_y[dir];

    board[move_x][move_y] = Cell_state::Empty;
    board[dest_x][dest_y] = player;
    // If the move is valid, place the player's Cell_state on the board at the
    // specified coordinates.
    if (tar > 0) {
        if (!path.empty()) {
            int restr_x = move_x + 2 * offset_x[dir];
            int restr_y = move_y + 2 * offset_y[dir];
            restricted_move =  {restr_x, restr_y};
            path.emplace_back(std::array<int, 2>{dest_x, dest_y});

        }
        else {
            path.emplace_back(std::array<int, 2>{move_x, move_y});
            path.emplace_back(std::array<int, 2>{dest_x, dest_y});
        }
        take(move_x, move_y, dir, tar, player);
    }

}

void Board::take(int move_x, int move_y, int dir, int tar, Cell_state player) {
    int mult;
    int tar_row, tar_col;

    if (tar == 2) {
        mult = 1;
        tar_row = move_x + 2 * offset_x[dir];
        tar_col = move_y + 2 * offset_y[dir];
    }
    else {
        mult = -1;
        tar_row = move_x - offset_x[dir];
        tar_col = move_y - offset_y[dir];
    }

    // Loop to remove all targets
    while (is_within_bounds(tar_row, tar_col) && board[tar_row][tar_col] != player && board[tar_row][tar_col] != Cell_state::Empty) {

        board[tar_row][tar_col] = Cell_state::Empty;
        tar_row += mult * offset_x[dir];
        tar_col += mult * offset_y[dir];
    }
}


Cell_state Board::check_winner() const {

    bool hasO = false;
    bool hasX = false;

    // Loop through each cell in the 5x5 board
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell == Cell_state::O) {
                hasO = true;
            }
            else if (cell == Cell_state::X) {
                hasX = true;
            }
        }
    }

    // Return the missing color type
    if (!hasX) {
        return Cell_state::O;
    }
    else if (!hasO) {
        return Cell_state::X;
    }

    return Cell_state::Empty;
}

void Board::display_board(std::ostream& os = std::cout) const {
    os << "\n";

    // Loop through each cell in the board.
    for (size_t row = 0; row < 5; ++row) {
        for (size_t col = 0; col < static_cast<std::size_t>(board_size); ++col) {
            // Print the state of the cell.
            os << board[row][col] << " ";
        }
        // Print the row number at the end of each row.
        os << row + 1;
        os << "\n";
    }

    // Print the bottom coordinate labels
    for (size_t col = 0; col < static_cast<std::size_t>(board_size); ++col) {
        os << static_cast<char>('a' + col) << " ";
    }
    os << "\n\n";
}

std::ostream& operator<<(std::ostream& os, const Board& board) {
  // Call the display_board() function to print the board to the output stream.
  board.display_board(os);
  return os;
}
