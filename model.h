#pragma once
#include "constants.h"

// 初始化地图（墙、障碍物、传送门）
void InitMap();

// 初始化游戏全部数据
void InitGame();

// 蛇移动主逻辑
void MoveSnake();

// 碰撞检测：撞墙/障碍物/自身/反向移动
bool CheckCollision();

// BFS 寻路（E键辅助路径）
bool BFSFindPath(int sx, int sy, int ex, int ey, std::vector<POINT>& path);

// 技能1：闪电突袭（向前冲刺3格）
bool SkillDash();

// 技能2：原地复活（死亡后缩半重生）
bool SkillRevive();

// 加载/保存最高分（本地文件）
void LoadHighScore();
void SaveHighScore();


// 刷新所有食物（保证苹果1个、炸弹3个）
void RefreshAllFood();

// 在合适位置加入
POINT GetRandomEmptyPos();

// 新手指引页索引（GAME_START 状态下使用）
extern int tutorialPage;
extern const int TUTORIAL_PAGES;