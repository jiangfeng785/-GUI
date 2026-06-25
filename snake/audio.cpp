#include "audio.h"
#include "constants.h"
#include <windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

// ====== 音频任务结构 ======
struct AudioTask
{
    bool isStep;                // 标记是否为步伐任务
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

        // 在互斥锁外执行蜂鸣任务，不阻塞其他任务入队
        if (task.func)
            task.func();
    }
}

// 初始化音频：启动独立的音频后台线程并 detach（与主程序脱离）
void InitAudio()
{
    audioThreadRunning = true;
    audioThread = std::thread(AudioWorker);
    audioThread.detach(); // 主程序退出后，音频线程继续独立运行直到完成当前任务
}

// 停止音频：通知后台线程优雅退出（处理完剩余任务）
void StopAudio()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        audioThreadRunning = false;
    }
    queueCV.notify_one();
    // 线程已 detach，无需 join，它会自动退出
}

// 【核心修复】步伐任务：清空队列中过期的脚步任务，确保与移动实时同步
void PlayStepSound()
{
    if (gameState == GAME_PAUSE) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        // 创建一个临时新队列，只保留非步伐任务（如苹果、炸弹），过滤掉所有旧步伐任务
        std::queue<AudioTask> newQueue;
        while (!taskQueue.empty())
        {
            if (!taskQueue.front().isStep)
            {
                newQueue.push(taskQueue.front());
            }
            taskQueue.pop();
        }
        taskQueue.swap(newQueue);

        // 放入最新的步伐音频任务
        taskQueue.push({ true, []() { Beep(800, 40); } });
    }
    queueCV.notify_one(); // 唤醒音频线程
}

// 吃苹果特殊音效（上升的三音阶）
void PlayAppleSound()
{
    if (gameState == GAME_PAUSE) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ false, []() {
            Beep(880, 80);
            Sleep(40);
            Beep(1047, 80);
            Sleep(40);
            Beep(1319, 140);
        } });
    }
    queueCV.notify_one();
}

// 炸弹爆炸音效（低频隆响+短促噪音模拟）
void PlayBombSound()
{
    if (gameState == GAME_PAUSE) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ false, []() {
            Beep(200, 200);
            Sleep(50);
            Beep(400, 100);
            Sleep(30);
            Beep(250, 150);
        } });
    }
    queueCV.notify_one();
}

// 撞墙死亡音效（低沉急促的下降三音阶）
void PlayDeathSound()
{
    if (gameState == GAME_PAUSE) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ false, []() {
            Beep(800, 120);
            Sleep(30);
            Beep(600, 120);
            Sleep(30);
            Beep(400, 220);
        } });
    }
    queueCV.notify_one();
}

// 传送门奇幻音效（重叠的上升/下降短音模拟漩涡感）
void PlayPortalSound()
{
    if (gameState == GAME_PAUSE) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ false, []() {
            Beep(600, 80);
            Sleep(40);
            Beep(900, 60);
            Sleep(30);
            Beep(700, 100);
            Sleep(20);
            Beep(1200, 140);
        } });
    }
    queueCV.notify_one();
}