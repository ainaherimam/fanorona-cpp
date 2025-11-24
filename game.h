#ifndef GAME_H
#define GAME_H

#include <memory>

#include "board.h"
#include "cell_state.h"
#include "player.h"
#include "nn_model.h"

/*
 @class Game
 @brief Represents a Fanorona game.
 
 This class define a complete Fanorona game, including the game board and the
 two players. It handles the game loop, player turns, and game state
 transitions.
 */
class Game {
 public:
  /*
   @brief Game object.
   
   Initializes a new game with a specified board size and two
   players.
   
   @param board_size The size of the game board.
   @param player1  pointer to the first player.
   @param player2  pointer to the second player.
   */
  Game(int board_size, std::unique_ptr<Player> player_1,
       std::unique_ptr<Player> player_2,
       GameDataset& dataset);
  /*
   @brief Give the correct string output for a move represented by an array of integers.
  */
  std::string print_move(std::array<int, 4> moves);
  /*
   @brief Starts and manages the game.
   
   This function contains the main game loop. It continues until a player
   wins.

   Once a player wins, it displays the final state of the board and the
   winner.
   */
  Cell_state play();

 private:
  Board board; 
  std::unique_ptr<Player>
      players[2];           
  int current_player_index;
  std::vector<torch::Tensor> result_logits;
  std::vector<torch::Tensor> result_states;
  std::vector<torch::Tensor> result_z;\
  std::vector<torch::Tensor> result_mask;

  GameDataset& dataset_;
  /*
   @brief Switches the current player.
   
   This function switches the turn to the other player.
   */
  void switch_player();
};

#endif