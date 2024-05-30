#pragma once
#include "Lib.h"
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;
using namespace std;

extern int Foods_Amount;

// Declaration
int Random(int min, int max);
int Partition(vector<pathfinding>& arr, int left, int right);
void Quick_Sort(vector<pathfinding>& arr, int left, int right);

void Draw_Line(const HWND hWnd, const POINT p1, const POINT p2, const COLORREF color, const int thickness = 1);
void Draw_Rect(const HWND hWnd, const RECT Rect, const COLORREF colorBorder, const COLORREF colorFill);
void Draw_Fill_Rect(const HWND hWnd, const RECT Rect, const COLORREF colorBorder, const COLORREF colorFill);
void Flood_Fill(const HWND hWnd, const POINT point, const COLORREF color, const DWORD model = FLOODFILLSURFACE);
bool Point_In_Rect(const RECT& scope, const POINT& Pt);

void Load_Button_Images(const HWND hwnd);
void Unload_Button_Images();
void Draw_Image(HWND hWnd, int x, int y, const wchar_t* imagePath);
void Play_Media(LPCTSTR pszSound, HMODULE hmod = NULL, DWORD fdwSound = SND_ASYNC | SND_ALIAS);
COLORREF Get_Pixel_Color(HWND hWnd, POINT point);
place LEFT(place location);
place RIGHT(place location);
place DOWN(place location);
place UP(place location);
void RetroSnake_Initialize(HWND hwnd, bool Using_Grid, bool Spawning_Foods, int FA, int Len, int Lvl);
void RetroSnake_Destruction();
void Foods_Create(int value, int max);
void Foods_Create(int* queue, int max);
void Foods_Spawn(int Amount = Foods_Amount);
void Update_Snake_Head(HWND hWnd);
void Update_Snake_Rail(HWND hWnd);
void Check_Snake_To_Update(HWND hWnd, place Now, place Next, place Before);
void Draw_Snake_According_To_Vector(HWND hWnd);
void Random_Draw_Snake();
void Random_Draw_Obstacle();
void Refresh_ScoreBoard(int Type);
void Inherit(unordered_map<place, int> temp);
void Circulate_Draw_Image(HWND hWnd, int x, int y, int max_x, int max_y, const wchar_t* imagePath);
int Adjacent_Unit_Amount(place location, int type);
int Surrounding_Unit_Amount(place location, int type);
place Find_Common_Side(place a, place b);
void Lengthen(place source, place target, COLORREF GridColor, COLORREF InsideColor);
void Shorten(int begin, int model = 2);
void Moving_Upward(bool lengthen, int type);
void Moving_Downward(bool lengthen, int type);
void Moving_Left(bool lengthen, int type);
void Moving_Right(bool lengthen, int type);
void Clear_Block_Occupied(char Forward, place loc);
void Water_Clear(place loc);
void Water_Spreading();
void Blocks_Drying();
int KeyBoard_Input(int userKey);
void Auto_Moving();
void GameOver();
void Hash_Info();
void Eating(int type);
void Level_Up();
int Detect(place location, int type, int model);
void Clear_Windows();
// GDI+ Initialization
void Gdiplus_Startup_Wrapper();
void Gdiplus_Shutdown_Wrapper();
VOID CALLBACK Liquid_Flowing_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK Auto_Moving(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK Liquid_Refresh_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void StartNewGameWindow();
void OpenSetting();
void Enter_Name();
void Save_Score();
void Ranking_Calculate();
void Enchantment();
void Solidification(place location);
void Event();