## ▶️ Run the project on colab via this [Link](https://colab.research.google.com/drive/1JO8eqtxZQfcTK0KhBFiy-G2EYGEl9rKc?usp=sharing)


# Fanorona on C++20 with MCTS

 A project that implements the Monte Carlo Tree Search (MCTS) algorithm for the traditional board game of [Fanorona](https://en.wikipedia.org/wiki/Fanorona) on C++20.

 ![img1](./images/fanorona.png)

---

## 📖 About Fanorona

Fanorona is a centuries-old strategy board game originating from Madagascar. It is played on a 9x5 grid (fanorona 9) or a 5x5 grid (fanorona 5)  and challenges players to capture their opponent's pieces using a variety of strategic moves. The game is renowned for its depth and complexity, making it an excellent candidate for AI research.

---
## 🎯 Project
- Implement a simple interface for the Fanorona game 
- Implement the **Monte Carlo Tree Search (MCTS)** algorithm to play Fanorona.
- Develop an algorithm capable of making intelligent, adaptive decisions.

---

## 🚀 Features
- **Fanorona Game Logic**: The game board is represented programmatically, supporting all valid moves and captures.
- **MCTS Algorithm**: To calculate optimal moves.
- **Interactive Gameplay**: Play against the AI (Still in dev)
- **Learn from SelfPlay**: AlphaZero style implementation (Still in dev)
---

## 🛠️ Getting Started

## Prerequisites
- **C++20 Compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake 3.20+**
- **Libtorch**

## Installation
```sh
# Clone repository
git clone https://github.com/ainaherimam/fanorona-cpp.git && cd fanorona-cpp

# Create build directory
mkdir build && cd build

# Configure and build
cmake -DCMAKE_PREFIX_PATH=${LIBTORCH_PATH} ..
make -j$(nproc)
```

## 🛠️ To do
- Outer loop of Self-play
- Parallelization of MCTS


## 🌟 Acknowledgments
- Inspired by the board game [Fanorona](https://en.wikipedia.org/wiki/Fanorona)

Enjoy playing and exploring the strategies of Fanorona!
