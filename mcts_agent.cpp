#include "mcts_agent.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

Mcts_agent::Mcts_agent(double exploration_factor,
                       std::chrono::milliseconds max_decision_time,
                       bool is_verbose)
    : exploration_factor(exploration_factor),
      max_decision_time(max_decision_time),
      logger(Logger::instance(is_verbose)),
      random_generator(random_device()) {
}

Mcts_agent::Node::Node(Cell_state player, std::array<int, 4> move,
                       std::shared_ptr<Node> parent_node)
    : win_count(0),
      visit_count(0),
      move(move),
      player(player),
      child_nodes(),
      parent_node(parent_node) {}

std::array<int, 4> Mcts_agent::choose_move(const Board& board,
                                            Cell_state player) {
  logger->log_mcts_start(player);
  // Create a new root node for MCTS
  std::array<int, 4> arr = { -1, -1, -1, -1 };
  root = std::make_shared<Node>(player, arr, nullptr);

  // Expand root based on the current game state
  expand_node(root, board);

  int mcts_iteration_counter = 0;
  auto start_time = std::chrono::high_resolution_clock::now();
  auto end_time = start_time + max_decision_time;
  // Run MCTS until the timer runs out
  
  perform_mcts_iterations(end_time, mcts_iteration_counter, board);
  logger->log_timer_ran_out(mcts_iteration_counter);
  // Select the child with the highest win ratio as the best move:
  std::shared_ptr<Node> best_child = select_best_child();

  for (const auto& child : root->child_nodes) {
      std::ostringstream message;
      message << "Child node " << child->move[0] << child->move[1] << child->move[2] << child->move[3]
          << ": Wins: " << child->win_count << ", Visits: " << child->visit_count
          << ". Win ratio: ";

      if (child->visit_count) {
          message << std::fixed << std::setprecision(2)
              << static_cast<double>(child->win_count) / child->visit_count;
      }
      else {
          message << "N/A (no visits yet)";
      }
      message << "\n";
      std::cout << message.str();

  }

  logger->log_best_child_chosen(
      mcts_iteration_counter, best_child->move,
      static_cast<double>(best_child->win_count) / best_child->visit_count);
  logger->log_mcts_end();
  std::cout << "MCTS Counter:" << mcts_iteration_counter;
  return best_child->move; 
}

void Mcts_agent::expand_node(const std::shared_ptr<Node>& node,
                             const Board& board) {
  Cell_state current_player = node->player;
  std::vector<std::array<int, 4>> valid_moves = board.get_valid_moves(current_player);
  // For each valid move, create a new child node and add it to the node's
  // children.
  for (const auto& move : valid_moves) {
    std::shared_ptr<Node> new_child =
        std::make_shared<Node>(node->player, std::array<int, 4>(move), node);
    node->child_nodes.push_back(new_child);
    logger->log_expanded_child(std::array<int, 4>(move));
  }
}

void Mcts_agent::perform_mcts_iterations(
    const std::chrono::time_point<std::chrono::high_resolution_clock>& end_time,
    int& mcts_iteration_counter, const Board& board) {

  while (std::chrono::high_resolution_clock::now() < end_time) {
    logger->log_iteration_number(mcts_iteration_counter + 1);
    // Select a child node for playout
    std::shared_ptr<Node> chosen_child = select_child_for_playout(root);
   
    Cell_state playout_winner = simulate_random_playout(chosen_child, board);
    backpropagate(chosen_child, playout_winner);

    logger->log_root_stats(root->visit_count, root->win_count,
                           root->child_nodes.size());
    for (const auto& child : root->child_nodes) {
      logger->log_child_node_stats(child->move, child->win_count,
                                   child->visit_count);
    }
    mcts_iteration_counter++;
  }
}

std::shared_ptr<Mcts_agent::Node> Mcts_agent::select_child_for_playout(
    const std::shared_ptr<Node>& parent_node) {
  // Initialize best_child as the first child and compute its UCT score
  std::shared_ptr<Node> best_child = parent_node->child_nodes[0];
  double max_score = calculate_uct_score(best_child, parent_node);
  // Loop over the remaining child nodes to find the one with the highest UCT score
  for (auto iterator = std::next(parent_node->child_nodes.begin());
       iterator != parent_node->child_nodes.end(); ++iterator) {
    const auto& child = *iterator;
    double uct_score = calculate_uct_score(child, parent_node);

    if (uct_score > max_score) {
      max_score = uct_score;
      best_child = child;
    }
  }
  // If verbose mode is enabled, print the move coordinates and UCT score of the selected child
  logger->log_selected_child(best_child->move, max_score);
  return best_child;
}

double Mcts_agent::calculate_uct_score(
    const std::shared_ptr<Node>& child_node,
    const std::shared_ptr<Node>& parent_node) {
  // If any child node has not been visited yet, return a high value to encourage exploration
  if (child_node->visit_count == 0) {
    return std::numeric_limits<double>::max();
  } else {
    // Otherwise, calculate the UCT score using the UCT formula.
    return static_cast<double>(child_node->win_count) /
               child_node->visit_count +
           exploration_factor * std::sqrt(std::log(parent_node->visit_count) /
                                          child_node->visit_count);
  }
}

Cell_state Mcts_agent::simulate_random_playout(
    const std::shared_ptr<Node>& node, Board board) {
  //Start the simulation
  Cell_state current_player = node->player;
  // Make the move at the node first.
  board.make_move(node->move[0], node->move[1], node->move[2], node->move[3], current_player);

  if (node->move[3] < 1) {
      // Switch player
      current_player = (current_player == Cell_state::X ? Cell_state::O
          : Cell_state::X);
      board.clear_state();
  }

  logger->log_simulation_start(node->move, board);
  // Continue simulation until a winner is detected
  while (board.check_winner() == Cell_state::Empty) {
    
    // Get all valid moves
    std::vector<std::array<int, 4>> valid_moves = board.get_valid_moves(current_player);
    
    // Sometimes player are surrounded by the other player pieces and can't move
    if (valid_moves.empty() == true) {
        break;
    }

    // Generate a distribution and choose a move randomly
    std::uniform_int_distribution<> distribution(
        0, static_cast<int>(valid_moves.size() - 1));
    std::array<int, 4> random_move =
        valid_moves[distribution(random_generator)];
    logger->log_simulation_step(current_player, board, random_move);
    board.make_move(random_move[0], random_move[1], random_move[2], random_move[3], current_player);
    // If a player has won, break the loop
    if (board.check_winner() != Cell_state::Empty) {
      logger->log_simulation_end(current_player, board);
      break;
    }
    if (random_move[3] < 1) {
        // Switch player if the move is a taking move
        current_player = (current_player == Cell_state::X ? Cell_state::O
            : Cell_state::X);
        board.clear_state();
    }
    
  }
  return board.check_winner();
}

void Mcts_agent::backpropagate(std::shared_ptr<Node>& node, Cell_state winner) {
  // Start backpropagation
  std::shared_ptr<Node> current_node = node;
  while (current_node != nullptr) {
    // Lock the node's mutex before updating its data
    std::lock_guard<std::mutex> lock(current_node->node_mutex);
    // Increment the node's visit count
    current_node->visit_count += 1;
    // If the winner is the same as the player at the node, increment the node's win count
    if (winner == current_node->player) {
      current_node->win_count += 1;
    }
    logger->log_backpropagation_result(
        current_node->move, current_node->win_count, current_node->visit_count);
    // Move to the parent node for the next loop
    current_node = current_node->parent_node;
  }
}

std::shared_ptr<Mcts_agent::Node> Mcts_agent::select_best_child() {
  double max_win_ratio = -1.;
  std::shared_ptr<Node> best_child;
  // loop over the child nodes of the root to find the one with the highest win ratio
  for (const auto& child : root->child_nodes) {
    double win_ratio =
        static_cast<double>(child->win_count) / child->visit_count;
    // Print the win ratio for each child node.
    logger->log_node_win_ratio(child->move, child->win_count,
                               child->visit_count);
    if (win_ratio > max_win_ratio) {
      max_win_ratio = win_ratio;
      best_child = child;
    }
  }
  if (!best_child) {
    throw std::runtime_error("Statistics are not enough to determine a move. The robot had insufficient time for the given board size.");
  }
  return best_child;
}