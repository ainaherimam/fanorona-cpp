#include "game.h"
#include "logger.h"
#include <torch/torch.h>
#include <chrono>
#include "nn_model.h"

#include <iostream>

Game::Game(int board_size, std::unique_ptr<Player> player1,
           std::unique_ptr<Player> player_2)
    : board(board_size), current_player_index(0) {
  players[0] = std::move(player1);
  players[1] = std::move(player_2);
}

Cell_state Game::play() {

    // auto start_time = std::chrono::steady_clock::now();
    // const auto max_duration = std::chrono::seconds(5000);

    while (board.check_winner() == Cell_state::Empty) {


        // auto current_time = std::chrono::steady_clock::now();
        // if (current_time - start_time >= max_duration) {
        //     std::cout << "\nTime limit reached! Ending the game." << std::endl;
        //     break;
        // }

        board.get_board_size();
        Cell_state current_player =
            current_player_index == 0 ? Cell_state::X : Cell_state::O;

        std::cout << "\nPlayer " << current_player_index + 1 << "'s turn:" << std::endl;
        board.display_board(std::cout);
        auto [chosen_move, logits] = players[current_player_index]->choose_move(board, current_player);
        // Save game result for data
        if (logits.numel() > 1000) {

            result_logits.push_back(logits);              
            result_states.push_back(board.to_tensor(current_player));
            result_mask.push_back(board.to_tensor(current_player));
            float z_value;
            if (current_player == Cell_state::X) {z_value = 0.0;} 
            else if (current_player == Cell_state::O) {z_value = 1.0;}
            result_z.push_back(torch::full({1}, z_value, torch::kFloat32));

        }
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
    }
    
    board.display_board(std::cout);
    Cell_state winner = board.check_winner();
    std::cout << "Player " << winner << " wins!" << std::endl;

    if (!result_z.empty()) {
        for (auto& z_tensor : result_z) {
            float& val = z_tensor.accessor<float,1>()[0]; // get the single float

            if (winner == Cell_state::X) {
                // X wins: 0 → 1, 1 → -1
                val = (val == 0.0f) ? 1.0f : -1.0f;
            }
            else if (winner == Cell_state::O) {
                // O wins: 0 → -1, 1 → 1
                val = (val == 0.0f) ? -1.0f : 1.0f;
            }
            else {
                // Draw: all 0
                val = 0.0f;
            }
        }
    }

    for (size_t i = 0; i < result_states.size(); ++i) {
        auto board_tensor = result_states[i];  // (11,5,9) for example
        auto pi_tensor = result_logits[i];     // (1800)
        auto z_tensor = result_z[i];           // scalar (1)
        auto mask_tensor = result_mask[i];     // (1800)

        dataset.add_position(board_tensor, pi_tensor, z_tensor, mask_tensor);
    }

    // int idx = 3;
    // std::cout << "Logits at index " << idx << ":\n" << result_logits[idx] << std::endl;
    // std::cout << "States at index " << idx << ":\n" << result_states[idx] << std::endl;
    // std::cout << "Z at index " << idx << ":\n" << result_z[idx] << std::endl;

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
