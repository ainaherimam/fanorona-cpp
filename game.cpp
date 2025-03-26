#include "game.h"
#include "logger.h"

#include <iostream>

Game::Game(int board_size, std::unique_ptr<Player> player1,
           std::unique_ptr<Player> player_2)
    : board(board_size), current_player_index(0) {
  players[0] = std::move(player1);
  players[1] = std::move(player_2);
}

void Game::play() {
  while (board.check_winner() == Cell_state::Empty) {
    Cell_state current_player =
        current_player_index == 0 ? Cell_state::X : Cell_state::O;
    std::cout << "\nPlayer " << current_player_index + 1 << "'s turn:" << std::endl;
    board.display_board(std::cout);
    std::array<int, 4> chosen_move =
        players[current_player_index]->choose_move(board, current_player);
    int chosen_x = chosen_move[0];
    int chosen_y = chosen_move[1];
    int chosen_dir = chosen_move[2];
    int chosen_tar = chosen_move[3];
    std::cout << "\nPlayer " << current_player_index + 1 << " chose move: " << print_move(chosen_move) << std::endl;
    board.make_move(chosen_x, chosen_y, chosen_dir, chosen_tar, current_player);
    if (chosen_move[3] < 1) {
        switch_player();
        board.clear_state();
    }
    
  }
  board.display_board(std::cout);
  Cell_state winning_player = board.check_winner();
  std::cout << "Player " << winning_player << " wins!" << std::endl;
}
std::string Game::print_move(std::array<int, 4> move) {

    // Print the row as a number and the column as an alphabet
    char column = 'a' + move[1];
    int row = move[0] + 1;

    // Determine direction based on the dir value
    std::string direction;
    switch (move[2]) {
    case 1: direction = "Move downleft "; break;
    case 2: direction = "Move down"; break;
    case 3: direction = "Move downright"; break;
    case 4: direction = "Move left"; break;
    case 5: direction = "Stay"; break;
    case 6: direction = "Move right"; break;
    case 7: direction = "Moveupleft"; break;
    case 8: direction = "Move up"; break;
    case 9: direction = "Move upright"; break;
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

    std::string message = "(" + std::to_string(row) + ", " + column + ") "
        + direction + " " + move_type;

    return message;
}

void Game::switch_player() {
    current_player_index = 1 - current_player_index;
    

}
