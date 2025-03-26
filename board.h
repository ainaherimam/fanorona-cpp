#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "cell_state.h"

/*
 Fanorona is a centuries-old strategy board game originating from Madagascar. 
 It is played on a 9x5 grid (fanorona 9) or a 5x5 grid (fanorona 5) 
 and challenges players to capture their opponent's pieces using a variety 
 of strategic moves. 

 The game is renowned for its depth and complexity, making it an excellent candidate for AI research.

 The Board class provides functionality for managing and interacting with the
 game board, including:
   - Initializing the board.
   - Displaying the board in the console.
   - Checking if a cell is within the bounds of the board.
   - Compute all valid moves
   - Making a move on the board.
   - Checking if there is a winner.
 
 The Board class also overloads the << operator to enable printing the board
 directly to an output stream.
 
 The board is represented internally as a 2D vector of Cell_state. The
 Cell_state enum represents the state of a cell on the board (empty, occupied
 by player 1, or occupied by player 2).

*/

class Board {

 public:
  /*
   @brief Constructor for Board class.
   @param size: Integer to set the size of the board.
   */
  Board(int size);

  /*
   @brief Clear paths and all restricted move from previous turn.
  */
  void clear_state() {
      path.clear();
      restricted_move = {-1,-1};
  }
  /*
   @brief Getter for the size of the board.
   
   @return The size of the board.
   */
  int get_board_size() const;

  /*
    @brief Checks if a given cell, identified by its x and y coordinates, is
    within the bounds of the board.
    @param move_x: The x-coordinate of the cell.
    @param move_y: The y-coordinate of the cell.
    @return True if the cell is within the bounds of the board, else False.
   */
  bool is_within_bounds(int move_x, int move_y) const;

  /*
   @brief Print out a given a vector of valid moves
   @param moves: a vector of valid moves
  */
  void print_valid_moves(std::vector<std::array<int, 4>> moves) const;

  /*
   @brief Remove all target of a given move
   @param move_x: The x-coordinate of the cell.
   @param move_y: The y-coordinate of the cell.
   @param dir: The direction of the move
   @param tar: The chosen target
   @param player: The cell name of the current player
   */
  void take(int move_x, int move_y, int dir, int tar, Cell_state player);

  /*
   @brief Compute all direction availble from a given (x,y) cell.
   @param move_x: The x-coordinate of the cell.
   @param move_y: The y-coordinate of the cell.
   @return a vector  of directions. 
  */
  std::vector<int> all_direction(int x, int y) const;

  /*
   @brief Check if (x,y) is in the given vector
   @param move_x: The x-coordinate of the cell.
   @param move_y: The y-coordinate of the cell.
   @param vector: A vector.
   @return True if (x,y) is in the vector, otherwise False
  */
  bool is_in_vector(int x, int y, const std::vector<std::array<int, 2>>& vector) const;


  /*
   * @brief Get all valid moves available on the board for the given player.
   * @param player: The current player.
   * @return A vector containing all valid moves.
  */
  std::vector<std::array<int, 4>> get_valid_moves(Cell_state player) const;

  /*
    @brief Execute a move on the board. Updates the board state based on the specified move.

    @param move_x: The x-coordinate of the cell.
    @param move_y: The y-coordinate of the cell.
    @param dir: The direction of the move
    @param tar: The chosen target
    @param player: The current player.
  */
  void make_move(int move_x, int move_y, int dir, int tar, Cell_state player);

  /*
    @brief Checks the winner.
    @return The cell symbol of the winner
  */
  Cell_state check_winner() const;

  /*
   @brief Outputs the current state of the board to an output stream.
   @param os: The output stream to which the board state is to be printed.
   Defaults to std::cout if not provided.
   */
  void display_board(std::ostream& os) const;

  /*
   @brief Overloads the << operator for the Board class. This function allows
   the board to be directly printed to an output stream (such as std::cout) by
   calling the display_board() method of the Board class.
   @param os: The output stream to which the board state is to be printed.
   @param board: The board to be printed.
   @return The output stream with the board state appended.
   */
  friend std::ostream& operator<<(std::ostream& os, const Board& board);

 private:
  /*
   @brief The size of the board.
   */
  int board_size;

  /*
   @brief A 2D vector representing the game board. Each Cell_state
   signifies the state of a cell in the board - it can be either empty,
   or occupied by one of the two players.
  */
  std::vector<std::vector<Cell_state>> board;

  /*
   @brief An array storing the x offsets for the nine possible directions
   in the game.
   */
  std::array<int, 10> offset_x = {0, 1, 1, 1, 0, 0, 0, -1, -1, -1};

  /*
   @brief An array storing the y offsets for the nine possible directions
   in the game. 
  */
  std::array<int, 10> offset_y = { 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };

  /*
   @brief A vector of restricted moves that cannot be performed on the current board.  
  */
  std::array<int, 2> restricted_move = { -1,-1 }; // array (int, 2)

  /*
   @brief A vector containing all moves previously done by the current player
   */
  std::vector<std::array<int, 2>> path; // Vector of array (int, 2)

  /*
   @brief A vector containing possible positions on a 5x5 board.
  */
  std::vector<std::array<int, 2>> coordinates = [] {
      std::vector<std::array<int, 2>> coo;
      for (int x = 0; x <= 4; ++x)
          for (int y = 0; y <= 4; ++y)
              coo.push_back({ x, y });
      return coo;
      }();

  /*
   @brief A vector containing possible positions on a 5x9 board.
  */
  std::vector<std::array<int, 2>> coordinates_9 = [] {
      std::vector<std::array<int, 2>> coo;
      for (int x = 0; x <= 8; ++x)
          for (int y = 0; y <= 8; ++y)
              coo.push_back({ x, y });
      return coo;
      }();

 

};

#endif
