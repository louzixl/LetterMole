#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>
#include <ctime>
#include <thread>
#include <mutex>

using std::cout;
using std::endl;

//常量定义
const wchar_t *title = L"Letter Mole";
const wchar_t *name = L"字母打地鼠";
const wchar_t *tips1 = L"这是一个打字母游戏，打到的字母越多，积分越高；";
const wchar_t *tips2 = L"但是字母下降的速度越快，游戏越难！";
const wchar_t *enter = L"按任意键进入游戏~~~";
const wchar_t *score = L"当前分数：";
const wchar_t *miss = L"当前Miss：";
const wchar_t *overgame = L"Game Over!";

//全局变量
HANDLE hout;
int g_nScore, g_nMiss;
int g_nScreen_width, g_nScreen_height;
int g_nSpeed;
wchar_t g_ch_random[2];
char g_ch_input;
bool g_bFlag;
bool g_bOver;

//函数声明
void Print_start_interface();
void Print_game_info();
void Game();
int Random(const int max);
void Letter_down();
void Over_game();
void Process_input();

int main()
{
	Print_start_interface();
	system("cls");                      //清除屏幕
	Game();

	return 0;
}

void Print_start_interface()
{
	size_t i;
	DWORD len;
	CONSOLE_SCREEN_BUFFER_INFO console_info;
	CONSOLE_CURSOR_INFO cursor_info;
	COORD name_pos, tips_pos, enter_pos;
	WORD name_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_INTENSITY;
	WORD tips_color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	WORD enter_color = FOREGROUND_RED | FOREGROUND_INTENSITY;

	SetConsoleTitle(title);                                      //设置窗口标题
	hout = GetStdHandle(STD_OUTPUT_HANDLE);                      //获取输出句柄
	GetConsoleCursorInfo(hout, &cursor_info);                    //隐藏光标
	cursor_info.bVisible = FALSE;
	SetConsoleCursorInfo(hout, &cursor_info);

	GetConsoleScreenBufferInfo(hout, &console_info);                           //获取窗口信息
	g_nScreen_width = console_info.srWindow.Right + 1;
	g_nScreen_height = console_info.srWindow.Bottom + 1;
	name_pos.X = (console_info.srWindow.Right - wcslen(name)) / 2;
	name_pos.Y = console_info.srWindow.Bottom / 4;
	WriteConsoleOutputCharacter(hout, name, wcslen(name), name_pos, &len);     //输出程序名称
	for (i = 0; i < wcslen(name) * 2; i++)
	{
		COORD temp;
		temp.X = name_pos.X + i;
		temp.Y = name_pos.Y;
		WriteConsoleOutputAttribute(hout, &name_color, 1, temp, &len);
	}
	tips_pos.X = 20;
	tips_pos.Y = console_info.srWindow.Bottom / 2;
	WriteConsoleOutputCharacter(hout, tips1, wcslen(tips1), tips_pos, &len);   //输出程序提示
	for (i = 0; i < wcslen(tips1) * 2; i++)
	{
		COORD temp;
		temp.X = tips_pos.X + i;
		temp.Y = tips_pos.Y;
		WriteConsoleOutputAttribute(hout, &tips_color, 1, temp, &len);
	}
	tips_pos.Y += 1;
	WriteConsoleOutputCharacter(hout, tips2, wcslen(tips2), tips_pos, &len);   //输出进入游戏提示
	for (i = 0; i < wcslen(tips2) * 2; i++)
	{
		COORD temp;
		temp.X = tips_pos.X + i;
		temp.Y = tips_pos.Y;
		WriteConsoleOutputAttribute(hout, &tips_color, 1, temp, &len);
	}
	enter_pos.X = (console_info.srWindow.Right - wcslen(enter)) / 2;
	enter_pos.Y = console_info.srWindow.Bottom / 4 * 3;
	while (!_kbhit())
	{
		WriteConsoleOutputCharacter(hout, enter, wcslen(enter), enter_pos, &len);
		for (i = 0; i < wcslen(tips2) * 2; i++)
		{
			COORD temp;
			temp.X = enter_pos.X + i;
			temp.Y = enter_pos.Y;
			WriteConsoleOutputAttribute(hout, &enter_color, 1, temp, &len);
		}
		if (enter_color == FOREGROUND_RED)
			enter_color = 0;
		else if (enter_color == 0)
			enter_color = FOREGROUND_RED | FOREGROUND_INTENSITY;
		else if (enter_color == (FOREGROUND_RED | FOREGROUND_INTENSITY))
			enter_color = FOREGROUND_RED;
		Sleep(150);
	}
	int ch = _getch();
}

void Print_game_info()
{
	DWORD len;
	COORD score_pos = { 1, 1 }, miss_pos = { 1, 2 };
	WriteConsoleOutputCharacter(hout, score, wcslen(score), score_pos, &len);
	WriteConsoleOutputCharacter(hout, miss, wcslen(miss), miss_pos, &len);
}

void Game()
{
	DWORD len;
	g_nSpeed = 300;
	g_nMiss = 0;
	g_nScore = 0;
	g_bFlag = false;
	g_bOver = false;
	std::thread *alpha_down;
	std::thread *process_input;

	COORD score_pos = { wcslen(score) * 2 + 1, 1 };
	wchar_t score_num[5] = {};
	wsprintf(score_num, L"%d", g_nScore);
	WriteConsoleOutputCharacter(hout, score_num, wcslen(score_num), score_pos, &len);
	COORD miss_pos = { wcslen(score) * 2 + 1, 2 };
	wchar_t miss_num[5] = {};
	wsprintf(miss_num, L"%d", g_nMiss);
	WriteConsoleOutputCharacter(hout, miss_num, wcslen(miss_num), miss_pos, &len);

	alpha_down = new std::thread(Letter_down);
	process_input = new std::thread(Process_input);
	alpha_down->join();
	process_input->join();

	Over_game();
}

int Random(const int max)
{
	std::default_random_engine engine(time(0));
	return std::uniform_int_distribution<int>(0, max)(engine);
}

void Letter_down()
{
	int i;
	DWORD len;
	COORD ch_pos;
	std::mutex m1, m2;

	while (1)
	{
		ch_pos.X = Random(g_nScreen_width - 1);
		ch_pos.Y = 0;
		m1.lock();
		g_ch_random[0] = 'A' + Random(25);
		g_ch_random[1] = '\0';
		m1.unlock();
		for (i = 0; i < g_nScreen_height; i++)
		{
			Print_game_info();
			WriteConsoleOutputCharacter(hout, g_ch_random, 1, ch_pos, &len);
			Sleep(g_nSpeed);
			WriteConsoleOutputCharacter(hout, L" ", 1, ch_pos, &len);
			ch_pos.Y++;
			if (g_bOver)
				break;
			if (g_bFlag)
			{
				m2.lock();
				g_bFlag = false;
				m2.unlock();
				break;
			}
		}
		if (i == g_nScreen_height)
		{
			g_nMiss++;
			COORD miss_pos = {wcslen(score) * 2 + 1, 2};
			wchar_t miss_num[5] = {};
			wsprintf(miss_num, L"%d", g_nMiss);
			WriteConsoleOutputCharacter(hout, miss_num, wcslen(miss_num), miss_pos, &len);
		}
		if (g_bOver)
			break;
	}
}

void Over_game()
{
	system("cls");
	COORD over_pos;
	DWORD len;
	WORD over_color;

	over_pos.X = (g_nScreen_width - wcslen(overgame)) / 2;
	over_pos.Y = g_nScreen_height / 2;
	over_color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	WriteConsoleOutputCharacter(hout, overgame, wcslen(overgame), over_pos, &len);
	for (size_t i = 0; i < wcslen(overgame); i++)
	{
		WriteConsoleOutputAttribute(hout, &over_color, 1, over_pos, &len);
		over_pos.X++;
	}
}

void Process_input()
{
	std::mutex m1, m2, m3;
	DWORD len;

	while (1)
	{
		g_ch_input = _getch();
		if (isalpha(g_ch_input) && toupper(g_ch_input) == g_ch_random[0])
		{
			m1.lock();
			g_bFlag = true;
			m1.unlock();
			g_nScore++;
			m3.lock();
			if (g_nScore % 10 == 0 && g_nSpeed > 50)
				g_nSpeed -= 50;
			m3.unlock();
			COORD score_pos = { wcslen(score)*2 + 1, 1 };
			wchar_t score_num[5] = {};
			wsprintf(score_num, L"%d", g_nScore);
			WriteConsoleOutputCharacter(hout, score_num, wcslen(score_num), score_pos, &len);
		}
		else if (g_ch_input == 27)
		{
			break;
		}
	}
	m2.lock();
	g_bOver = true;
	m2.unlock();
}