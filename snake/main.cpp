#include "constants.h"
#include "model.h"
#include "view.h"
#include "controller.h"
#include "audio.h"
#include <windows.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main()
{
    LoadHighScore();
    InitView();
    InitAudio();

    while (true)
    {
        HandleKeyInput();

        if (gameState == GAME_RUNNING)
        {
            MoveSnake();
            PlayStepSound(); // 与移动卡节奏
        }

        RenderAll();
        Sleep(GetMoveInterval());// 移动间隔
    }

    StopAudio();
    CloseView();
    return 0;
}