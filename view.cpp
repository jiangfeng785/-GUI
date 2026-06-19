#include "view.h"
#include "constants.h"
#include "model.h"
#include <windows.h>
#include <string>
#include <io.h>

IMAGE imgWall;        // 红砖墙
IMAGE imgSnakeHead;   // 蛇头
IMAGE imgAppleRed;     // 红苹果
IMAGE imgAppleGold;   // 金苹果
IMAGE imgBomb;        // 炸弹
IMAGE imgPortal;      // 传送门



// 全局标记：图片是否已加载
bool bResLoaded = false;

static std::wstring GetExeDir()
{
    wchar_t buf[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    std::wstring s(buf);
    size_t pos = s.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        s = s.substr(0, pos);
    return s;
}

static std::wstring FindImageFile(const wchar_t* name)
{
    std::wstring exe = GetExeDir();
    std::wstring candidates[] = { exe + L"\\" + name, exe + L"\\res\\" + name, std::wstring(name) };
    for (auto &c : candidates)
    {
        if (_waccess(c.c_str(),0) ==0)
            return c;
    }
    // fallback to the raw name (let loadimage decide)
    return candidates[2];
}

void InitView()
{
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    setbkcolor(RGB(20, 20, 20));
    cleardevice();
    settextstyle(18, 0, _T("微软雅黑"));

    //加载图片资源（仅首次运行加载）
    if (!bResLoaded)
    {
        // 加载图片，参数：图片对象, 图片路径, 宽, 高
        loadimage(&imgWall, FindImageFile(L"wall.png").c_str(), CELL_SIZE, 27, 0);
        loadimage(&imgSnakeHead, FindImageFile(L"snake_head.png").c_str(), CELL_SIZE, CELL_SIZE,0);
        loadimage(&imgAppleRed, FindImageFile(L"apple_red.png").c_str(), CELL_SIZE, CELL_SIZE,0);
        loadimage(&imgAppleGold, FindImageFile(L"apple_gold.png").c_str(), CELL_SIZE, CELL_SIZE,0);
        loadimage(&imgBomb, FindImageFile(L"bomb.png").c_str(), CELL_SIZE, CELL_SIZE, 0);
        loadimage(&imgPortal, FindImageFile(L"portal.png").c_str(), CELL_SIZE, CELL_SIZE, 0);

        bResLoaded = true;
    }
}

void RenderMap()
{
    // 先铺满全局背景
    setfillcolor(COLOR_BG);
    solidrectangle(0, 0, MAP_WIDTH * CELL_SIZE, MAP_HEIGHT * CELL_SIZE);

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            int px = x * CELL_SIZE;
            int py = y * CELL_SIZE;
            switch (gameMap[y][x])
            {
            case ELEM_WALL:
                // 【修改】红砖墙体贴图
                putimage(px, py, &imgWall);
                break;

            case ELEM_OBSTACLE:
                // 障碍物保留原有纯色样式（你没要求改，可不动）
                setfillcolor(COLOR_OBSTACLE);
                solidrectangle(px + 4, py + 4, px + CELL_SIZE - 4, py + CELL_SIZE - 4);
                setlinecolor(RGB(140, 90, 60));
                rectangle(px + 4, py + 4, px + CELL_SIZE - 4, py + CELL_SIZE - 4);
                break;

            case ELEM_PORTAL_A:
            case ELEM_PORTAL_B:
                // 【修改】紫色漩涡传送门贴图
                putimage(px, py, &imgPortal);
                break;

            default:
                // 空白格子保留网格线
                setlinecolor(COLOR_GRID);
                rectangle(px, py, px + CELL_SIZE, py + CELL_SIZE);
                break;
            }
        }
    }
}

void RenderSnake()
{
    for (size_t i = 0; i < snake.size(); i++)
    {
        int px = snake[i].x * CELL_SIZE;
        int py = snake[i].y * CELL_SIZE;

        // 多皮肤规则
        if (score > 500)
        {
            // 赛博机甲蛇 霓虹蓝
            setfillcolor(RGB(0, 200, 255));
        }
        else if (score > 100)
        {
            // 金色神龙
            setfillcolor(RGB(255, 215, 0));
        }
        else
        {
            // 初始绿蛇
            setfillcolor(RGB(0, 220, 0));
        }

        if (i == 0)
        {
            // 蛇头圆形
            solidcircle(px + CELL_SIZE / 2, py + CELL_SIZE / 2, CELL_SIZE / 2 - 2);
            // 眼睛
            setfillcolor(BLACK);
            solidcircle(px + 8, py + 8, 3);
            solidcircle(px + CELL_SIZE - 8, py + 8, 3);
        }
        else
        {
            solidrectangle(px + 3, py + 3, px + CELL_SIZE - 3, py + CELL_SIZE - 3);
        }
    }
}

void RenderFood()
{
    // 单个苹果，根据类型选择贴图或颜色
    int fx = curFood.x * CELL_SIZE;
    int fy = curFood.y * CELL_SIZE;
    if (curFood.type == FOOD_GOLD)
    {
        putimage(fx, fy, &imgAppleGold);
    }
    else
    {
        putimage(fx, fy, &imgAppleRed);
    }

    // 绘制炸弹
    for (size_t i = 0; i < bombList.size(); i++)
    {
        int bx = bombList[i].x * CELL_SIZE;
        int by = bombList[i].y * CELL_SIZE;
        putimage(bx, by, &imgBomb);
    }
}

void RenderUI()
{
    int uiX = MAP_WIDTH * CELL_SIZE + 10;
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(24, 0, _T("微软雅黑"));

    outtextxy(uiX, 20, _T("贪吃蛇加强版"));
    settextstyle(18, 0, _T("微软雅黑"));

    TCHAR buf[64];
    _stprintf_s(buf, _T("当前分数：%d"), score);
    outtextxy(uiX, 80, buf);

    _stprintf_s(buf, _T("最高分数：%d"), highScore);
    outtextxy(uiX, 120, buf);

    // 状态提示
    if (gameState == GAME_PAUSE)
        outtextxy(uiX, 180, _T("【已暂停】空格继续"));
    else if (gameState == GAME_OVER)
        outtextxy(uiX, 180, _T("游戏结束！R重开"));
    else if (gameState == GAME_START)
        outtextxy(uiX, 180, _T("按任意键开始"));

    outtextxy(uiX, 240, _T("E:寻路辅助"));
    outtextxy(uiX, 280, _T("Q:闪电冲刺(-50分)"));
    outtextxy(uiX, 320, _T("F:原地复活(-500分)"));
    outtextxy(uiX, 360, _T("ESC:退出游戏"));
}

void RenderTutorial(int page)
{
    int centerX = (MAP_WIDTH * CELL_SIZE) /2;
    int centerY = (MAP_HEIGHT * CELL_SIZE) /2;
    settextstyle(20, 0, _T("微软雅黑"));
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    switch (page)
    {
    case 0:
        outtextxy(centerX -100, centerY -60, _T("欢迎来到贪吃蛇加强版！"));
        outtextxy(centerX -120, centerY -30, _T("WASD 控制移动，空格暂停/继续"));
        outtextxy(centerX -120, centerY, _T("按任意键开始游戏"));
        break;
    case 1:
        outtextxy(centerX -120, centerY -60, _T("吃红苹果 +10，吃金苹果 +50"));
        outtextxy(centerX -120, centerY -30, _T("撞炸弹 -30；撞障碍3次可摧毁"));
        outtextxy(centerX -120, centerY, _T("E: 寻路辅助；Q: 闪电冲刺；F: 原地复活"));
        break;
    case 2:
        outtextxy(centerX -120, centerY -60, _T("地图每局随机生成，炸弹每10步刷新"));
        outtextxy(centerX -120, centerY -30, _T("传送门会把你传到另一端"));
        outtextxy(centerX -120, centerY, _T("祝你游戏愉快！按任意键继续"));
        break;
    }
}

void NextTutorialPage()
{
    extern int tutorialPage;
    extern const int TUTORIAL_PAGES;
    tutorialPage++;
    if (tutorialPage >= TUTORIAL_PAGES)
    {
        // 当越过最后一页时，开始游戏
        tutorialPage = 0;
        InitGame();
    }
}

void RenderHelperPath(const std::vector<POINT>& path)
{
    setfillcolor(RGB(0, 150, 255));
    for (auto& p : path)
    {
        int x = p.x * CELL_SIZE + CELL_SIZE / 2;
        int y = p.y * CELL_SIZE + CELL_SIZE / 2;
        solidcircle(x, y, 4);
    }
}

void RenderAll()
{
    cleardevice();
    RenderMap();

    if (gameState == GAME_START)
    {
        extern int tutorialPage;
        RenderTutorial(tutorialPage);
    }
    else
    {
        RenderFood();
        RenderSnake();
        RenderUI();
    }
}

void CloseView()
{
    closegraph();
}