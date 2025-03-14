#include "player.h"

#include <chrono>
#include <iostream>


std::array<int, 4> Human_player::choose_move(const Board& board,
                                              Cell_state player) {
  int choice;
  bool valid_choice = false;
  std::vector<std::array<int, 4>> all_moves = board.get_valid_moves(player);
  board.print_valid_moves(all_moves);

 
  while (!valid_choice) {
      std::cout << "\n Choose one move among the given above: ";
      if (!(std::cin >> choice)) {
          std::cout << "Invalid input! Try again." << std::endl;
          std::cin.clear();  // clear error flags
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
              '\n');  // ignore the rest of the line
          continue;
      }
      else {
          if (choice < 1 || choice > all_moves.size()) {
              std::cout << "Invalid choice! Try again." << std::endl;
              continue;
          }
          else {
              return all_moves[choice-1];
          }
      }

  }

  return { -1, -1, -1, -1 };  // should never reach this
}
