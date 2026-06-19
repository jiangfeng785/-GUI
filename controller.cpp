#include "controller.h"
#include "model.h"
#include "view.h"
#include <conio.h>

void HandleKeyInput()
{
    if (!_kbhit()) return;
    char key = _getch();

    if (gameState == GAME_START)
    {
        // 在开始界面按任意键翻页新手引导或开始
        NextTutorialPage();
        return;
    }

    switch (key)
    {
    case 'W': case 'w': nextDir = DIR_UP; break;
    case 'S': case 's': nextDir = DIR_DOWN; break;
    case 'A': case 'a': nextDir = DIR_LEFT; break;
    case 'D': case 'd': nextDir = DIR_RIGHT; break;

    case ' ': // 空格暂停/继续
        if (gameState == GAME_RUNNING) gameState = GAME_PAUSE;
        else if (gameState == GAME_PAUSE) gameState = GAME_RUNNING;
        break;

    case 'R': case 'r': // 重开
        InitGame();
        break;

    case 'Q': case 'q': // 闪电冲刺
        SkillDash();
        break;

    case 'F': case 'f': // 原地复活
        SkillRevive();
        break;

    case 'E': case 'e':
    {
        std::vector<POINT> path;
        int hx = snake[0].x;
        int hy = snake[0].y;

        // 寻路到当前存在的单个苹果
        BFSFindPath(hx, hy, curFood.x, curFood.y, path);
        RenderHelperPath(path);
        break;
    }

    case 27: // ESC退出
        SaveHighScore();
        CloseView();
        exit(0);
        break;
    }
}