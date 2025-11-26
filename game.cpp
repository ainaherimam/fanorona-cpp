#include "game.h"
#include "logger.h"
#include <torch/torch.h>
#include <chrono>
#include "nn_model.h"

#include <iostream>
#include <random>


static std::mt19937 random_generator(std::random_device{}());


Game::Game(int board_size, std::unique_ptr<Player> player1,
           std::unique_ptr<Player> player_2, GameDataset& dataset, bool verbose)
    : board(board_size), current_player_index(0), dataset_(dataset), verbose(verbose) {
  players[0] = std::move(player1);
  players[1] = std::move(player_2);
}

Cell_state Game::play() {

    int move_counter = 0;
    int max_move = 70;

    Cell_state current_player =
            current_player_index == 0 ? Cell_state::X : Cell_state::O;

    // Make 10 random move first
    random_move(10);


    while (board.check_winner() == Cell_state::Empty) {


        if (move_counter>max_move){
            break;
        }

        board.get_board_size();
        Cell_state current_player =
            current_player_index == 0 ? Cell_state::X : Cell_state::O;

        // board.display_board(std::cout);
        auto [chosen_move, logits] = players[current_player_index]->choose_move(board, current_player);
        // Skip if random move or human move
        if (logits.numel() > 1000) {
            auto board_tensor = board.to_tensor(current_player);
            auto pi_tensor = logits;
            auto mask_tensor = board.get_legal_mask(current_player);  
            
            float z_value;
            if (current_player == Cell_state::X) {z_value = 0.0;} 
            else if (current_player == Cell_state::O) {z_value = 1.0;}

            auto z_tensor = torch::tensor(z_value, torch::dtype(torch::kFloat32));
            result_z.push_back(z_tensor);

            dataset_.add_position(board_tensor, pi_tensor, z_tensor, mask_tensor);
        }

        int chosen_x = chosen_move[0];
        int chosen_y = chosen_move[1];
        int chosen_dir = chosen_move[2];
        int chosen_tar = chosen_move[3];

        board.make_move(chosen_x, chosen_y, chosen_dir, chosen_tar, current_player);
        if (chosen_move[3] < 1) {
            switch_player();
            board.clear_state();
        }
        move_counter ++;
    }

    Cell_state winner = board.check_winner();
    dataset_.update_last_z(result_z, winner);
    
    return winner;
}

Cell_state Game::simple_play() {

    int move_counter = 0;
    int max_move = 70;

    Cell_state current_player =
            current_player_index == 0 ? Cell_state::X : Cell_state::O;

    while (board.check_winner() == Cell_state::Empty) {


        if (move_counter>max_move){
            break;
        }

        board.get_board_size();
        Cell_state current_player = current_player_index == 0 ? Cell_state::X : Cell_state::O;

        std::cout << "\nPlayer " << current_player_index + 1 << "'s turn:" << std::endl;
        board.display_board(std::cout);

        auto [chosen_move, logits] = players[current_player_index]->choose_move(board, current_player);

        int chosen_x = chosen_move[0];
        int chosen_y = chosen_move[1];
        int chosen_dir = chosen_move[2];
        int chosen_tar = chosen_move[3];
        std::cout << "\nPlayer " << current_player_index + 1 << " chose move: " << print_move(chosen_move) << std::endl;
        board.make_move(chosen_x, chosen_y, chosen_dir, chosen_tar, current_player);
        if (chosen_move[3] < 1) {
            switch_player();
            board.clear_state();
        }
        move_counter ++;
    }
    Cell_state winner = board.check_winner();

    board.display_board(std::cout);
    std::cout << "Player " << winner << " wins!" << std::endl;
    
    return winner;
}
std::string Game::print_move(std::array<int, 4> move) {

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

    std::string message = "(" + std::to_string(row) + ", " + column + ") "
        + direction + " " + move_type;

    return message;
}

void Game::switch_player() {
    current_player_index = 1 - current_player_index;

}


void Game::random_move(int random_move_number) {

    int move_counter = 0;
    Cell_state player = (current_player_index == 0 
                         ? Cell_state::X 
                         : Cell_state::O);

    while (move_counter < random_move_number) {


        std::vector<std::array<int, 4>> valid_moves = board.get_valid_moves(player);
        if (valid_moves.empty()) {
            break;
        }

        std::uniform_int_distribution<> dist(0, static_cast<int>(valid_moves.size() - 1));
        const std::array<int, 4>& random_move = valid_moves[dist(random_generator)];

        board.make_move(random_move[0], random_move[1],
                        random_move[2], random_move[3], player);

        move_counter++;

        if (board.check_winner() != Cell_state::Empty) {
            break;
        }

        if (random_move[3] < 1) {
            switch_player();
            player = (current_player_index == 0 
                       ? Cell_state::X 
                       : Cell_state::O);

            board.clear_state();
        }
    }
}

