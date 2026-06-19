#pragma once
#include <graphics.h>
#include <vector>
#include <fstream>

// ========== 基础地图常量 ==========
const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;
const int CELL_SIZE = 30;
const int MAX_SNAKE_LEN = 400;

// 窗口尺寸
const int UI_WIDTH = 220;
const int WIN_WIDTH = MAP_WIDTH * CELL_SIZE + UI_WIDTH;
const int WIN_HEIGHT = MAP_HEIGHT * CELL_SIZE;

// 界面配色常量
const DWORD COLOR_BG = RGB(18, 18, 22);
const DWORD COLOR_GRID = RGB(45, 45, 52);
const DWORD COLOR_WALL = RGB(60, 30, 30);
const DWORD COLOR_OBSTACLE = RGB(90, 60, 40);
const DWORD COLOR_SNAKE_HEAD = RGB(0, 255, 120);
const DWORD COLOR_SNAKE_BODY = RGB(0, 180, 90);
const DWORD COLOR_FOOD_NORMAL = RGB(255, 60, 80);
const DWORD COLOR_FOOD_GOLD = RGB(255, 200, 0);
const DWORD COLOR_BOMB = RGB(100, 100, 100);
const DWORD COLOR_TEXT_MAIN = RGB(240, 240, 240);
const DWORD COLOR_TEXT_SUB = RGB(160, 160, 160);
const DWORD COLOR_TEXT_WARN = RGB(255, 70, 70);
const DWORD COLOR_TEXT_HINT = RGB(80, 200, 255);

// 游戏格子元素标识
enum MapElem
{
	ELEM_EMPTY = '.',
	ELEM_WALL = '#',
	ELEM_OBSTACLE = 'O',
	ELEM_FOOD_NORMAL = 'F', // 普通食物 +10
	ELEM_FOOD_GOLD = '/', // 金苹果 +50
	ELEM_BOMB = '*', // 炸弹 -20
	ELEM_PORTAL_A = '@', //传送门A
	ELEM_PORTAL_B = '&' //传送门B
};

// 游戏状态机
enum GameState
{
	GAME_START,
	GAME_RUNNING,
	GAME_PAUSE,
	GAME_OVER
};

// 移动方向
enum Dir
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
};

// 食物/道具类型
enum FoodType
{
	FOOD_NORMAL = 0,
	FOOD_GOLD = 1,
	FOOD_BOMB = 2,
	PORTAL_A = 3,
	PORTAL_B = 4
};

// 蛇身节点（带RGB颜色，支持多皮肤）
struct SnakeNode
{
	int x, y;
	int r, g, b;
	SnakeNode(int x_ = 0, int y_ = 0, int r_ = 0, int g_ = 255, int b_ = 0)
		: x(x_), y(y_), r(r_), g(g_), b(b_) {
	}
};

// 食物结构体
struct Food
{
	int x, y;
	int type;
	Food() : x(0), y(0), type(FOOD_NORMAL) {}
};

// 全局游戏数据
extern char gameMap[25][25];
extern int score;
extern int highScore;
extern GameState gameState;
extern std::vector<SnakeNode> snake;
extern Dir curDir;
extern Dir nextDir;
extern int obstacleHitCount[25][25]; // 障碍物撞击计数(3次消失)
// 食物结构体不变，新增数组

// 食物数量常量
const int MAX_NORMAL_FOOD = 1;  // 红苹果
const int MAX_GOLD_FOOD = 1;    // 金苹果
const int MAX_BOMB_COUNT = 3;  // 炸弹

// 全局食物对象（单个苹果）
extern Food curFood;
extern std::vector<Food> bombList;

// BFS 节点
struct BFSNode
{
	int x, y;
	int step;
	BFSNode* parent;
	BFSNode(int x_, int y_, int s_, BFSNode* p) : x(x_), y(y_), step(s_), parent(p) {}
};

