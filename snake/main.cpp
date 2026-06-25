#include "constants.h"
#include "model.h"
#include "view.h"
#include "controller.h"
#include "audio.h"
#include <windows.h>
#include <chrono>   // 用于高精度计时

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main()
{
    LoadHighScore();
    InitView();
    InitAudio();

    // 记录上一次蛇移动的时刻
    auto lastMove = std::chrono::steady_clock::now();

    while (true)
    {
        // 1. 优先处理键盘输入（非阻塞，几乎立即响应）
        HandleKeyInput();

        // 2. 检查是否到达移动间隔
        auto now = std::chrono::steady_clock::now();
        if (gameState == GAME_RUNNING)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMove).count();
            if (elapsed >= GetMoveInterval())
            {
                MoveSnake();
                PlayStepSound();          // 步伐音效与移动同步
                lastMove = now;           // 重置计时器
            }
        }

        // 3. 渲染画面（总是刷新，保证界面实时响应）
        RenderAll();

        // 4. 短休眠，让出 CPU 时间片，避免忙等占用过高
        Sleep(1);
    }

    StopAudio();
    CloseView();
    return 0;
}