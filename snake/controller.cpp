#include "controller.h"
#include "model.h"
#include "view.h"
#include <graphics.h>   // 确保 ExMessage 可用

void HandleKeyInput()
{
    ExMessage msg;

    // 非阻塞检测键盘消息（有按键才处理，没有立即返回）
    if (!peekmessage(&msg, EX_KEY, true))  // EX_KEY 过滤键盘消息，true 表示从队列移除
        return;

    // 只处理按下事件，避免按住时连续触发
    if (msg.message != WM_KEYDOWN)
        return;

    // ========== 游戏开始页：任意键翻页 ==========
    if (gameState == GAME_START)
    {
        NextTutorialPage();
        return;
    }

    // ========== ESC 退出（最高优先级） ==========
    if (msg.vkcode == VK_ESCAPE)
    {
        SaveHighScore();
        CloseView();
        exit(0);
        return;
    }

    // ========== 方向键（上下左右） ==========
    switch (msg.vkcode)
    {
    case VK_UP:    nextDir = DIR_UP;    break;
    case VK_DOWN:  nextDir = DIR_DOWN;  break;
    case VK_LEFT:  nextDir = DIR_LEFT;  break;
    case VK_RIGHT: nextDir = DIR_RIGHT; break;
    }

    // ========== 字母 / 功能键（WASD 等） ==========
    // 注意：msg.ch 存放 ASCII 字符（对于字母键）
    char key = msg.ch;

    switch (key)
    {
    case 'W': case 'w': nextDir = DIR_UP;    break;
    case 'S': case 's': nextDir = DIR_DOWN;  break;
    case 'A': case 'a': nextDir = DIR_LEFT;  break;
    case 'D': case 'd': nextDir = DIR_RIGHT; break;

    case ' ':  // 空格键
        if (gameState == GAME_RUNNING) gameState = GAME_PAUSE;
        else if (gameState == GAME_PAUSE) gameState = GAME_RUNNING;
        break;

    case 'R': case 'r': InitGame(); break;
    case 'Q': case 'q': SkillDash(); break;
    case 'F': case 'f': SkillRevive(); break;

    case 'E': case 'e':
        // 原 E 键逻辑（显示寻路辅助路径）
        if (gameState == GAME_RUNNING)
        {
            gameState = GAME_PAUSE;
            helperPath.clear();
            int hx = snake[0].x;
            int hy = snake[0].y;
            if (BFSFindPath(hx, hy, curFood.x, curFood.y, helperPath))
                showHelperPath = true;
            else
                showHelperPath = false;
        }
        else if (gameState == GAME_PAUSE)
        {
            gameState = GAME_RUNNING;
            showHelperPath = false;
        }
        break;

    default:
        break;
    }
}