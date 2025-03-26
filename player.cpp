#include "player.h"

#include <chrono>
#include <iostream>

#include "mcts_agent.h"

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


Mcts_player::Mcts_player(double exploration_factor,
                         std::chrono::milliseconds max_decision_time,
                         bool is_verbose)
    : exploration_factor(exploration_factor),
      max_decision_time(max_decision_time),
      is_verbose(is_verbose) {}

std::array<int, 4> Mcts_player::choose_move(const Board& board,
                                             Cell_state player) {

  Mcts_agent agent(exploration_factor, max_decision_time, is_verbose);

  return agent.choose_move(board, player);
}

bool Mcts_player::get_is_verbose() const { return is_verbose; }