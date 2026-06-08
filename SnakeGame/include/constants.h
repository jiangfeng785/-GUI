#ifndef CONSTANTS_H
#define CONSTANTS_H

// 地图核心常量
const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;
const int MAX_SNAKE_LEN = 400;

// 地图元素字符标识
const char MAP_EMPTY = '.';
const char MAP_WALL = '#';
const char MAP_OBSTACLE = 'O';
const char MAP_SNAKE_HEAD = 'H';
const char MAP_SNAKE_BODY = 'B';
const char MAP_FOOD_NORMAL = 'F';
const char MAP_FOOD_GOLDEN = 'G';
const char MAP_BOMB = '*';
const char MAP_PORTAL_A = '@';
const char MAP_PORTAL_B = '&';

// 分数与技能配置
const int SCORE_NORMAL_FOOD = 10;
const int SCORE_GOLDEN_FOOD = 50;
const int SCORE_BOMB_PENALTY = -20;
const int SKILL_DASH_COST = 50;
const int SKILL_REVIVE_COST = 500;

// 游戏机制常量
const int OBSTACLE_MAX_HITS = 3;
const int DASH_DISTANCE = 3;

#endif // CONSTANTS_H
