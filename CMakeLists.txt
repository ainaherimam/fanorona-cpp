cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MCTS-Fanorona)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
add_executable(MCTS-Fanorona
    main.cpp
    board.cpp
    cell_state.cpp
    console_interface.cpp
    game.cpp
    player.cpp
)
