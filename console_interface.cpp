#include "console_interface.h"
#include <chrono>
#include <thread>
#include <climits>

#include "board.h"


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



void start_human_arena() {
  int board_size = 5;
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
                << "[2] (H)Exit\n";

      option = get_parameter_within_bounds("Option: ", 1, 2);
      std::cout << "\n";

      switch (option) {
        case 1:
           start_human_arena();
          break;
        case 2:
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

void test() {
    
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
