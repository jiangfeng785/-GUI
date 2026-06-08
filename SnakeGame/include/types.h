#ifndef TYPES_H
#define TYPES_H

#include "constants.h"
#include <vector>
#include <utility>

// 游戏状态机
enum GameState {
    GAME_START,
    GAME_RUNNING,
    GAME_PAUSE,
    GAME_OVER
};

// 移动方向
enum Direction {
    DIR_UP = 0,
    DIR_LEFT = 1,
    DIR_DOWN = 2,
    DIR_RIGHT = 3,
    DIR_NONE = 4
};

// 蛇身节点（纯数据结构）
struct SnakeNode {
    int x;
    int y;
    int r, g, b;

    SnakeNode(int x_ = 0, int y_ = 0, int r_ = 0, int g_ = 255, int b_ = 0)
        : x(x_), y(y_), r(r_), g(g_), b(b_) {}
};

// 食物/道具节点（纯数据结构）
struct Food {
    int x;
    int y;
    int type; // 0=普通,1=金苹果,2=炸弹,3=传送门A,4=传送门B

    Food(int x_ = 0, int y_ = 0, int type_ = 0)
        : x(x_), y(y_), type(type_) {}
};

#endif // TYPES_H
