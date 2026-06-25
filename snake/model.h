#pragma once
#include "constants.h"
#include <vector>

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

// 获取随机空白位置（用于放置食物/炸弹）
POINT GetRandomEmptyPos();

// 新手指引页索引（GAME_START 状态下使用）
extern int tutorialPage;
extern const int TUTORIAL_PAGES;

// AI 路径控制（由 E 键触发的原始实现）
extern std::vector<POINT> aiPath;
extern bool aiActive;
extern size_t aiPathIndex;

// 寻路辅助显示（新手提示/暂停时显示路径）
extern std::vector<POINT> helperPath;
extern bool showHelperPath;

// ========== 关卡系统动态参数 ==========
extern int gameLevel;                     // 当前关卡（从1开始）

// 根据当前关卡获取动态参数
int GetObstacleCount();                   // 障碍物数量
int GetBombRefreshStep();                 // 炸弹刷新步数间隔
int GetMoveInterval();                    // 蛇移动间隔（毫秒）