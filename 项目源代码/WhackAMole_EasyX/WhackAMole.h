#ifndef _WHACK_A_MOLE_H_
#define _WHACK_A_MOLE_H_

#include <graphics.h>

#include <string>
#include <iostream>
#include <codecvt>
#include <mmsystem.h>
#include <functional>

#include "Sound.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	950

#define BACK_COLOR		RGB(251, 251, 251)	// 背景颜色
#define TEXT_COLOR		RGB(225, 15, 105)	// 文本颜色

#define SPEED			100					// 游戏速度，值越小地鼠刷新越快

ExMessage msg = { 0 };

class Game
{
public:
	IMAGE imageEmpty;
	IMAGE imageHammer;
	IMAGE imageHamster;
	bool map[4][4] =
	{
		{ false, false, false, false },
		{ false, false, false, false },
		{ false, false, false, false },
		{ false, false, false, false },
	};											// 地图
	int timer = 0;								// 游戏计时器
	int idx_x = rand() % 4, idx_y = rand() % 4;	// 当前地鼠位置

public:
	Game();
	~Game();
	void UpdateTimer();
	void HandleEvent(ExMessage& msg);
	void DrawMap();
	void DrawHit();
	void DrawScore();

private:
	int cursor_x = 0, cursor_y = 0;				// 鼠标位置
	int player_score = 0;						// 玩家得分

private:
	void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg);
};

namespace Math
{
	struct Rect
	{
		int x = 0, y = 0;
		int w = 0, h = 0;
		Rect() = default;
		~Rect() = default;
		Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
		Rect(const RECT& rect) : x(rect.left), y(rect.top),
			w(rect.right - rect.left), h(rect.bottom - rect.top) {
		}
		RECT Convert() { return { x, y, x + w, y + h }; }
	};

	struct Vec2
	{
		float x = 0;
		float y = 0;
	};

	template<typename T>
	inline T Clamp(T val, T min, T max)
	{
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}

	inline bool PointRectOverlap(const POINT& point, const RECT& rect)
	{
		return point.x >= rect.left && point.y >= rect.top && point.x <= rect.right && point.y <= rect.bottom;
	}

	inline bool RectRectOverlap(const RECT& rect_a, const RECT& rect_b)
	{
		bool is_x_overlap = (max(rect_a.right, rect_b.right) - min(rect_a.left, rect_b.left)) <= (rect_a.right - rect_a.left) + (rect_b.right - rect_b.left);
		bool is_y_overlap = (max(rect_a.bottom, rect_b.bottom) - min(rect_a.top, rect_b.top)) <= (rect_a.bottom - rect_a.top) + (rect_b.bottom - rect_b.top);
		return is_x_overlap && is_y_overlap;
	}
}

class Button
{
public:
	Button(const std::string& text, const RECT& rect = { 10, 10, 85, 40 },
		const COLORREF& color_text = RGB(20, 20, 20), const COLORREF& color_idle = RGB(205, 205, 205),
		const COLORREF& color_hover = RGB(235, 235, 235), const COLORREF& color_down = RGB(185, 185, 185))
		: text(text), rect(rect), color_text(color_text), color_idle(color_idle), color_hover(color_hover), color_down(color_down)
	{
	}

	~Button() = default;

	virtual void OnInput(const ExMessage& msg) final
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			is_hovered = Math::PointRectOverlap(POINT{ msg.x, msg.y }, rect);
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (is_hovered) is_pushed = true;
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			if (is_pushed) on_click();
			is_pushed = false;
			break;
		default:
			break;
		}
	}

	virtual void OnRender()
	{
		setlinecolor(RGB(20, 20, 20));
		if (is_pushed)
			setfillcolor(color_down);
		else if (is_hovered)
			setfillcolor(color_hover);
		else
			setfillcolor(color_idle);

		fillrectangle(rect.left, rect.top, rect.right, rect.bottom);

		setbkmode(TRANSPARENT);
		settextcolor(color_text);
		static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		drawtext(converter.from_bytes(text).c_str(), &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}

	virtual void OnClick(std::function<void()> on_click) final
	{
		this->on_click = on_click;
	}

	virtual void Text(const std::string& text) final
	{
		this->text = text;
	}

private:
	RECT rect;
	std::string text;
	std::function<void()> on_click = []() {};
	bool is_hovered = false, is_pushed = false;
	COLORREF color_text, color_idle, color_hover, color_down;
};

#endif // !_WHACK_A_MOLE_H_
