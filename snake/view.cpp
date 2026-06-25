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
    wchar_t buf[MAX_PATH] = { 0 };
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
    for (auto& c : candidates)
    {
        if (_waccess(c.c_str(), 0) == 0)
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
        loadimage(&imgSnakeHead, FindImageFile(L"snake_head.png").c_str(), CELL_SIZE, CELL_SIZE, 0);
        loadimage(&imgAppleRed, FindImageFile(L"apple_red.png").c_str(), CELL_SIZE, CELL_SIZE, 0);
        loadimage(&imgAppleGold, FindImageFile(L"apple_gold.png").c_str(), CELL_SIZE, CELL_SIZE, 0);
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

    outtextxy(uiX, 20, _T("贪吃蛇"));
    settextstyle(18, 0, _T("微软雅黑"));

    TCHAR buf[64];
    _stprintf_s(buf, _T("当前分数：%d"), score);
    outtextxy(uiX, 150, buf);   // 注意调整Y坐标避免重叠（原状态提示在180）

    _stprintf_s(buf, _T("当前关卡：%d"), gameLevel);
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
    // ---------- 背景 ----------
    setbkcolor(RGB(18, 28, 46));
    cleardevice();

    // 网格线
    setlinecolor(RGB(50, 60, 80));
    for (int i = 0; i <= MAP_WIDTH; i++)
        line(i * CELL_SIZE, 0, i * CELL_SIZE, WIN_HEIGHT);
    for (int i = 0; i <= MAP_HEIGHT; i++)
        line(0, i * CELL_SIZE, MAP_WIDTH * CELL_SIZE, i * CELL_SIZE);

    // ---------- 主面板 ----------
    int panelW = 760;
    int panelH = 540;                // 高度稍增，给内容更多空间
    int panelL = (WIN_WIDTH - panelW) / 2;
    int panelT = (WIN_HEIGHT - panelH) / 2;
    int panelR = panelL + panelW;
    int panelB = panelT + panelH;
    int radius = 30;

    setfillcolor(RGB(10, 15, 25));
    fillroundrect(panelL + 10, panelT + 10, panelR + 10, panelB + 10, radius, radius);

    setfillcolor(RGB(30, 45, 70));
    setlinecolor(RGB(70, 180, 255));
    fillroundrect(panelL, panelT, panelR, panelB, radius, radius);
    setlinecolor(RGB(50, 140, 200));
    roundrect(panelL, panelT, panelR, panelB, radius, radius);

    setbkmode(TRANSPARENT);

    // ---------- 绘制蛇头（手动绘制，不依赖图片） ----------
    int headSize = 48;               // 蛇头大小
    int headX = panelL + 30;         // 左上角X
    int headY = panelT + 25;         // 左上角Y
    int centerX = headX + headSize / 2;
    int centerY = headY + headSize / 2;

    // 画圆形背景
    setfillcolor(RGB(50, 70, 100));
    setlinecolor(RGB(100, 180, 255));
    solidcircle(centerX, centerY, headSize / 2 + 4);

    // 蛇头主体（绿色圆形）
    setfillcolor(RGB(0, 220, 0));
    setlinecolor(RGB(0, 180, 0));
    solidcircle(centerX, centerY, headSize / 2 - 2);

    // 眼睛（白色底）
    setfillcolor(WHITE);
    solidcircle(centerX - 10, centerY - 6, 8);
    solidcircle(centerX + 10, centerY - 6, 8);

    // 瞳孔（黑色）
    setfillcolor(BLACK);
    solidcircle(centerX - 12, centerY - 8, 4);
    solidcircle(centerX + 8, centerY - 8, 4);

    // 舌头（红色分叉）
    setlinecolor(RGB(255, 0, 0));
    setlinestyle(PS_SOLID, 3);
    line(centerX, centerY + 12, centerX - 6, centerY + 22);
    line(centerX, centerY + 12, centerX + 6, centerY + 22);
    setlinestyle(PS_SOLID, 1); // 重置线宽

    // ---------- 标题文字 ----------
    settextstyle(46, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 215, 0));
    outtextxy(panelL + 100, panelT + 28, _T("贪吃蛇"));

    // 副标题
    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(160, 200, 255));
    outtextxy(panelL + 100, panelT + 80, _T("经典贪吃蛇 · 鼠标与键盘双控"));

    // 最高分
    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 200, 100));
    TCHAR scoreBuf[64];
    _stprintf_s(scoreBuf, _T("🏆 最高分：%d"), highScore);
    int scoreX = panelR - textwidth(scoreBuf) - 25;
    outtextxy(scoreX, panelT + 30, scoreBuf);

    // ---------- 内容区 ----------
    // 将内容起始Y下移（从140改为180）
    const int lineCount = 5;
    const int lineH = 50;
    const int iconColX = panelL + 40;       // 图标圆心X
    const int textColX = panelL + 130;      // 文字起始X
    int contentTop = panelT + 180;          // 下移 40 像素

    // 辅助函数：绘制带圆圈的图标（若图片为空则绘制图形）
    auto DrawIconWithCircle = [&](int row, IMAGE* img, int imgW, int imgH,
        DWORD circleColor = RGB(45, 65, 90),
        DWORD borderColor = RGB(120, 160, 200),
        bool useTexture = false, int textureColor = 0) {
            int y = contentTop + row * lineH + (lineH - imgH) / 2;
            int cx = iconColX + imgW / 2;
            int cy = y + imgH / 2;
            int r = imgW / 2 + 4;

            setfillcolor(circleColor);
            setlinecolor(borderColor);
            solidcircle(cx, cy, r);

            if (img != NULL)
            {
                putimage(iconColX, y, imgW, imgH, img, 0, 0);
            }
            else
            {
                // 无图片时的后备绘制（此处未使用，因为障碍物和传送门将直接使用图片）
            }
        };

    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(RGB(220, 230, 250));

    switch (page)
    {
    case 0: // 欢迎页
    {
        outtextxy(textColX, contentTop + 0 * lineH, _T("欢迎来到贪吃蛇世界！"));

        outtextxy(textColX, contentTop + 1 * lineH, _T("使用  WASD  或  方向键  控制蛇移动"));
        outtextxy(textColX, contentTop + 2 * lineH, _T("空格键  暂停 / 继续游戏"));

        // 右侧小蛇装饰（此处可保留或省略）
        // 由于我们没有蛇头图片，可以画一个小蛇，或省略
        break;
    }

    case 1: // 规则页
    {
        DrawIconWithCircle(0, &imgAppleRed, 30, 30);
        outtextxy(textColX, contentTop + 0 * lineH, _T("红苹果  +10 分"));

        DrawIconWithCircle(1, &imgAppleGold, 30, 30);
        outtextxy(textColX, contentTop + 1 * lineH, _T("金苹果  +50 分"));

        DrawIconWithCircle(2, &imgBomb, 30, 30);
        outtextxy(textColX, contentTop + 2 * lineH, _T("炸弹    -30 分"));

        // 障碍物使用 wall.png
        DrawIconWithCircle(3, &imgWall, 30, 30);
        outtextxy(textColX, contentTop + 3 * lineH, _T("障碍物  撞击 3 次可摧毁"));

        // 传送门使用 portal.png
        DrawIconWithCircle(4, &imgPortal, 30, 30);
        outtextxy(textColX, contentTop + 4 * lineH, _T("传送门  瞬间穿梭到另一端"));
        break;
    }

    case 2: // 技能与准备
    {
        outtextxy(textColX, contentTop + 0 * lineH, _T("⚡  闪电冲刺 (Q)   -50 分，向前冲 3 格"));
        outtextxy(textColX, contentTop + 1 * lineH, _T("💫  原地复活 (F)   -500 分，蛇身减半"));
        outtextxy(textColX, contentTop + 2 * lineH, _T("🔍  寻路辅助 (E)   显示到达食物的最短路径"));
        outtextxy(textColX, contentTop + 3 * lineH, _T(""));
        outtextxy(textColX, contentTop + 4 * lineH, _T("✨  准备好挑战最高分了吗？"));

        // 右下角小蛇（手绘）
        int sx = panelR - 100, sy = panelB - 80;
        setfillcolor(RGB(0, 220, 0));
        solidcircle(sx, sy, 12);
        solidcircle(sx - 22, sy, 12);
        solidcircle(sx - 44, sy, 12);
        setfillcolor(BLACK);
        solidcircle(sx + 4, sy - 4, 3);
        solidcircle(sx + 4, sy + 4, 3);
        break;
    }
    }

    // ---------- 页码指示点 ----------
    int dotCount = 3;
    int dotR = 12;
    int dotSpacing = 40;
    int dotY = panelB - 50;
    int dotStartX = (panelL + panelR) / 2 - (dotCount - 1) * dotSpacing / 2;
    for (int i = 0; i < dotCount; i++)
    {
        setfillcolor(i == page ? RGB(255, 215, 0) : RGB(100, 120, 160));
        solidcircle(dotStartX + i * dotSpacing, dotY, dotR);
    }

    // ---------- 底部提示 ----------
    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(170, 200, 240));
    const wchar_t* tip = (page < 2) ? L"按任意键查看下一页  →" : L"按任意键开始游戏  🚀";
    int tipX = (panelL + panelR) / 2 - textwidth(tip) / 2;
    int tipY = panelB - 20;
    outtextxy(tipX, tipY, tip);
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
    cleardevice(); // 清空窗口

    if (gameState == GAME_START)
    {
        RenderTutorial(tutorialPage);
        return; // 开始界面独占，不绘制地图和游戏元素
    }

    // 正常游戏状态（运行/暂停/结束）才绘制地图及游戏内容
    RenderMap();
    RenderFood();
    RenderSnake();
    RenderUI();

    if (showHelperPath && !helperPath.empty())
    {
        setfillcolor(RGB(255, 200, 0));
        for (auto& p : helperPath)
        {
            int x = p.x * CELL_SIZE + CELL_SIZE / 2;
            int y = p.y * CELL_SIZE + CELL_SIZE / 2;
            solidcircle(x, y, 6);
        }
    }
}
void CloseView()
{
    closegraph();
}