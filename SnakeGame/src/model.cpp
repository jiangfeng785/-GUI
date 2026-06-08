#include "model.h"
#include <fstream>
#include <algorithm>
#include <climits>
#include <random>
#include <cstring>

// 随机数生成器（类内部私有）
static std::mt19937 g_rng(std::random_device{}());

GameModel::GameModel() {
    loadHighScore();
    initGame();
}

void GameModel::initGame() {
    m_score = 0;
    m_gameState = GAME_START;
    m_currentDir = DIR_RIGHT;
    m_nextDir = DIR_RIGHT;
    m_assistMode = false;
    m_snake.clear();
    m_foods.clear();
    m_bfsPath.clear();
    memset(m_obstacleHits, 0, sizeof(m_obstacleHits));

    initMap();
    initSnake();
    generateFood();
    updateMapData();
}

void GameModel::initMap() {
    // 填充空地
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            m_map[i][j] = MAP_EMPTY;
        }
    }

    // 绘制四周墙
    for (int i = 0; i < MAP_WIDTH; i++) {
        m_map[i][0] = MAP_WALL;
        m_map[i][MAP_HEIGHT-1] = MAP_WALL;
    }
    for (int j = 0; j < MAP_HEIGHT; j++) {
        m_map[0][j] = MAP_WALL;
        m_map[MAP_WIDTH-1][j] = MAP_WALL;
    }

    // 随机生成5个障碍物
    for (int i = 0; i < 5; i++) {
        auto pos = getRandomEmptyPosition();
        m_map[pos.first][pos.second] = MAP_OBSTACLE;
    }
}

void GameModel::initSnake() {
    int centerX = MAP_WIDTH / 2;
    int centerY = MAP_HEIGHT / 2;
    m_snake.emplace_back(centerX, centerY, 0, 255, 0);
    m_snake.emplace_back(centerX-1, centerY, 0, 200, 0);
    m_snake.emplace_back(centerX-2, centerY, 0, 150, 0);
}

std::pair<int, int> GameModel::getRandomEmptyPosition() {
    std::uniform_int_distribution<int> distX(1, MAP_WIDTH-2);
    std::uniform_int_distribution<int> distY(1, MAP_HEIGHT-2);

    int x, y;
    do {
        x = distX(g_rng);
        y = distY(g_rng);
    } while (!isPositionEmpty(x, y));

    return {x, y};
}

bool GameModel::isPositionEmpty(int x, int y) const {
    if (x <= 0 || x >= MAP_WIDTH-1 || y <= 0 || y >= MAP_HEIGHT-1) return false;
    if (m_map[x][y] != MAP_EMPTY) return false;
    
    for (const auto& node : m_snake) {
        if (node.x == x && node.y == y) return false;
    }
    for (const auto& food : m_foods) {
        if (food.x == x && food.y == y) return false;
    }
    return true;
}

void GameModel::generateFood() {
    if (m_foods.size() >= 3) return;

    std::uniform_int_distribution<int> dist(1, 100);
    int roll = dist(g_rng);
    int type;

    if (roll <= 70) type = 0;
    else if (roll <= 85) type = 1;
    else if (roll <= 95) type = 2;
    else {
        generatePortalPair();
        return;
    }

    auto pos = getRandomEmptyPosition();
    m_foods.emplace_back(pos.first, pos.second, type);
}

void GameModel::generatePortalPair() {
    auto posA = getRandomEmptyPosition();
    auto posB = getRandomEmptyPosition();
    while (posA.first == posB.first && posA.second == posB.second) {
        posB = getRandomEmptyPosition();
    }
    m_foods.emplace_back(posA.first, posA.second, 3);
    m_foods.emplace_back(posB.first, posB.second, 4);
}

bool GameModel::setDirection(Direction newDir) {
    if (newDir == DIR_NONE || newDir == getOppositeDirection(m_currentDir)) {
        return false;
    }
    m_nextDir = newDir;
    return true;
}

Direction GameModel::getOppositeDirection(Direction dir) const {
    switch (dir) {
        case DIR_UP: return DIR_DOWN;
        case DIR_DOWN: return DIR_UP;
        case DIR_LEFT: return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default: return DIR_NONE;
    }
}

void GameModel::update() {
    if (m_gameState != GAME_RUNNING) return;

    m_currentDir = m_nextDir;
    moveSnake();
    if (m_assistMode) calculateBFSPath();
    updateMapData();
}

void GameModel::moveSnake() {
    int headX = m_snake[0].x;
    int headY = m_snake[0].y;

    switch (m_currentDir) {
        case DIR_UP: headY--; break;
        case DIR_DOWN: headY++; break;
        case DIR_LEFT: headX--; break;
        case DIR_RIGHT: headX++; break;
        default: return;
    }

    if (checkCollision(headX, headY)) {
        m_gameState = GAME_OVER;
        return;
    }

    // 处理障碍物耐久度
    if (m_map[headX][headY] == MAP_OBSTACLE) {
        if (++m_obstacleHits[headX][headY] >= OBSTACLE_MAX_HITS) {
            m_map[headX][headY] = MAP_EMPTY;
        }
    }

    // 处理食物/道具碰撞
    bool grow = false;
    for (auto it = m_foods.begin(); it != m_foods.end(); ) {
        if (it->x == headX && it->y == headY) {
            switch (it->type) {
                case 0: m_score += SCORE_NORMAL_FOOD; grow = true; break;
                case 1: m_score += SCORE_GOLDEN_FOOD; grow = true; break;
                case 2: m_score = std::max(0, m_score + SCORE_BOMB_PENALTY); break;
                case 3:
                    for (const auto& p : m_foods) {
                        if (p.type == 4) { headX = p.x; headY = p.y; break; }
                    }
                    break;
                case 4:
                    for (const auto& p : m_foods) {
                        if (p.type == 3) { headX = p.x; headY = p.y; break; }
                    }
                    break;
            }
            it = m_foods.erase(it);
            generateFood();
        } else {
            ++it;
        }
    }

    // 更新蛇身
    m_snake.insert(m_snake.begin(), SnakeNode(headX, headY, 0, 255, 0));
    if (!grow) m_snake.pop_back();

    // 蛇身颜色渐变
    for (size_t i = 1; i < m_snake.size(); i++) {
        m_snake[i].g = std::max(100, 255 - (int)i * 5);
    }
}

bool GameModel::checkCollision(int x, int y) const {
    if (m_map[x][y] == MAP_WALL) return true;
    
    for (const auto& node : m_snake) {
        if (node.x == x && node.y == y) return true;
    }

    if (m_map[x][y] == MAP_OBSTACLE && m_obstacleHits[x][y] < OBSTACLE_MAX_HITS) {
        return true;
    }

    return false;
}

bool GameModel::useDash() {
    if (m_score < SKILL_DASH_COST || m_gameState != GAME_RUNNING) return false;
    m_score -= SKILL_DASH_COST;
    dashSnake();
    return true;
}

void GameModel::dashSnake() {
    int dx = 0, dy = 0;
    switch (m_currentDir) {
        case DIR_UP: dy = -1; break;
        case DIR_DOWN: dy = 1; break;
        case DIR_LEFT: dx = -1; break;
        case DIR_RIGHT: dx = 1; break;
        default: return;
    }

    int headX = m_snake[0].x;
    int headY = m_snake[0].y;
    int finalX = headX + dx * DASH_DISTANCE;
    int finalY = headY + dy * DASH_DISTANCE;

    // 终点撞墙则找最近安全位置
    if (m_map[finalX][finalY] == MAP_WALL) {
        for (int i = DASH_DISTANCE-1; i >= 1; i--) {
            int testX = headX + dx * i;
            int testY = headY + dy * i;
            if (m_map[testX][testY] != MAP_WALL) {
                finalX = testX;
                finalY = testY;
                break;
            }
        }
    }

    // 执行冲刺移动
    int steps = abs(finalX - headX) + abs(finalY - headY);
    for (int i = 0; i < steps; i++) {
        headX += dx;
        headY += dy;
        m_snake.insert(m_snake.begin(), SnakeNode(headX, headY, 0, 255, 0));
        m_snake.pop_back();
    }

    // 终点碰撞检测
    if (checkCollision(finalX, finalY)) {
        m_gameState = GAME_OVER;
        return;
    }

    // 处理终点食物
    for (auto it = m_foods.begin(); it != m_foods.end(); ) {
        if (it->x == finalX && it->y == finalY) {
            if (it->type == 0 || it->type == 1) {
                m_score += (it->type == 0) ? SCORE_NORMAL_FOOD : SCORE_GOLDEN_FOOD;
                m_snake.emplace_back(m_snake.back());
            } else if (it->type == 2) {
                m_score = std::max(0, m_score + SCORE_BOMB_PENALTY);
            }
            it = m_foods.erase(it);
            generateFood();
        } else {
            ++it;
        }
    }

    updateMapData();
}

bool GameModel::useRevive() {
    if (m_score < SKILL_REVIVE_COST || m_gameState != GAME_OVER) return false;
    m_score -= SKILL_REVIVE_COST;

    // 截断一半蛇身，至少保留3节
    size_t newLength = std::max((size_t)3, m_snake.size() / 2);
    m_snake.resize(newLength);

    m_gameState = GAME_RUNNING;
    updateMapData();
    return true;
}

void GameModel::calculateBFSPath() {
    m_bfsPath.clear();
    if (m_foods.empty() || m_snake.empty()) return;

    // 找到最近的非炸弹目标
    Food target = m_foods[0];
    int minDist = INT_MAX;
    for (const auto& food : m_foods) {
        if (food.type == 2) continue;
        int dist = abs(food.x - m_snake[0].x) + abs(food.y - m_snake[0].y);
        if (dist < minDist) {
            minDist = dist;
            target = food;
        }
    }

    // BFS实现（略，与之前版本完全一致）
    int startX = m_snake[0].x, startY = m_snake[0].y;
    int endX = target.x, endY = target.y;

    std::vector<std::vector<bool>> visited(MAP_WIDTH, std::vector<bool>(MAP_HEIGHT, false));
    std::vector<std::pair<int, int>> queue;
    std::vector<std::vector<std::pair<int, int>>> parent(MAP_WIDTH, std::vector<std::pair<int, int>>(MAP_HEIGHT, {-1, -1}));

    queue.emplace_back(startX, startY);
    visited[startX][startY] = true;

    int dirs[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};
    bool found = false;
    size_t front = 0;

    while (front < queue.size()) {
        auto [x, y] = queue[front++];
        if (x == endX && y == endY) { found = true; break; }

        for (int i = 0; i < 4; i++) {
            int nx = x + dirs[i][0];
            int ny = y + dirs[i][1];

            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT &&
                !visited[nx][ny] && m_map[nx][ny] != MAP_WALL &&
                m_map[nx][ny] != MAP_OBSTACLE) {
                // 排除蛇身（除蛇尾）
                bool isBody = false;
                for (size_t j = 0; j < m_snake.size()-1; j++) {
                    if (m_snake[j].x == nx && m_snake[j].y == ny) {
                        isBody = true;
                        break;
                    }
                }
                if (!isBody) {
                    visited[nx][ny] = true;
                    parent[nx][ny] = {x, y};
                    queue.emplace_back(nx, ny);
                }
            }
        }
    }

    // 回溯路径
    if (found) {
        int x = endX, y = endY;
        while (x != -1 && y != -1) {
            m_bfsPath.emplace_back(x, y);
            auto [px, py] = parent[x][y];
            x = px; y = py;
        }
        std::reverse(m_bfsPath.begin(), m_bfsPath.end());
    }
}

void GameModel::toggleAssistMode() {
    m_assistMode = !m_assistMode;
    if (m_assistMode) calculateBFSPath();
    else m_bfsPath.clear();
}

void GameModel::updateMapData() {
    // 重置非墙/障碍物区域
    for (int i = 1; i < MAP_WIDTH-1; i++) {
        for (int j = 1; j < MAP_HEIGHT-1; j++) {
            if (m_map[i][j] != MAP_WALL && m_map[i][j] != MAP_OBSTACLE) {
                m_map[i][j] = MAP_EMPTY;
            }
        }
    }

    // 绘制食物
    for (const auto& food : m_foods) {
        switch (food.type) {
            case 0: m_map[food.x][food.y] = MAP_FOOD_NORMAL; break;
            case 1: m_map[food.x][food.y] = MAP_FOOD_GOLDEN; break;
            case 2: m_map[food.x][food.y] = MAP_BOMB; break;
            case 3: m_map[food.x][food.y] = MAP_PORTAL_A; break;
            case 4: m_map[food.x][food.y] = MAP_PORTAL_B; break;
        }
    }

    // 绘制蛇
    if (!m_snake.empty()) {
        m_map[m_snake[0].x][m_snake[0].y] = MAP_SNAKE_HEAD;
        for (size_t i = 1; i < m_snake.size(); i++) {
            m_map[m_snake[i].x][m_snake[i].y] = MAP_SNAKE_BODY;
        }
    }
}

void GameModel::loadHighScore() {
    std::ifstream file("highscore.txt");
    m_highScore = (file.is_open()) ? (file >> m_highScore, m_highScore) : 0;
}

void GameModel::saveHighScore() const {
    if (m_score > m_highScore) {
        std::ofstream file("highscore.txt");
        if (file.is_open()) file << m_score;
    }
}

// ==================== 纯数据Getter实现 ====================
const char* GameModel::getMap() const { return &m_map[0][0]; }
const std::vector<SnakeNode>& GameModel::getSnake() const { return m_snake; }
const std::vector<Food>& GameModel::getFoods() const { return m_foods; }
const std::vector<std::pair<int, int>>& GameModel::getBFSPath() const { return m_bfsPath; }
int GameModel::getScore() const { return m_score; }
int GameModel::getHighScore() const { return m_highScore; }
GameState GameModel::getGameState() const { return m_gameState; }
bool GameModel::isAssistModeOn() const { return m_assistMode; }
void GameModel::setGameState(GameState state) { m_gameState = state; }
