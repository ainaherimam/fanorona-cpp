#include "logger.h"

// Initialize static member
std::shared_ptr<Logger> Logger::logger = nullptr;

std::shared_ptr<Logger> Logger::instance(bool is_verbose) {
  if (!logger) {
    logger = std::make_shared<Logger>(is_verbose);
  }
  return logger;
}

void Logger::log(const std::string& message, bool always_print = false) {
  if (is_verbose || always_print) {
    // Lock the mutex to prevent interleaved output
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << message << std::endl;
  }
}

void Logger::log_mcts_start(Cell_state player) {
  std::ostringstream message;
  if (get_verbosity()) {
    message << "\n-------------MCTS VERBOSE START - " << player
            << " to move-------------\n";
    log(message.str());
  } else {
    message << "Thinking silently...";
    log(message.str(), true);
  }
}

void Logger::log_iteration_number(int iteration_number) {
  std::ostringstream message;
  message << "\n------------------STARTING SIMULATION " << iteration_number
          << "------------------\n";
  log(message.str());
}

void Logger::log_expanded_child(const std::array<int, 4>& move) {
  std::ostringstream message;
  message << "EXPANDED CHILD " << print_move(move);
  log(message.str());
}

void Logger::log_selected_child(const std::array<int, 4>& move,
                                double uct_score) {
  std::ostringstream message;
  message << "SELECTED CHILD " << print_move(move)
          << " with UCT of ";
  if (uct_score == std::numeric_limits<double>::max()) {
    message << "infinity";
  } else {
    message << std::setprecision(4) << uct_score;
  }
  log(message.str());
}

void Logger::log_simulation_start(const std::array<int, 4>& move,
                                  const Board& board) {
  if (is_verbose) {
    std::ostringstream message;
    std::ostringstream board_string;
    board.display_board(board_string);
    message << "\nSIMULATING A RANDOM PLAYOUT from node " << print_move(move) 
            << ". Simulation board is in state:\n"
            << board_string.str();
    log(message.str());
  }
}

void Logger::log_simulation_step(Cell_state current_player, const Board& board,
                                 const std::array<int, 4>& move) {
  if (is_verbose) {
    std::ostringstream message;
    std::ostringstream board_string;
    board.display_board(board_string);
    message << "Current player in simulation is " << current_player
            << " in Board state:\n"
            << board_string.str();
    message << current_player << " makes random move " << print_move(move) << ". ";
    log(message.str());
  }
}

void Logger::log_simulation_end(Cell_state winning_player, const Board& board) {
  if (is_verbose) {
    std::ostringstream message;
    std::ostringstream board_string;
    board.display_board(board_string);
    message << "DETECTED WIN for player " << winning_player
            << " in Board state:\n"
            << board_string.str();
    log(message.str());
  }
}

void Logger::log_backpropagation_result(const std::array<int, 4>& move,
                                        int win_count, int visit_count) {
  std::ostringstream message;
  message << "BACKPROPAGATED result to node " << print_move(move)  
          << ". It currently has " << win_count << " wins and "
          << visit_count << " visits.";
  log(message.str());
}

void Logger::log_root_stats(int visit_count, int win_count,
                            size_t child_nodes) {
  std::ostringstream message;
  message << "\nAFTER BACKPROPAGATION, root node has " << visit_count
          << " visits, " << win_count << " wins, and " << child_nodes
          << " child nodes. Their details are:\n";
  log(message.str());
}

void Logger::log_child_node_stats(const std::array<int, 4>& move,
                                  int win_count, int visit_count) {
  std::ostringstream message;
  message << "Child node " << print_move(move)
          << ": Wins: " << win_count << ", Visits: " << visit_count
          << ". Win ratio: ";

  if (visit_count) {
    message << std::fixed << std::setprecision(2)
            << static_cast<double>(win_count) / visit_count;
  } else {
    message << "N/A (no visits yet)";
  }
  log(message.str());
}

void Logger::log_timer_ran_out(int iteration_counter) {
  std::ostringstream message;
  message << "\nTIMER RAN OUT. " << iteration_counter
          << " iterations completed. CHOOSING A MOVE FROM ROOT'S CHILDREN:\n";
  log(message.str());
}

void Logger::log_node_win_ratio(const std::array<int, 4>& move, int win_count,
                                int visit_count) {
  std::ostringstream win_ratio_stream;
  if (visit_count > 0) {
    win_ratio_stream << std::fixed << std::setprecision(2)
                     << static_cast<double>(win_count) / visit_count;
  } else {
    win_ratio_stream << "N/A (no visits yet)";
  }

  std::ostringstream message;
  message << "Child " << print_move(move)
          << " has a win ratio of " << win_ratio_stream.str();
  log(message.str());
}

void Logger::log_best_child_chosen(int iteration_counter,
                                   const std::array<int, 4>& move,
                                   double win_ratio) {
  std::ostringstream message;
  message << "\nAfter " << iteration_counter << " iterations, chose child "
          << print_move(move) << " with win ratio "
          << std::setprecision(4) << win_ratio;
  log(message.str());
}

void Logger::log_mcts_end() {
  log("\n--------------------MCTS VERBOSE END--------------------\n");
}

std::string Logger::print_move(std::array<int, 4> move) {

        // Print the row as a number and the column as an alphabet
        char column = 'a' + move[1];  
        int row = move[0] + 1; 

        // Determine direction based on the dir value
        std::string direction;
        switch (move[2]) {
        case 1: direction = "Move downleft "; break;
        case 2: direction = "Move down"; break;
        case 3: direction = "Move downright"; break;
        case 4: direction = "Move left"; break;
        case 5: direction = "Stay"; break;
        case 6: direction = "Move right"; break;
        case 7: direction = "Moveupleft"; break;
        case 8: direction = "Move up"; break;
        case 9: direction = "Move upright"; break;
        default: direction = "unknown"; break;
        }

        // Determine move type
        std::string move_type;
        switch (move[3]) {
        case -1: move_type = ""; break;
        case 0: move_type = ""; break;
        case 1: move_type = "and take backward"; break;
        case 2: move_type = "and take forward"; break;
        default: move_type = "unknown"; break;
        }

        std::string message = "("+ std::to_string(row) + ", " + column +") "
            + direction + " " + move_type;

        return message;
}
