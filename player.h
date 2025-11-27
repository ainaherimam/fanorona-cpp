#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>
#include <utility>

#include "board.h"
#include "logger.h"
#include <torch/torch.h>

/**
 * @brief The Player class is an abstract base class for all game player types
 *
 * A Player's primary responsibility is to choose a move based on the current
 * state of the game board. This interaction is modelled via function choose_move().
 */
class Player {
 public:
  /**
   * @brief Abstract function for choosing a move on the game board
   *
   * @param board Current state of the board
   * @param player The cell of the current player
   *
   * @return The chosen move as an array of integers and policy tensor for data collection
   */
  virtual std::pair<std::array<int, 4>, torch::Tensor> choose_move(
      const Board& board,
      Cell_state player) = 0;
};

/**
 * @brief The Human_player class is a concrete class that inherits from Player,
 * implementing the choose_move() function to allow user input for move selection,
 * with validation to ensure the move is legitimate
 */
class Human_player : public Player {
 public:
  /**
   * @brief Implementation of the choose_move function for the Human_player class
   *
   * This function prompts the user to make a choice of which move to make.
   * if the user choose a move that is not in the given list, this function prompts the user to try again.
   *
   * @param board Current state of the board
   * @param player The cell of the current player
   *
   * @return The chosen move as an array of integers and a dummy policy tensor (we don't get data from human games)
   */
  std::pair<std::array<int, 4>, torch::Tensor> choose_move(
      const Board& board,
      Cell_state player) override;
};

/**
 * @brief Mcts_player is a concrete class derived from the Player base class,
 * embodying a player that utilizes the MCTS algorithm for decision making
 * during a game
 *
 * The choose_move() function selects the best move based on MCTS,
 * considering an exploration factor, max iteration number, and level of logging.
 */
class Mcts_player : public Player {
 public:
  /**
   * @brief Mcts_player constructor initializes MCTS parameters
   *
   * @param exploration_factor Exploration factor for MCTS
   * @param number_iteration Maximum iteration number
   * @param log_level Log Level
   */
  Mcts_player(double exploration_factor,
              int number_iteration,
              LogLevel log_level = LogLevel::NONE);

  /**
   * @brief Implementation of the choose_move function for the Mcts_player class
   *
   * @param board The current state of the game board
   * @param player The current player
   *
   * @return The chosen move as an array of integers and policy tensor for data collection
   */
  std::pair<std::array<int, 4>, torch::Tensor> choose_move(
      const Board& board,
      Cell_state player) override;

  /**
   * @brief Getter for verbose level private member of the Mcts_player class
   *
   * @return The verbose level
   */
  LogLevel get_verbose_level() const;

 private:
  double exploration_factor;  // The exploration factor used in MCTS
  int number_iteration;       // The maximum number of iterations
  LogLevel log_level;            // Verbose level
};

#endif