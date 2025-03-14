#include "game.h"

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
    std::cout << "\nPlayer " << current_player_index + 1 << " chose move: " << chosen_x
              << " " << chosen_y << " " << chosen_dir << " " << chosen_tar << std::endl;
    board.make_move(chosen_x, chosen_y, chosen_dir, chosen_tar, current_player);
    if (chosen_move[3] < 1) {
        switch_player();
        board.clear_state();
    }
    
  }
  board.display_board(std::cout);
  Cell_state winning_player =
      (current_player_index == 0) ? Cell_state::O : Cell_state::X;
  std::cout << "Player " << winning_player << " wins!" << std::endl;
}

void Game::switch_player() {
    current_player_index = 1 - current_player_index;
    

}
