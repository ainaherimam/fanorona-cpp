#ifndef MCTS_AGENT_H
#define MCTS_AGENT_H

#include <chrono>
#include <memory>
#include <mutex>
#include <random>
#include <vector>

#include "board.h"
#include "logger.h"

/*
 @class Mcts_agent
 @brief Implements a Monte Carlo Tree Search (MCTS) agent for decision-making in games.

 Simulates gameplay to determine the best move within a time limit, balancing exploration
 and exploitation using the UCT formula. Supports optional logging.
 
 @note Assumes a `Board` class with `get_valid_moves()`, `make_move()`, and `check_winner()`
 methods, and a `Cell_state` enum with `Empty`, `X`, and `O`.

 @param exploration_factor Controls exploration vs exploitation in UCT.
 @param max_decision_time Time limit for move selection (ms).
 @param is_verbose Enables detailed logging.
 
 */

class Mcts_agent {
 public:
  /*
   @brief Constructs a new Mcts_agent.
   
   @param exploration_factor  The constant of exploration in
   the UCT formula.
   @param max_decision_time Maximum time allowed for making a decision (milliseconds)
   @param is_verbose If true, enables logging to the console
   using the Logger class.
 
   */
  Mcts_agent(double exploration_factor,
             std::chrono::milliseconds max_decision_time,
             bool is_verbose = false);

  /*
   Selects the best move using Monte Carlo Tree Search (MCTS).
   
   Simulates multiple games from the current state, updating node statistics to
   identify the optimal move. The process runs until the time limit is reached,
   then selects the child with the highest win ratio.
   
   If verbose mode is enabled, logs MCTS statistics.
   
   @param board Current game state.
   @param player The player making the move.
   @return A std::pair<int, int> representing the best move.
   @throws runtime_error If insufficient simulations prevent a reliable decision.
   */

  std::array<int, 4> choose_move(const Board& board, Cell_state player);

 private:
  // Agent configuration
  double exploration_factor;
  std::chrono::milliseconds max_decision_time;
  bool is_verbose = false;

  // Logging
  std::shared_ptr<Logger> logger;

  //  Random number generation initialization
  std::random_device random_device;
  std::mt19937 random_generator;

  // Root node of the game tree
  struct Node;
  std::shared_ptr<Node> root;

  /*
   @brief Represents a node in the Monte Carlo Tree Search (MCTS) tree.
   
   Each node corresponds to a unique game state, storing both the state
   information and the progress of the search.
   */

  struct Node {
    /*
     @brief The number of wins that have been recorded through this node.
     
     This is incremented every time a simulation (or playout) that passes
     through this node results in a win.
     */
    int win_count;
    /*
     @brief The number of times this node has been visited during the search.
     
     This is incremented every time the search algorithm selects this node to
     start a new simulation (or playout).
     */
    int visit_count;
    /*
     @brief The move that led to this game state from the parent node's game
     state. For a parent node, it's filler value is {-1, -1, -1, -1}.
     
     This is represented as a array of integers, typically representing
     coordinates on a game board.
     */
    std::array<int, 4> move;
    /*
     @brief The player who made the move from the parent node's state to this
     node's state (Cell_state).
     */
    Cell_state player;
    /*
     @brief The child nodes of this node, each representing a game state that
     can be reached from this node's game state by one move.
     */
    std::vector<std::shared_ptr<Node>> child_nodes;
    /*
     @brief A pointer to the parent node of this node, representing the game
     state from which this node's game state can be reached by one move.
     */
    std::shared_ptr<Node> parent_node;
    /*
     @brief A mutex to ensure thread-safety during the updating of the node's
     data.
     
     This is especially important when the MCTS algorithm is used with
     parallel computations.
     */
    std::mutex node_mutex;

    /*
      A new Node.
     
     @param player The player making a move (Cell_state).
     @param move The move that can be made by the player. (-1, -1) if
     the node is the root node.
     @param parent_node The parent node in the tree. default: nullptr
     is used for the root node.
     */
    Node(Cell_state player, std::array<int, 4> move,
         std::shared_ptr<Node> parent_node = nullptr);
  };

  /*
   @brief Expands a node by generating all valid child nodes based on the current game state.
   
   Populates the `child_nodes` of the given node with new nodes representing valid moves.
   Each child node is linked back to the input node as its parent.
   If verbose mode is enabled, details of each new child node are printed.
   
   @param node A shared_ptr to the node to be expanded.
   @param board The current game state.
   */

  void expand_node(const std::shared_ptr<Node>& node, const Board& board);

  /*
   @brief Runs the main loop of Monte Carlo Tree Search (MCTS).

   Executes MCTS iterations until `end_time` is reached. Each iteration selects
   a child node using UCT, simulates a playout (parallel or serial), and
   backpropagates results. Logs root and child node statistics via Logger.

   @param end_time The iteration time.
   @param mcts_iteration_counter Reference to the iteration counter.
   @param board The current game board state.
  */

  void perform_mcts_iterations(
      const std::chrono::time_point<std::chrono::high_resolution_clock>&
          end_time,
      int& mcts_iteration_counter, const Board& board);

  /*
   @brief Selects the best child of a given parent node based on the Upper
   Confidence Bound for Trees (UCT) score.
   
   This function iterates through all the child nodes of the given parent
   node, and for each child, calculates its UCT score using the
   calculate_uct_score() method. The child with the highest UCT score is
   selected as the best child. If verbose mode is enabled, the function prints
   the move coordinates and the UCT score of the selected child.
   
   @param parent_node A shared_ptr to the parent Node whose child nodes are to
   be evaluated.
   @return A shared_ptr to the Node that is selected as the best child.
   */
  std::shared_ptr<Node> select_child_for_playout(
      const std::shared_ptr<Node>& parent_node);

  /**
   @brief Computes the Upper Confidence Bound for Trees (UCT) score.
  
   Used in MCTS selection to balance exploration vs. exploitation.
   The score combines the win ratio (exploitation) with an exploration
   term based on visit counts. Unvisited nodes return a high value to
   encourage exploration.

   @param child_node A shared_ptr to the child Node for which the UCT score is
   being calculated.
   @param parent_node A shared_ptr to the parent Node of the child node.
   @return The calculated UCT score.
   */
  double calculate_uct_score(const std::shared_ptr<Node>& child_node,
                             const std::shared_ptr<Node>& parent_node);

  /**
   @brief Simulates a random playout from a given node on the board.
 
   Alternates moves between players until the game ends, selecting random
   valid moves. If verbose mode is enabled, logs simulation details via Logger.
   
   @param node A shared_ptr to the Node from which the simulation starts.
   @param board The Board on which the simulation is conducted. The board
   state is copied, so the original board is not modified.
   @return The Cell_state of the winning player.
   */
  Cell_state simulate_random_playout(const std::shared_ptr<Node>& node,
                                     Board board);


  /*
   @brief Backpropagates the result of a simulation through the tree.
   
   @brief Backpropagates simulation results through the MCTS tree.
  
   Updates visit counts and win counts from the given node up to the root.
   Increments win count if the node's player matches the winner. Ensures
   thread safety by locking the node's mutex before updates.
   
   @param node A shared_ptr to the Node at which to start the backpropagation.
   @param winner The Cell_state of the winning player in the game simulation.
   */
  void backpropagate(std::shared_ptr<Node>& node, Cell_state winner);

  /**
   @brief Selects the best child node based on the highest win ratio.
   
   Iterates over child nodes, calculates win ratio, and returns the node with
   the highest ratio. Logs ratios in verbose mode. Throws a runtime error if
   no child can be selected due to insufficient statistics.
   
   @return A shared pointer to the child node with the highest win ratio.
   @throws std::runtime_error If no child can be selected due to insufficient
   statistics.
   */
  std::shared_ptr<Node> select_best_child();
};

#endif
