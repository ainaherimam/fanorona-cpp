#include "mcts_agent.h"
#include "nn_model.h"
#include <torch/torch.h>

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

NeuralN::NeuralN(const std::string& model_path, torch::Device device_)
    : device(device_)
{
    try {
        // Load the model once
        model = AlphaZeroNetWithMaskImpl::load_model(model_path);
        model->to(device);
        model->eval();
    } catch (const c10::Error& e) {
        throw;
    }
}

std::pair<torch::Tensor, torch::Tensor> NeuralN::predict(torch::Tensor input, torch::Tensor legal_mask) {
    torch::NoGradGuard no_grad; 
    input = input.to(device);
    if (legal_mask.defined()) legal_mask = legal_mask.to(device);
    return model->forward(input, legal_mask);
}


Mcts_agent::Mcts_agent(double exploration_factor,
                       int number_iteration,
                       bool is_verbose)
    : exploration_factor(exploration_factor),
      number_iteration(number_iteration),
      logger(Logger::instance(is_verbose)),
      random_generator(random_device()) {

      agent = std::make_shared<NeuralN>("checkpoint/1.pt");
}

Mcts_agent::Node::Node(Cell_state player, std::array<int, 4> move, float prior_proba, float value_from_nn,
                       std::shared_ptr<Node> parent_node)
    : value_from_nn(value_from_nn),
      value_from_mcts(0.0f),
      expanded(false),
      acc_value(0.0f),
      prior_proba(prior_proba),
      visit_count(0),
      move(move),
      player(player),
      child_nodes(),
      parent_node(parent_node) {}

std::pair<std::array<int, 4>,torch::Tensor> Mcts_agent::choose_move(const Board& board,
                                            Cell_state player) {
  logger->log_mcts_start(player);
  // Create a new  root node and expand it
  std::array<int, 4> arr = { -1, -1, -1, -1 };
  root = std::make_shared<Node>(player, arr, 0.0, 0.0, nullptr);
  initiate_and_run_nn(root, board);

  int mcts_iteration_counter =0;
  // Run MCTS until the timer runs out
  perform_mcts_iterations(number_iteration, mcts_iteration_counter, board);
  logger->log_timer_ran_out(mcts_iteration_counter);

  // Select the child with the highest win ratio as the best move:
  std::shared_ptr<Node> best_child = select_best_child(root);
  
  logger->log_best_child_chosen(
      mcts_iteration_counter, best_child->move,
      static_cast<double>(best_child->acc_value) / best_child->visit_count);
  logger->log_mcts_end();
  return {best_child->move, get_policy_logits(root)}; 

}

void Mcts_agent::random_move(Board& board, Cell_state player, int random_move_number) {
    
    int move_counter = 0; 

    while (move_counter < random_move_number) {

        std::vector<std::array<int, 4>> valid_moves = board.get_valid_moves(player);
        if (valid_moves.empty()) {
            break; 
        }

        std::uniform_int_distribution<> dist(0, static_cast<int>(valid_moves.size() - 1));
        std::array<int, 4> random_move = valid_moves[dist(random_generator)];

        board.make_move(random_move[0], random_move[1], random_move[2], random_move[3], player);
        move_counter++;

        if (board.check_winner() != Cell_state::Empty) {
            break;
        }

        if (random_move[3] < 1) {
            player = (player == Cell_state::X ? Cell_state::O : Cell_state::X);
            board.clear_state();
        }

    }
}


float Mcts_agent::initiate_and_run_nn(const std::shared_ptr<Node>& node,
                             const Board& board) {

  Cell_state current_player = node->player;
  Cell_state actual_player = node->player;

  torch::Tensor input = board.to_tensor(current_player).unsqueeze(0);
  torch::Tensor legal_mask = board.get_legal_mask(current_player).unsqueeze(0);  

  auto [policy, value] = agent->predict(input, legal_mask);

  std::vector<std::pair<std::array<int,4>, float>> move_with_logit = get_moves_with_logits(policy);
  
  // For each valid move, create a new child node and add it to the node's
  // children.
  for (const auto& [move, logit] : move_with_logit) {
    
    if (move[3] < 1) {
      // Switch player
      actual_player = (current_player == Cell_state::X ? Cell_state::O : Cell_state::X);
    }
    else{
      actual_player = current_player;
    }
    std::shared_ptr<Node> new_child =
        std::make_shared<Node>(actual_player, std::array<int, 4>(move), logit, 0.0, node);
    node->child_nodes.push_back(new_child);
    node->value_from_nn =  value.item<float>();
    node->expanded = true;
  }

  return value.item<float>();
}


void Mcts_agent::perform_mcts_iterations(
    int number_iteration,
    int& mcts_iteration_counter, const Board& board) {
    int count = 0;
    while (mcts_iteration_counter < number_iteration) {
      logger->log_iteration_number(mcts_iteration_counter + 1);

      auto [chosen_child, new_board] = select_child_for_playout(root, board);
      float value_from_nn = simulate_random_playout(chosen_child, new_board);
      backpropagate(chosen_child, value_from_nn);

      logger->log_root_stats(root->visit_count, root->acc_value,
                            root->child_nodes.size());
      for (const auto& child : root->child_nodes) {
      logger->log_child_node_stats(child->move, child->acc_value,
                                    child->visit_count);
      }
      mcts_iteration_counter++;
    }
}


torch::Tensor Mcts_agent::get_policy_logits(const std::shared_ptr<Node>& parent_node) const {
    const int X = 5;
    const int Y = 10;
    const int DIR = 9;
    const int TAR = 4; 
    const int total_size = X * Y * DIR * TAR;

    torch::Tensor all_moves = torch::zeros({total_size}, torch::kFloat32);

    //(x,y,dir,tar) → index
    auto index = [&](int x, int y, int dir, int tar) -> int {
        int dir_idx = dir - 1; 
        int tar_idx = tar + 1; 
        return x * (Y * DIR * TAR) + y * (DIR * TAR) + dir_idx * TAR + tar_idx;
        };


    for (const auto& child : parent_node->child_nodes) {
        auto m = child->move;
        float winrate = (child->visit_count > 0)
            ? (child->visit_count / parent_node->visit_count)
            : 0.0;

        int idx = index(m[0], m[1], m[2], m[3]);
        if (idx >= 0 && idx < total_size)
            all_moves[idx] = winrate;
    }

    return all_moves;
}

std::vector<std::pair<std::array<int,4>, float>>
Mcts_agent::get_moves_with_logits(const torch::Tensor& logits_tensor) const 
{
    const int X = 5;
    const int Y = 10;
    const int DIR = 9;
    const int TAR = 4;
    const int total = X * Y * DIR * TAR;

    torch::Tensor softmax_p = torch::exp(logits_tensor);
    // Move tensor to CPU and flatten to 1D
    torch::Tensor logits = softmax_p.to(torch::kCPU).contiguous().view({total});

    // Get pointer to raw data (fastest access)
    const float* data = logits.data_ptr<float>();

    std::vector<std::pair<std::array<int,4>, float>> moves;
    moves.reserve(total);  

    for (int idx = 0; idx < total; idx++) {

        float logit = data[idx];
        if (logit == 0.0f)
            continue;               


        int t = idx;

        int x = t / (Y * DIR * TAR); 
        t %= (Y * DIR * TAR);

        int y = t / (DIR * TAR);
        t %= (DIR * TAR);

        int dir_idx = t / TAR;
        int tar_idx = t % TAR;

        int dir = dir_idx + 1;      
        int tar = tar_idx - 1;      

        std::array<int,4> move = { x, y, dir, tar };

        moves.emplace_back(move, logit);
    }


    return moves;
}


std::pair<std::shared_ptr<Mcts_agent::Node>, Board>
Mcts_agent::select_child_for_playout(
    const std::shared_ptr<Node>& parent_node,
    Board board)
{
    std::shared_ptr<Node> current = parent_node;
    Cell_state current_player = current->player;

    while (current->expanded && !current->child_nodes.empty()) 
    {
        // Pick best child
        std::shared_ptr<Node> best_child = current->child_nodes[0];
        double max_score = calculate_puct_score(best_child, current);

        for (size_t i = 1; i < current->child_nodes.size(); i++) {
            auto& child = current->child_nodes[i];
            double score = calculate_puct_score(child, current);
            if (score > max_score) {
                max_score = score;
                best_child = child;
            }
        }

        logger->log_selected_child(best_child->move, max_score);

        // Apply move
        board.make_move(best_child->move[0], best_child->move[1], best_child->move[2], best_child->move[3], current_player);

        if (best_child->move[3] < 1) {
            // Switch player
            current_player = (current_player == Cell_state::X ? Cell_state::O
                                                              : Cell_state::X);
            board.clear_state();
        }

        current = best_child;
    }

    return { current, board };
}

double Mcts_agent::calculate_puct_score(
    const std::shared_ptr<Node>& child_node,
    const std::shared_ptr<Node>& parent_node) {

    return static_cast<double>(child_node->value_from_mcts + exploration_factor * child_node->prior_proba * ( std::sqrt(parent_node->visit_count) / (child_node->visit_count + 1) ));

  }


float Mcts_agent::simulate_random_playout(
    const std::shared_ptr<Node>& node, Board board) {
  
  //Start the simulation

  Cell_state winner = board.check_winner();
  if (winner == root->player) {
      return 1.0;   // current player won
      
  } 
  else if (winner == Cell_state::Empty) {
      float value = initiate_and_run_nn(node,board);
      logger->log_simulation_end(value);
      return value;
  }
  else {
      return -1.0;  // opponent won
  }

}

void Mcts_agent::backpropagate(std::shared_ptr<Node>& node, float value) {
  // Start backpropagation
  std::shared_ptr<Node> current_node = node;
  while (current_node != nullptr) {
    // Lock the node's mutex before updating its data
    std::lock_guard<std::mutex> lock(current_node->node_mutex);
    // Increment the node's visit count
    current_node->visit_count += 1;
    // Update accumulated value of the node
    current_node->acc_value += value;
    current_node->value_from_mcts = current_node->acc_value / current_node->visit_count;
    logger->log_backpropagation_result(
        current_node->move, current_node->acc_value, current_node->visit_count);
    // Move to the parent node for the next loop
    if (current_node->parent_node != nullptr && 
    current_node->player != current_node->parent_node->player) {
        value = -value;
    }

    current_node = current_node->parent_node;
  }
}

std::shared_ptr<Mcts_agent::Node> Mcts_agent::select_best_child(std::shared_ptr<Node>& node) {
  double max_win_ratio = -1.;
  std::shared_ptr<Node> best_child;
  // loop over the child nodes of the root to find the one with the highest win ratio
  for (const auto& child : node->child_nodes) {
    double win_ratio =
        static_cast<double>(child->acc_value) / child->visit_count;
    // Print the win ratio for each child node.
    logger->log_node_win_ratio(child->move, child->acc_value,
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