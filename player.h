#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>
#include <utility>

#include "board.h"

/*
 @brief The Player class is an abstract base class for all game player types

 A Player's primary responsibility is to choose a move based on the current
 state of the game board. This interaction is modelled via function choose_move().
 */
class Player {
 public:
  /*
   @brief Abstract function for choosing a move on the game board.
   @param board Current state of the board (Board).
   @param player The cell of the current player (Cell_state).
   @return The chosen move as an array of integers.
  */
  virtual std::array<int, 4> choose_move(const Board& board,
                                          Cell_state player) = 0;
};

/*
 @brief The Human_player class is a concrete class that inherits from Player,
 implementing the choose_move() function to allow user input for move selection,
 with validation to ensure the move is legitimate.
 */
class Human_player : public Player {
 public:
  /*
   @brief Implementation of the choose_move function for the Human_player
   class. This function prompts the user to input a row and column for their
   move. It checks if the move is valid and, if not, prompts the user to try
   again.

   @param board Current state of the board (Board).
   @param player The cell of the current player (Cell_state).
   @return The chosen move as a pair of integers, row first, column second.
  */
  std::array<int, 4> choose_move(const Board& board,
                                  Cell_state player) override;
};


/*
 @brief Mcts_player is a concrete class derived from the Player base class,
 embodying a player that utilizes the MCTS algorithm
 for decision making during a game.

 The choose_move() function selects the best move based on MCTS simulations,
 considering an exploration factor, max decision time, and verbose logging.
 These parameters are customizable upon instantiation.
*/
class Mcts_player : public Player {
public:
     /*
      @brief Mcts_player constructor initializes MCTS parameters.
      @param exploration_factor Exploration factor for MCTS.
      @param max_decision_time Max time for decision making.
      @param is_verbose Enables verbose logging if true.
     */
    Mcts_player(double exploration_factor,
        std::chrono::milliseconds max_decision_time,
        bool is_verbose = false);

    /*
     @brief Implementation of the choose_move function for the Mcts_player
     class.
     
     @param board The current state of the game board.
     @param player The current player.
     @return The chosen move as a pair of integers.
     */
    std::array<int, 4> choose_move(const Board& board,
        Cell_state player) override;

    /*
     @brief Getter for the is_verbose private member of the Mcts_player class.
     
     @return The value of is_verbose.
     */
    bool get_is_verbose() const;

private:
    double exploration_factor;  // The exploration factor used in MCTS.
    std::chrono::milliseconds max_decision_time;  // Maximum decision-making time.
    bool is_verbose;       // If true, enables verbose logging to console.
};

#endif
