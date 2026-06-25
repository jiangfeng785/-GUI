#include "model.h"
#include "constants.h"
#include "audio.h"
#include <queue>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <windows.h> // for POINT

int gameLevel = 1;   // 初始关卡为1

// 前向声明，InitMap 中会使用
bool IsNearSnakeHead(int x, int y);

// 全局食物变量（单个苹果）
Food curFood;
std::vector<Food> bombList;

// 全局基础游戏变量
char gameMap[25][25] = { 0 };
int score = 0;
int highScore = 0;
GameState gameState = GAME_START;
std::vector<SnakeNode> snake;
Dir curDir = DIR_UP;
Dir nextDir = DIR_UP;
int obstacleHitCount[25][25] = { 0 };

// 新手指引页索引（开始页使用）
int tutorialPage = 0;
const int TUTORIAL_PAGES = 3; //0..2 共3页

//记录移动步数，用于每10步刷新炸弹位置
static int moveStepCount = 0;

//方向偏移量 上、下、左、右
const int dx[] = { 0,0, -1,1 };
const int dy[] = { -1,1,0,0 };

// AI相关变量
std::vector<POINT> aiPath;
bool aiActive = false;
size_t aiPathIndex = 0;

// 寻路辅助显示（E键生成，高亮显示在暂停时）
std::vector<POINT> helperPath;
bool showHelperPath = false;

// 将所有炸弹重新随机安放（保持数量不变）
static void RelocateBombs()
{
    for (size_t i = 0; i < bombList.size(); ++i)
    {
        POINT p;
        // 找到一个合适的空位，避免与蛇、苹果、传送门、障碍、墙重叠
        do {
            p = GetRandomEmptyPos();
        } while (p.x == curFood.x && p.y == curFood.y);
        bombList[i].x = p.x;
        bombList[i].y = p.y;
        bombList[i].type = FOOD_BOMB;
    }
}

void InitMap()
{
    // 全部初始化为空地
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            gameMap[i][j] = ELEM_EMPTY;
            obstacleHitCount[i][j] = 0;
        }
    }

    // 外围围墙
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        gameMap[0][i] = ELEM_WALL;
        gameMap[MAP_HEIGHT - 1][i] = ELEM_WALL;
    }
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        gameMap[i][0] = ELEM_WALL;
        gameMap[i][MAP_WIDTH - 1] = ELEM_WALL;
    }

    // 随机生成若干障碍物（位置每次游戏不同）
    const int OBSTACLE_COUNT = GetObstacleCount();// 保持与之前类似数量
    int placed = 0;
    while (placed < OBSTACLE_COUNT)
    {
        int x = rand() % (MAP_WIDTH - 2) + 1;
        int y = rand() % (MAP_HEIGHT - 2) + 1;
        // 避免放在蛇的位置或重复
        bool onSnake = false;
        for (auto& n : snake) if (n.x == x && n.y == y) { onSnake = true; break; }
        if (onSnake) continue;
        if (gameMap[y][x] == ELEM_EMPTY)
        {
            gameMap[y][x] = ELEM_OBSTACLE;
            placed++;
        }
    }

    // 随机成对放置传送门，要求：不在死胡同且两门距离较远以便发挥作用
    auto IsDeadEnd = [&](int x, int y) {
        int freeCount = 0;
        const int nx[4] = { 0,0, -1,1 };
        const int ny[4] = { -1,1,0,0 };
        for (int d = 0; d < 4; d++)
        {
            int tx = x + nx[d];
            int ty = y + ny[d];
            if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT) continue;
            char t = gameMap[ty][tx];
            if (t != ELEM_WALL && t != ELEM_OBSTACLE) freeCount++;
        }
        return freeCount <= 1; //0 或1 个合法邻居都视为死胡同
        };

    POINT pa, pb;
    int attempts = 0;
    const int maxAttempts = 1000;
    int minDist = (MAP_WIDTH > MAP_HEIGHT ? MAP_WIDTH : MAP_HEIGHT) / 2; // 最小曼哈顿距离

    // 找到第一个合适位置
    attempts = 0;
    do {
        pa.x = rand() % (MAP_WIDTH - 2) + 1; pa.y = rand() % (MAP_HEIGHT - 2) + 1;
        attempts++;
    } while ((gameMap[pa.y][pa.x] != ELEM_EMPTY || IsNearSnakeHead(pa.x, pa.y) || IsDeadEnd(pa.x, pa.y)) && attempts < maxAttempts);
    if (gameMap[pa.y][pa.x] == ELEM_EMPTY && !IsDeadEnd(pa.x, pa.y))
        gameMap[pa.y][pa.x] = ELEM_PORTAL_A;

    // 找到第二个合适位置，确保与第一个足够远且同样不在死胡同
    attempts = 0;
    do {
        pb.x = rand() % (MAP_WIDTH - 2) + 1; pb.y = rand() % (MAP_HEIGHT - 2) + 1;
        attempts++;
    } while ((gameMap[pb.y][pb.x] != ELEM_EMPTY || IsNearSnakeHead(pb.x, pb.y) || IsDeadEnd(pb.x, pb.y)
        || (abs(pb.x - pa.x) + abs(pb.y - pa.y) < minDist)) && attempts < maxAttempts);
    if (gameMap[pb.y][pb.x] == ELEM_EMPTY && !IsDeadEnd(pb.x, pb.y)
        && (abs(pb.x - pa.x) + abs(pb.y - pa.y) >= minDist))
        gameMap[pb.y][pb.x] = ELEM_PORTAL_B;

    // 若未能满足距离或死胡同条件，则回退到任意空位，保证至少成对存在
    if (gameMap[pa.y][pa.x] != ELEM_PORTAL_A)
    {
        // 强制放置A在任意空位
        for (int y = 1; y < MAP_HEIGHT - 1 && gameMap[pa.y][pa.x] != ELEM_PORTAL_A; y++)
            for (int x = 1; x < MAP_WIDTH - 1 && gameMap[pa.y][pa.x] != ELEM_PORTAL_A; x++)
                if (gameMap[y][x] == ELEM_EMPTY && !IsNearSnakeHead(x, y)) { gameMap[y][x] = ELEM_PORTAL_A; pa.x = x; pa.y = y; break; }
    }
    if (gameMap[pb.y][pb.x] != ELEM_PORTAL_B)
    {
        // 强制放置B为距离较远的空位
        for (int y = MAP_HEIGHT - 2; y >= 1 && gameMap[pb.y][pb.x] != ELEM_PORTAL_B; y--)
            for (int x = MAP_WIDTH - 2; x >= 1 && gameMap[pb.y][pb.x] != ELEM_PORTAL_B; x--)
                if (gameMap[y][x] == ELEM_EMPTY && !IsNearSnakeHead(x, y) && (abs(x - pa.x) + abs(y - pa.y) >= minDist)) { gameMap[y][x] = ELEM_PORTAL_B; pb.x = x; pb.y = y; break; }
        // 如果仍未找到，放在任意空位
        if (gameMap[pb.y][pb.x] != ELEM_PORTAL_B)
        {
            for (int y = 1; y < MAP_HEIGHT - 1 && gameMap[pb.y][pb.x] != ELEM_PORTAL_B; y++)
                for (int x = 1; x < MAP_WIDTH - 1 && gameMap[pb.y][pb.x] != ELEM_PORTAL_B; x++)
                    if (gameMap[y][x] == ELEM_EMPTY && !IsNearSnakeHead(x, y)) { gameMap[y][x] = ELEM_PORTAL_B; pb.x = x; pb.y = y; break; }
        }
    }
}

void InitGame()
{
    gameLevel = 1;   // 新游戏从头开始
    srand((unsigned int)time(NULL));
    score = 0;
    curDir = DIR_UP;
    nextDir = DIR_UP;
    snake.clear();
    bombList.clear();
    moveStepCount = 0;

    // 蛇出生在地图中心
    int startX = MAP_WIDTH / 2;
    int startY = MAP_HEIGHT / 2;
    for (int i = 0; i < 3; i++)
    {
        snake.emplace_back(startX, startY + i);
    }

    InitMap();
    RefreshAllFood(); // 开局生成1苹果+3炸弹
    gameState = GAME_RUNNING;
}

bool CheckCollision()
{
    SnakeNode head = snake[0];
    int nx = head.x + dx[nextDir];
    int ny = head.y + dy[nextDir];

    // 禁止直接反向掉头
    if ((curDir == DIR_UP && nextDir == DIR_DOWN) ||
        (curDir == DIR_DOWN && nextDir == DIR_UP) ||
        (curDir == DIR_LEFT && nextDir == DIR_RIGHT) ||
        (curDir == DIR_RIGHT && nextDir == DIR_LEFT))
    {
        return true;
    }

    // 【重要修复】越界检测：防止蛇身走出地图边界，从而避免闯入右侧UI区
    if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT)
        return true;

    // 撞墙
    if (gameMap[ny][nx] == ELEM_WALL)
        return true;

    // 撞击障碍物，撞击3次障碍物会消失
    if (gameMap[ny][nx] == ELEM_OBSTACLE)
    {
        obstacleHitCount[ny][nx]++;
        if (obstacleHitCount[ny][nx] >= 3)
            gameMap[ny][nx] = ELEM_EMPTY;
        return true;
    }

    // 撞到自身身体
    for (auto& node : snake)
    {
        if (node.x == nx && node.y == ny)
            return true;
    }
    return false;
}

void MoveSnake()
{
    if (gameState != GAME_RUNNING) return;

    // 如果AI激活，由AI路径决定下一步方向
    if (aiActive && aiPathIndex < aiPath.size())
    {
        POINT target = aiPath[aiPathIndex];
        int hx = snake[0].x;
        int hy = snake[0].y;
        if (target.x == hx && target.y == hy)
        {
            // 已经在目标格，推进索引
            aiPathIndex++;
            if (aiPathIndex >= aiPath.size()) { aiActive = false; }
        }
        else
        {
            if (target.x > hx) nextDir = DIR_RIGHT;
            else if (target.x < hx) nextDir = DIR_LEFT;
            else if (target.y > hy) nextDir = DIR_DOWN;
            else if (target.y < hy) nextDir = DIR_UP;
        }
    }

    curDir = nextDir;
    if (CheckCollision())
    {
        // 播放死亡音效并结束游戏
        PlayDeathSound();
        gameState = GAME_OVER;
        return;
    }

    SnakeNode head = snake[0];
    int nx = head.x + dx[curDir];
    int ny = head.y + dy[curDir];

    //传送门跳转逻辑
    if (gameMap[ny][nx] == ELEM_PORTAL_A)
    {
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            for (int j = 0; j < MAP_WIDTH; j++)
            {
                if (gameMap[i][j] == ELEM_PORTAL_B)
                {
                    nx = j;
                    ny = i;
                    // 播放传送门音效
                    PlayPortalSound();
                    goto endPortal;
                }
            }
        }
    }
    else if (gameMap[ny][nx] == ELEM_PORTAL_B)
    {
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            for (int j = 0; j < MAP_WIDTH; j++)
            {
                if (gameMap[i][j] == ELEM_PORTAL_A)
                {
                    nx = j;
                    ny = i;
                    // 播放传送门音效
                    PlayPortalSound();
                    goto endPortal;
                }
            }
        }
    }
endPortal:

    // 插入新蛇头
    SnakeNode newHead(nx, ny);
    snake.insert(snake.begin(), newHead);

    // 如果AI激活并且到达当前ai目标索引位置则推进
    if (aiActive && aiPathIndex < aiPath.size())
    {
        if (snake[0].x == aiPath[aiPathIndex].x && snake[0].y == aiPath[aiPathIndex].y)
        {
            aiPathIndex++;
            if (aiPathIndex >= aiPath.size()) aiActive = false;
        }
    }

    bool eatFood = false;
    int headX = snake[0].x;
    int headY = snake[0].y;

    // 碰撞苹果
    if (headX == curFood.x && headY == curFood.y)
    {
        if (curFood.type == FOOD_NORMAL)
        {
            PlayAppleSoundRed();
            score += 10;
        }
            
        else if (curFood.type == FOOD_GOLD)
        {
            score += 50;
            PlayAppleSoundGolden();
        }
            

        

        // 重置苹果位置与类型（随机为红或金）
        POINT newPos = GetRandomEmptyPos();
        curFood.x = newPos.x;
        curFood.y = newPos.y;
        curFood.type = (rand() % 2 == 0) ? FOOD_NORMAL : FOOD_GOLD;
        eatFood = true;

        // 吃到苹果后如果之前显示了高亮寻路路径，则清除它
        helperPath.clear();
        showHelperPath = false;
    }

    // 碰撞炸弹 -30分，删除并补新炸弹维持3个
    for (size_t i = 0; i < bombList.size(); i++)
    {
        if (headX == bombList[i].x && headY == bombList[i].y)
        {
            score -= 30;
            // 播放炸弹爆炸音效
            PlayBombSound();
            bombList.erase(bombList.begin() + i);
            POINT newBombPos = GetRandomEmptyPos();
            Food newB;
            newB.x = newBombPos.x;
            newB.y = newBombPos.y;
            newB.type = FOOD_BOMB;
            bombList.push_back(newB);
            break;
        }
    }

    // 如果分数小于0，判定游戏结束
    if (score < 0)
    {
        PlayDeathSound();
        gameState = GAME_OVER;
        return;
    }

    // 没吃到食物就删掉尾巴，长度不变；吃到则保留尾巴实现变长
    if (!eatFood)
        snake.pop_back();

    // 步数统计并每10步刷新炸弹位置
    moveStepCount++;
    if (moveStepCount % 10 == 0)
    {
        RelocateBombs();
    }
}

// BFS寻路辅助
bool BFSFindPath(int sx, int sy, int ex, int ey, std::vector<POINT>& path)
{
    path.clear();

    // blocked map: walls, obstacles, bombs, snake body, and portals
    bool blocked[25][25] = { false };
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            // 将墙、障碍物、传送门都视为不可通行
            if (gameMap[y][x] == ELEM_WALL || gameMap[y][x] == ELEM_OBSTACLE ||
                gameMap[y][x] == ELEM_PORTAL_A || gameMap[y][x] == ELEM_PORTAL_B)
                blocked[y][x] = true;
        }
    }
    // mark bombs as blocked
    for (auto& b : bombList)
    {
        if (b.x >= 0 && b.x < MAP_WIDTH && b.y >= 0 && b.y < MAP_HEIGHT)
            blocked[b.y][b.x] = true;
    }
    // mark snake body as blocked (avoid path through self)
    for (size_t i = 0; i < snake.size(); ++i)
    {
        int bx = snake[i].x;
        int by = snake[i].y;
        if (bx >= 0 && bx < MAP_WIDTH && by >= 0 && by < MAP_HEIGHT)
            blocked[by][bx] = true;
    }
    // allow start position even if it's on snake head (head itself is passable)
    if (sx >= 0 && sx < MAP_WIDTH && sy >= 0 && sy < MAP_HEIGHT)
        blocked[sy][sx] = false;

    bool vis[25][25] = { false };
    std::queue<BFSNode*> q;
    std::vector<BFSNode*> allocated;

    BFSNode* start = new BFSNode(sx, sy, 0, nullptr);
    q.push(start);
    allocated.push_back(start);
    vis[sy][sx] = true;

    int dirX[] = { 0, 0, -1, 1 };
    int dirY[] = { -1, 1, 0, 0 };

    while (!q.empty())
    {
        BFSNode* now = q.front();
        q.pop();

        if (now->x == ex && now->y == ey)
        {
            BFSNode* p = now;
            while (p)
            {
                path.push_back({ p->x, p->y });
                p = p->parent;
            }
            std::reverse(path.begin(), path.end());
            for (auto n : allocated) delete n;
            return true;
        }

        for (int d = 0; d < 4; d++)
        {
            int nx = now->x + dirX[d];
            int ny = now->y + dirY[d];
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT
                && !vis[ny][nx] && !blocked[ny][nx])
            {
                vis[ny][nx] = true;
                BFSNode* node = new BFSNode(nx, ny, now->step + 1, now);
                q.push(node);
                allocated.push_back(node);
            }
        }
    }

    // 未找到路径，清理内存
    for (auto n : allocated) delete n;
    return false;
}

// 闪电冲刺 消耗50分向前冲3格
bool SkillDash()
{
    if (score < 50 || gameState != GAME_RUNNING) return false;
    score -= 50;
    SnakeNode head = snake[0];
    int nx = head.x, ny = head.y;
    for (int i = 0; i < 3; i++)
    {
        nx += dx[curDir];
        ny += dy[curDir];
        if (gameMap[ny][nx] == ELEM_WALL) break;
    }
    snake.insert(snake.begin(), SnakeNode(nx, ny));
    snake.pop_back();
    return true;
}

// 原地复活 消耗500分，蛇缩一半长度
bool SkillRevive()
{
    if (score < 500 || gameState != GAME_OVER) return false;
    score -= 500;
    int half = snake.size() / 2;
    while (snake.size() > half)
        snake.pop_back();
    gameState = GAME_RUNNING;
    return true;
}

//读写最高分文件
void LoadHighScore()
{
    std::ifstream fin("highscore.txt");
    if (fin.is_open())
    {
        fin >> highScore;
        fin.close();
    }
    else
        highScore = 0;
}

void SaveHighScore()
{
    if (score > highScore)
    {
        highScore = score;
        std::ofstream fout("highscore.txt");
        fout << highScore;
        fout.close();
    }
}

// 判断坐标是否紧贴蛇头上下左右
bool IsNearSnakeHead(int x, int y)
{
    int headX = snake[0].x;
    int headY = snake[0].y;
    if ((x == headX && y == headY - 1) ||
        (x == headX && y == headY + 1) ||
        (x == headX - 1 && y == headY) ||
        (x == headX + 1 && y == headY))
    {
        return true;
    }
    return false;
}

// 获取安全空白坐标（过滤墙、障碍、蛇身、蛇头四周）
POINT GetRandomEmptyPos()
{
    POINT res;
    while (true)
    {
        res.x = rand() % MAP_WIDTH;
        res.y = rand() % MAP_HEIGHT;

        //过滤墙体、障碍、传送门
        char tile = gameMap[res.y][res.x];
        if (tile == ELEM_WALL || tile == ELEM_OBSTACLE || tile == ELEM_PORTAL_A || tile == ELEM_PORTAL_B)
            continue;

        //过滤蛇全身
        bool isSnakeBody = false;
        for (size_t i = 0; i < snake.size(); i++)
        {
            if (snake[i].x == res.x && snake[i].y == res.y)
            {
                isSnakeBody = true;
                break;
            }
        }
        if (isSnakeBody) continue;

        //过滤蛇头四邻格
        if (IsNearSnakeHead(res.x, res.y))
            continue;

        break;
    }
    return res;
}

// 刷新全部道具：红苹果1、金苹果1、炸弹固定3个
void RefreshAllFood()
{
    POINT pRed = GetRandomEmptyPos();
    curFood.x = pRed.x;
    curFood.y = pRed.y;
    curFood.type = FOOD_NORMAL;

    bombList.clear();
    for (int i = 0; i < MAX_BOMB_COUNT; i++)
    {
        POINT pBomb = GetRandomEmptyPos();
        Food b;
        b.x = pBomb.x;
        b.y = pBomb.y;
        b.type = FOOD_BOMB;
        bombList.push_back(b);
    }
}

// 障碍物数量 = 4 + (关卡-1)*2，上限20
int GetObstacleCount()
{
    int count = 4 + (gameLevel - 1) * 2;
    if (count > 20) count = 20;
    return count;
}

// 炸弹刷新步数间隔 = 10 - (关卡-1)，最少3步
int GetBombRefreshStep()
{
    int step = 10 - (gameLevel - 1);
    if (step < 3) step = 3;
    return step;
}

// 蛇移动间隔 = 400 - (关卡-1)*20，最少100ms
int GetMoveInterval()
{
    int interval = 400 - (gameLevel - 1) * 20;
    if (interval < 100) interval = 100;
    return interval;
}

// 检查关卡
void CheckLevel()
{
    if (score / (gameLevel * 200) == 1)
    {
        InitMap();
        RefreshAllFood();
		gameLevel++;
    }
    return;
}