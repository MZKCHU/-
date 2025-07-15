#include "WhackAMole.h"

IMAGE img_menu;

bool is_started = false;

int main(int argc, char** argv)
{
	// 游戏是否退出
	bool is_quit = false;

	// 定义游戏对象
	Game game;
	loadimage(&img_menu, _T("resources/menu.png"));
	HWND window_handle = initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	SetWindowText(window_handle, _T("EasyX 打地鼠"));

	BeginBatchDraw();

	// 定义按钮对象
	Button btn_start(u8"开 始 游 戏", { 300, 450, 500, 500 });
	Button btn_info(u8"游 戏 介 绍", { 300, 550, 500, 600 });
	Button btn_quit(u8"退 出 游 戏", { 300, 650, 500, 700 });
	btn_start.OnClick([&]()
		{
			is_started = true;
			mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
		});
	btn_info.OnClick([&]()
		{
			MessageBox(GetHWnd(), _T("地图中会有随机刷新的地鼠\n点击鼠标左键敲击地鼠得分"), _T("玩法介绍"), MB_OK);
		});
	btn_quit.OnClick([&]()
		{
			is_quit = true;
		});

	// 如果游戏不退出，则循环执行
	while (!is_quit)
	{
		setbkcolor(BACK_COLOR);		// 设置背景颜色
		cleardevice();				// 清空上一帧绘图内容

		game.UpdateTimer();			// 更新游戏计时器

		while (peekmessage(&msg, EM_MOUSE))
		{
			if (is_started)
				game.HandleEvent(msg);			// 处理事件
			else
			{
				btn_start.OnInput(msg);
				btn_info.OnInput(msg);
				btn_quit.OnInput(msg);
			}
		}

		if (is_started)
		{
			game.DrawMap();				// 绘制地图场景
			game.DrawHit();				// 绘制击中锤子特效
			game.DrawScore();			// 绘制玩家得分
		}
		else
		{
			putimage(0, 0, &img_menu);
			btn_start.OnRender();
			btn_info.OnRender();
			btn_quit.OnRender();
		}

		FlushBatchDraw();			// 执行未完成的绘制任务
		Sleep(5);					// 延时，防止帧率过高
	}

	// 结束批量绘图模式
	EndBatchDraw();

	return 0;
}

Game::Game()
{
	// 加载游戏资源
	loadimage(&imageEmpty, _T("resources/empty.jpg"));
	loadimage(&imageHammer, _T("resources/hammer.png"));
	loadimage(&imageHamster, _T("resources/hamster.jpg"));

	// 初始化地图
	map[idx_x][idx_y] = true;

	// 加载音效
	mciSendString(_T("open resources/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open resources/hit.mp3 alias mymusic"), NULL, 0, NULL);
}

Game::~Game()
{
}

void Game::UpdateTimer()
{
	// 更新时间
	timer = ++timer % SPEED;

	// 到达刷新时机，更新地鼠位置
	if (timer == 0)
	{
		map[idx_x][idx_y] = false;
		idx_x = rand() % 4;
		idx_y = rand() % 4;
		map[idx_x][idx_y] = true;
	}
}

void Game::HandleEvent(ExMessage& msg)
{
	// 处理鼠标消息
	if (msg.message == WM_MOUSEMOVE)
	{
		cursor_x = msg.x;
		cursor_y = msg.y;
	}
	else if (msg.message == WM_LBUTTONDOWN)
	{
		// 如果鼠标落在地鼠区域内
		if (map[idx_x][idx_y] &&
			(cursor_x >= idx_x * 184 + 32 && cursor_x <= (idx_x + 1) * 184 + 32
				&& cursor_y >= idx_y * 208 + 75 && cursor_y <= (idx_y + 1) * 208 + 75))
		{
			mciSendString(_T("play mymusic from 0"), NULL, 0, NULL);
			player_score += 10;			 // 增加分数
			map[idx_x][idx_y] = false;	 // 地鼠消失
		}
	}
}

void Game::DrawMap()
{
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			if (map[i][j])
				putimage(i * 184 + 32, j * 208 + 75, &imageHamster);
			else
				putimage(i * 184 + 32, j * 208 + 75, &imageEmpty);
		}
	}
}

void Game::DrawHit()
{
	// 绘制锤子
	transparentimage(nullptr, cursor_x - 62, cursor_y - 75, &imageHammer);
}

void Game::DrawScore()
{
	TCHAR s[512];
	_stprintf_s(s, _T("玩家得分：%d"), player_score);
	settextstyle(25, 0, _T("黑体"));
	settextcolor(TEXT_COLOR);
	outtextxy(25, 25, s);
}

// 半透明贴图函数
// 参数：
//   dstimg：目标图像（nullptr 表示默认窗口）
//   x, y：绘制位置
//   srcimg：源图像指针
void Game::transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{
	DWORD* dst = GetImageBuffer(dstimg);
	DWORD* src = GetImageBuffer(srcimg);
	int src_width = srcimg->getwidth();
	int src_height = srcimg->getheight();
	int dst_width = (dstimg == nullptr ? getwidth() : dstimg->getwidth());
	int dst_height = (dstimg == nullptr ? getheight() : dstimg->getheight());

	int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;
	int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;
	if (x < 0) { src += -x; iwidth -= -x; x = 0; }
	if (y < 0) { src += src_width * -y; iheight -= -y; y = 0; }

	dst += dst_width * y + x;

	for (int iy = 0; iy < iheight; iy++)
	{
		for (int ix = 0; ix < iwidth; ix++)
		{
			int sa = ((src[ix] & 0xff000000) >> 24);
			int sr = ((src[ix] & 0xff0000) >> 16);
			int sg = ((src[ix] & 0xff00) >> 8);
			int sb = src[ix] & 0xff;
			int dr = ((dst[ix] & 0xff0000) >> 16);
			int dg = ((dst[ix] & 0xff00) >> 8);
			int db = dst[ix] & 0xff;

			dst[ix] = ((sr + dr * (255 - sa) / 255) << 16)
				| ((sg + dg * (255 - sa) / 255) << 8)
				| (sb + db * (255 - sa) / 255);
		}
		dst += dst_width;
		src += src_width;
	}
}
