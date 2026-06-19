#include "audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <string>

#pragma comment(lib, "winmm.lib")

static std::string bgmPath;
static bool bgmPlaying = false;

void InitAudio()
{
    // 尝试在可执行目录查找 bgm.wav
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);
    std::string s(buf);
    size_t pos = s.find_last_of("\\/");
    if (pos != std::string::npos) s = s.substr(0, pos);

    std::string candidates[] = { s + "\\bgm.wav", s + "\\res\\bgm.wav" };
    for (auto &c : candidates)
    {
        if (GetFileAttributesA(c.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            bgmPath = c;
            break;
        }
    }

    if (!bgmPath.empty())
    {
        // 使用 PlaySound 循环播放背景音乐（非阻塞）
        PlaySoundA(bgmPath.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
        bgmPlaying = true;
    }
}

void PlayStepSound()
{
    // 简单发出短促声音作为步伐节拍（使用系统默认蜂鸣）
    Beep(800,40);
}

void StopAudio()
{
    if (bgmPlaying)
    {
        PlaySoundA(NULL, NULL,0);
        bgmPlaying = false;
    }
}
