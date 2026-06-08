#ifndef MODEL_H
#define MODEL_H

#include "types.h"
#include "constants.h"
#include <vector>

class GameModel {
public:
    // ==================== 构造与初始化接口 ====================
    GameModel();
    void initGame(); // 重置游戏到初始状态（纯数据重置）

    // ==================== 核心更新接口 ====================
    void update(); // 每帧调用一次，推进一帧游戏逻辑（纯数据计算）

    // ==================== 控制接口（纯数据操作，无IO） ====================
    bool setDirection(Direction newDir); // 设置移动方向（含防反向逻辑）
    bool useDash();       // 触发闪电突袭技能（纯数据修改）
    bool useRevive();     // 触发原地复活技能（纯数据修改）
    void toggleAssistMode(); // 切换BFS辅助模式（纯数据开关）

    // ==================== 纯数据获取接口 ====================
    const char* getMap() const;          // 获取20×20地图数组（只读）
    const std::vector<SnakeNode>& getSnake() const; // 获取蛇身节点列表（只读）
    const std::vector<Food>& getFoods() const;      // 获取所有食物/道具列表（只读）
    const std::vector<std::pair<int, int> >& getBFSPath() const; // 获取辅助模式路径（只读）
    
    int getScore() const;                // 获取当前分数
    int getHighScore() const;            // 获取历史最高分
    GameState getGameState() const;      // 获取当前游戏状态
    bool isAssistModeOn() const;         // 获取辅助模式状态

    // ==================== 状态与数据持久化接口 ====================
    void setGameState(GameState state);  // 设置游戏状态（纯数据修改）
    void saveHighScore() const;          // 保存最高分到本地文件（仅文件IO，无绘图）

private:
    // 所有实现细节完全隐藏
    char m_map[MAP_WIDTH][MAP_HEIGHT];
    std::vector<SnakeNode> m_snake;
    std::vector<Food> m_foods;
    std::vector<std::pair<int, int> > m_bfsPath;
    
    Direction m_currentDir;
    Direction m_nextDir;
    int m_score;
    int m_highScore;
    GameState m_gameState;
    int m_obstacleHits[MAP_WIDTH][MAP_HEIGHT];
    bool m_assistMode;

    // 私有核心逻辑（完全封装，外部不可见）
    void initMap();
    void initSnake();
    void generateFood();
    void generatePortalPair();
    void updateMapData();
    void moveSnake();
    void dashSnake();
    bool checkCollision(int x, int y) const;
    bool isPositionEmpty(int x, int y) const;
    void calculateBFSPath();
    std::pair<int, int> getRandomEmptyPosition();
    Direction getOppositeDirection(Direction dir) const;
    void loadHighScore();
};

#endif // MODEL_H
