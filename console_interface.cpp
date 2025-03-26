#include "console_interface.h"
#include <chrono>
#include <thread>
#include <climits>

#include "board.h"
#include "mcts_agent.h"


bool is_integer(const std::string& s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}

char get_yes_or_no_response(const std::string& prompt) {
  char response;
  while (true) {
    std::cout << prompt;
    std::cin >> response;

    if (std::cin.fail()) {
      std::cin.clear();  
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                      '\n');  
      std::cout << "Invalid input. Please enter 'y' or 'n'.\n";
    } else if (response != 'y' && response != 'n' && response != 'Y' &&
               response != 'N') {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                      '\n');
      std::cout << "Invalid response. Please enter 'y' or 'n'.\n";
    } else {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return std::tolower(response);
    }
  }
}

template <>
int get_parameter_within_bounds<int>(const std::string& prompt, int lower_bound,
                                     int upper_bound) {
  std::string input;
  int value;

  while (true) {
    std::cout << prompt;
    std::cin >> input;

    // Check if input is a valid integer
    if (!is_integer(input)) {
      std::cout << "Invalid input. Please enter a valid integer.\n";
      continue;
    }

    // Convert string to int
    value = std::stoi(input);

    // Check if value is within bounds
    if (!is_in_bounds(value, lower_bound, upper_bound)) {
      std::cout << "Invalid value. Please try again.\n";
    } else {
      break;
    }
  }

  return value;
}

template <>
double get_parameter_within_bounds<double>(const std::string& prompt,
                                           double lower_bound,
                                           double upper_bound) {
  std::string input;
  double value;

  while (true) {
    std::cout << prompt;
    std::cin >> input;

    // Check if input is a valid double
    try {
      value = std::stod(input);
    } catch (std::invalid_argument&) {
      std::cout << "Invalid input. Please enter a valid number.\n";
      continue;
    }

    // Check if value is within bounds
    if (!is_in_bounds(value, lower_bound, upper_bound)) {
      std::cout << "Invalid value. Please try again.\n";
    } else {
      break;
    }
  }

  return value;
}

std::unique_ptr<Mcts_player> create_mcts_agent(
    const std::string& agent_prompt) {
    std::cout << "\nInitializing " << agent_prompt << ":\n";

    int max_decision_time_ms = get_parameter_within_bounds(
        "Max decision time in milliseconds(at least 100) : ", 100, INT_MAX);

    double exploration_constant = 1.41;

    exploration_constant = get_parameter_within_bounds(
        "Enter exploration constant (between 0.1 and 2): ", 0.1, 2.0);

    bool is_verbose = false;

    is_verbose = (get_yes_or_no_response(
            "Would you like to enable verbose mode? (y/n): ") == 'y');

    return std::make_unique<Mcts_player>(
        exploration_constant, std::chrono::milliseconds(max_decision_time_ms),
        is_verbose);
}

void countdown(int seconds) {
    while (seconds > 0) {
        std::cout << "The agent will start thinking loudly in " << seconds
            << " ...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --seconds;
    }
}

void start_match_against_robot() {
    int human_player_number = get_parameter_within_bounds(
        "Enter '1' if you want to be Player 1 (X) or '2' if you "
        "want to be "
        "Player 2 (O): ",
        1, 2);

    int board_size = 9;

    auto mcts_agent = create_mcts_agent("agent");
    auto human_player = std::make_unique<Human_player>();

    if (human_player_number == 1) {
        Game game(board_size, std::move(human_player), std::move(mcts_agent));
        game.play();
    }
    else {
        if (mcts_agent->get_is_verbose()) {
            countdown(3);
        }
        Game game(board_size, std::move(mcts_agent), std::move(human_player));
        game.play();
    }
}

void start_robot_arena() {

    int board_size = 9;

    auto mcts_agent_1 = create_mcts_agent("first agent");
    auto mcts_agent_2 = create_mcts_agent("second agent");

    Game game(board_size, std::move(mcts_agent_1), std::move(mcts_agent_2));
    game.play();
}


void start_human_arena() {
  int board_size = 9;
  auto human_player_1 = std::make_unique<Human_player>();
  auto human_player_2 = std::make_unique<Human_player>();
  Game game(board_size, std::move(human_player_1), std::move(human_player_2));
  game.play();
}


void run_console_interface() {
  print_welcome_ascii_art();
  std::cout << "Hi ;).\n";

  bool is_running = true;
  while (is_running) {
    try {
      int option = 0;
      std::cout << "\nMENU:\n"
                << "[1] Human player vs Human player\n"
                << "[2] AI player vs AI player\n"
                << "[3] Human player vs AI player\n"
                << "[4] (H)Exit\n";

      option = get_parameter_within_bounds("Option: ", 1, 4);
      std::cout << "\n";

      switch (option) {
        case 1:
           start_human_arena();
           break;
        case 2:
            start_robot_arena();
            break;
        case 3:
            start_match_against_robot();
          break;
        case 4:
          is_running = false;
          break;
        default:
          break;
      }
    } catch (const std::invalid_argument& e) {
      std::cout << "Error: " << e.what() << "\n";
    } catch (const std::logic_error& e) {
      std::cout << "Error: " << e.what() << "\n";
    } catch (const std::runtime_error& e) {
      std::cout << "Error: " << e.what() << "\n";
    }
  }
  print_exit_ascii_art();
}

void print_welcome_ascii_art() {
  std::cout << R"(

    __  ____________________    ______                                        
   /  |/  / ____/_  __/ ___/   / ____/___ _____  ____  _________  ____  ____ _
  / /|_/ / /     / /  \__ \   / /_  / __ `/ __ \/ __ \/ ___/ __ \/ __ \/ __ `/
 / /  / / /___  / /  ___/ /  / __/ / /_/ / / / / /_/ / /  / /_/ / / / / /_/ / 
/_/  /_/\____/ /_/  /____/  /_/    \__,_/_/ /_/\____/_/   \____/_/ /_/\__,_/  
                                                                 by AinaHerimam          

)" << '\n';
}

void print_board_and_winner(Board& board) {
  board.display_board(std::cout);
  Cell_state winner = board.check_winner();
  std::cout << "Winner: " << winner << std::endl;
  std::cout << "------------------" << std::endl;
}

void print_exit_ascii_art() {
  std::cout << R"(
       
Tank you!!!
              ┛         
)" << '\n';
}
