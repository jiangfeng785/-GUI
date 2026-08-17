#include "audio.h"
#include "constants.h"
#include <windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <mmsystem.h> // 必须包含，用于 PlaySound API

// 链接 Windows 多媒体库
#pragma comment(lib, "winmm.lib")

// ====== 音频任务结构 ======
struct AudioTask
{
    std::function<void()> func; // 实际执行的音频函数
};

// ====== 音频独立线程相关变量 ======
static std::thread audioThread;
static std::queue<AudioTask> taskQueue;
static std::mutex queueMutex;
static std::condition_variable queueCV;
static bool audioThreadRunning = true;

// 音频后台线程主循环
static void AudioWorker()
{
    while (true)
    {
        AudioTask task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // 阻塞等待，直到有任务或者收到停止信号
            queueCV.wait(lock, [] { return !taskQueue.empty() || !audioThreadRunning; });

            // 如果是停止信号且任务队列已空，则退出线程
            if (!audioThreadRunning && taskQueue.empty())
                break;

            // 取出最前面的一个任务
            task = taskQueue.front();
            taskQueue.pop();
        }

        // 在互斥锁外执行播放任务，不阻塞其他入队
        if (task.func)
            task.func();
    }
}

// 初始化音频：启动独立的音频后台线程并 detach
void InitAudio()
{
    audioThreadRunning = true;
    audioThread = std::thread(AudioWorker);
    audioThread.detach(); // 主程序退出后，音频线程继续独立完成最后的播放
}

// 停止音频：通知后台线程优雅退出
void StopAudio()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        audioThreadRunning = false;
    }
    queueCV.notify_one();
}

// 根据要求取消步伐音效，该函数直接返回（无任何操作）
void PlayStepSound()
{
    // 移动音频已取消
    return;
}

// 辅助函数：将音频播放任务推入队列
static void PushPlayTask(const wchar_t* filename)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ [filename]() {
            // 使用 PlaySound 播放 WAV 文件
            // SND_ASYNC: 异步播放
            // SND_NOSTOP: 不打断当前正在播放的其他音效
            PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
        } });
    }
    queueCV.notify_one(); // 唤醒音频线程
}
// 吃金苹果音效
void PlayAppleSoundGolden()
{
    if (gameState == GAME_PAUSE) return;
    PushPlayTask(L"res/gold_apple.wav");
}


// 吃红苹果音效
void PlayAppleSoundRed()
{
    if (gameState == GAME_PAUSE) return;
    PushPlayTask(L"res/apple.wav");
}

// 炸弹爆炸音效
void PlayBombSound()
{
    if (gameState == GAME_PAUSE) return;
    PushPlayTask(L"res/bomb.wav");
}

// 撞墙死亡音效
void PlayDeathSound()
{
    if (gameState == GAME_PAUSE) return;
    PushPlayTask(L"res/die.wav");
}

// 传送门音效
void PlayPortalSound()
{
    if (gameState == GAME_PAUSE) return;
    PushPlayTask(L"res/portal.wav");
}