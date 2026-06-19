#pragma once
#include "constants.h"

// 初始化图形窗口
void InitView();

// 整体绘制
void RenderAll();

// 绘制地图、墙、障碍物、传送门
void RenderMap();

// 绘制蛇（多皮肤：普通/金龙/机甲）
void RenderSnake();

// 绘制食物、炸弹
void RenderFood();

// 绘制UI面板、分数、状态、按钮
void RenderUI();

// 绘制BFS辅助路径（E键）
void RenderHelperPath(const std::vector<POINT>& path);

// 关闭窗口
void CloseView();

// 新手指引渲染（在开始页面）
void RenderTutorial(int page);

// 前进或翻页新手指引
void NextTutorialPage();