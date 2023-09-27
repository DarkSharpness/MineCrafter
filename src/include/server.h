#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <set>
#include <utility>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to uss global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */

constexpr int MAXN = 1e3 + 5;

int rows; // The count of rows of the game map
int columns;  // The count of columns of the game map
int game_state; // The state of the game, 0 for continuing, 1 for winning, -1 for losing
int total_safe_block; // The count of safe blocks
int visit_count;  // The count of blocks visited
int step_count; // The count of steps taken
int mine_count[MAXN][MAXN];  // The count of mines in the adjacent blocks of (i, j), -1 if (i, j) is a mine
bool visited[MAXN][MAXN];    // Whether the block (i, j) has been visited

std::set<std::pair<int, int>> visited_blocks;

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map would
 * be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  for (int i = 0; i < rows; ++i) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; ++j) {
      if (line[j] == 'X') {
        mine_count[i][j] = -1;
      } else {
        mine_count[i][j] = 0;
        total_safe_block++;
      }
      visited[i][j] = false;
    }
  }
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (mine_count[i][j] == -1) {
        continue;
      }
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          int nx = i + dx;
          int ny = j + dy;
          if (nx < 0 || nx >= rows || ny < 0 || ny >= columns) {
            continue;
          }
          if (mine_count[nx][ny] == -1) {
            ++mine_count[i][j];
          }
        }
      }
    }
  }
}

void VisitRecursive(unsigned int row, unsigned int column) {
  if (visited[row][column]) {
    return;
  }
  if (mine_count[row][column] == -1) {
    return;
  }
  visited[row][column] = true;
  visit_count++;
  visited_blocks.insert(std::pair<int, int>(row, column));
  if (mine_count[row][column] != 0) {
    return;
  }
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      int nx = row + dx;
      int ny = column + dy;
      if (nx < 0 || nx >= rows || ny < 0 || ny >= columns) {
        continue;
      }
      VisitRecursive(nx, ny);
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param row The row coordinate (0-based) of the block to be visited.
 * @param column The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 */
void VisitBlock(unsigned int row, unsigned int column) {  
  step_count++;
  if (visited[row][column]) {
    game_state = 0;
    return;
  }
  if (mine_count[row][column] == -1) {
    visited[row][column] = true;
    game_state = -1;
    return;
  }
  VisitRecursive(row, column);
  if (game_state != -1 && visit_count == total_safe_block) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    std::string line;
    for (int j = 0; j < columns; ++j) {
      if (visited[i][j]) {
        if (mine_count[i][j] == -1) {
          line += 'X';
        } else {
          line += std::to_string(mine_count[i][j]);
        }
      } else {
        line += (game_state != 1 ? '?' : '@');
      }
    }
    std::cout << line << std::endl;
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game. 
 * It outputs a line according to the result, and a line of two integers, visit_count and step_count,
 * representing the number of blocks visited and the number of steps taken respectively.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
  }
  std::cout << visit_count << " " << step_count << std::endl;
  exit(0); // Exit the game immediately
}

#endif