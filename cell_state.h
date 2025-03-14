#ifndef CELL_STATE_H
#define CELL_STATE_H

#include <ostream>

/*
 @enum Cell_state
 @brief Represents the state of a cell in the game.
 
 A cell in a the game can be in one of three states:
 Empty, X, or O.
 
 Enumeration values:
 @value Empty: The cell has not been claimed by any player.
 @value X: The cell has been claimed by the player 1.
 @value O: The cell has been claimed by the player 2.
 */
enum class Cell_state {
  Empty,
  X,  
  O  
};

/*
 @brief Overloaded stream insertion operator for the Cell_state.
 @return A reference to the output stream with the given state character.
 */
std::ostream& operator<<(std::ostream& os, const Cell_state& state);

#endif  // CELL_STATE_H