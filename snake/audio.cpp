#include "audio.h"
#include "constants.h"
#include <windows.h>
#include <thread>

// 精简音频：只使用系统蜂鸣作为步伐节拍，不查找或播放文件
void InitAudio()
{
	// no-op
}

void PlayStepSound()
{
	// 不在暂停状态下才播放步伐声
	if (gameState == GAME_PAUSE) return;
	// 异步发出短促蜂鸣作为步伐节拍，避免阻塞主线程
	std::thread([]() { Beep(800, 40); }).detach();
}

void StopAudio()
{
	// no-op
}

// 吃苹果特殊音效（上升的三音阶）
void PlayAppleSound()
{
	if (gameState == GAME_PAUSE) return;
	std::thread([]() {
		Beep(880, 80);
		Sleep(40);
		Beep(1047, 80);
		Sleep(40);
		Beep(1319, 140);
		}).detach();
}

// 炸弹爆炸音效（低频隆响+短促噪音模拟）
void PlayBombSound()
{
	if (gameState == GAME_PAUSE) return;
	std::thread([]() {
		Beep(200, 200);
		Sleep(50);
		Beep(400, 100);
		Sleep(30);
		Beep(250, 150);
		}).detach();
}

// 撞墙死亡音效（低沉急促的下降三音阶）
void PlayDeathSound()
{
	if (gameState == GAME_PAUSE) return;
	std::thread([]() {
		Beep(800, 120);
		Sleep(30);
		Beep(600, 120);
		Sleep(30);
		Beep(400, 220);
		}).detach();
}

//传送门奇幻音效（重叠的上升/下降短音模拟漩涡感）
void PlayPortalSound()
{
	if (gameState == GAME_PAUSE) return;
	std::thread([]() {
		//叠加两路短音
		Beep(600, 80);
		Sleep(40);
		Beep(900, 60);
		Sleep(30);
		Beep(700, 100);
		Sleep(20);
		Beep(1200, 140);
		}).detach();
}
