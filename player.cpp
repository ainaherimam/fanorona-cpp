#include "player.h"

#include <chrono>
#include <iostream>

#include "mcts_agent.h"
#include <torch/torch.h>

std::pair<std::array<int, 4>,torch::Tensor> Human_player::choose_move(const Board& board,
                                              Cell_state player) {
  int choice;
  bool valid_choice = false;
  torch::Tensor dummy_tensor = torch::zeros({1}, torch::kFloat32);
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
              return {all_moves[choice-1], dummy_tensor};
          }
      }

  }

  return {{ -1, -1, -1, -1 }, dummy_tensor};  // should never reach this
}


Mcts_player::Mcts_player(double exploration_factor,
                         int number_iteration,
                         bool is_verbose)
    : exploration_factor(exploration_factor),
      number_iteration(number_iteration),
      is_verbose(is_verbose) {}

std::pair<std::array<int, 4>,torch::Tensor> Mcts_player::choose_move(const Board& board,
                                             Cell_state player) {

  Mcts_agent agent(exploration_factor, number_iteration, is_verbose);

  return agent.choose_move(board, player);
}

bool Mcts_player::get_is_verbose() const { return is_verbose; }