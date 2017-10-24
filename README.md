# Mancala_AI
Simple [Mancala](https://en.wikipedia.org/wiki/Mancala) board game vs. an AI. The game rules are specified [here](https://www.hackerrank.com/challenges/mancala6).

The AI implements [iterative deepening](https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search) with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning). 

Compile via 
```bash
g++ -std=c++11 -o Mancala Mancala.cpp
```

Play in console via 
```bash
./Mancala
```

