#pragma comment (lib,"Gdiplus.lib")
#include "ForgottenRealms.h"
#include "Config.cpp"
using namespace Gdiplus;
using namespace std;
#define GAME

bool isMouseOverButton[4] = { false, false, false, false };
bool isMouseOverSettingsButton[4] = { false, false, false, false };
int Reward[8] = { 1, 2, 3, 0, 5, 0, 0, 8 };

place head;
HINSTANCE g_hInstance;
WNDCLASS wc;
WNDCLASS ng;
WNDCLASS settings;
WNDCLASS nh;
HWND GameHwnd;
HWND hWnd;
HWND Button[4];
HWND Settings_Button[4];
HWND ScoreBoardHwnd;
HWND SettingsHwnd;
HWND NameHwnd;
RECT ButtonRect[4];
RECT Settings_Button_Rect[4];
HBRUSH hBrush;
HFONT hFont;
UINT_PTR AutoMovingTimer;
UINT_PTR LiquidRefresh;
UINT_PTR LiquidFlowing;
UINT_PTR AutoDmg;
UINT_PTR MusicPlayer;
ULONG_PTR g_gdiplusToken;

COLORREF BackGround_Color;
COLORREF Grid_Color;
COLORREF Snake_Head_Color;
COLORREF Snake_Color;
COLORREF Food_Color;
COLORREF Frame_Color;
COLORREF TextColor;

// Direct2D 启动！
ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
IDWriteFactory* pDWriteFactory = nullptr;
IDWriteTextFormat* pTextFormat = nullptr;
ID2D1SolidColorBrush* pBlackBrush = nullptr;

vector<vector<int>> RetroSnake_Hashes;
vector<vector<int>> Hash_Temp;
vector<place> SnakePlace;
vector<place> ObstaclePlace;
std::queue<place> WaterBlockToHandle;
std::set<place> WaterBlockVisited;
unordered_map<place, int> DryingBlock;
unordered_map<place, int> ObstacleAbove;
vector<unordered_map<place, int>> Liq(8);

const wchar_t* aw[8] = ORES;
const BUTTON ButtonList[4] = BUTTON_LIST;
const wchar_t* LavaList[19] = LAVA_LIST;
const wchar_t* WaterList[32] = WATER_LIST;
const wchar_t* KidsWaterList[16][7] = KIDS_WATER_LIST;
const wchar_t* KidsLavaList[8][3] = KIDS_LAVA_LIST;
const wchar_t* Setting_Buttons_List[4] = SETTING_LIST;
const BUTTON MusicList[8] = MUSIC_LIST;
const BUTTON SpeedList[3] = SPEED_LIST;
const BUTTON ModeList[2] = MODE_LIST;
int Music = 0;
int Music_Order = 0;
int Speed = 0;
int Mode = 0;
int Health;
int Oxygen = 10;
int Fortune;
int Fortune_Times;
const Media MediaList[6] = MEDIA_LIST;
WCHAR Name[256];

int Auto_Moving_Cooldown[3] = { AUTO_MOVING_COOLDOWN , AUTO_MOVING_COOLDOWN * 1.5, AUTO_MOVING_COOLDOWN / 2 };


Image* buttonNormalImage = nullptr;
Image* buttonActiveImage = nullptr;
char Forward = '\0';
bool Use_Grid = false;
bool Spawn_Foods = true;
bool Running;
bool In_The_Water;
int Owe_Length = 0;
int Max_Owe_Length = 0;
int Existing_Ores = 0;
int Max_Score_In_This_Layer = 0;
int Curr_Score_In_This_Layer = 0;
int Foods_Amount;
int Current_Foods_Amount;
int Length;
int Depth = 0;
vector<int> Awards(8, 0);
int Level = 0;
int BreakTime;
int Total_Steps = 0;
int Max_Steps = 0;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
double Theoretical_Score;
double Actrual_Score;
double Multiply;
bool BannedAutoMoving;
bool Settings_Init = false;

place Translate(int x, int y) {
    place temp;
    temp.x = x;
    temp.y = y;
    return temp;
}

int Partition(vector<pathfinding>& arr, int left, int right) {
    int pivotIndex = right;
    int pivotValue = arr[pivotIndex].value;
    int i = left - 1;

    for (int j = left; j < right; ++j) {
        if (arr[j].value <= pivotValue) {
            ++i;
            swap(arr[i], arr[j]);
        }
    }

    swap(arr[i + 1], arr[pivotIndex]);

    return i + 1;
}
void Quick_Sort(vector<pathfinding>& arr, int left, int right) {
    if (left < right) {
        int pivotIndex = Partition(arr, left, right);
        Quick_Sort(arr, left, pivotIndex - 1);
        Quick_Sort(arr, pivotIndex + 1, right);
    }
}

int Random(int min, int max) {
    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

void Draw_Line(const HWND hWnd, const POINT p1, const POINT p2, const COLORREF color, const int thickness) {
    HDC hdc = GetDC(hWnd);
    int x1 = p1.x;
    int x2 = p2.x;
    int y1 = p1.y;
    int y2 = p2.y;
    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    ReleaseDC(hWnd, hdc);
}

void Draw_Rect(const HWND hWnd, const RECT Rect, const COLORREF colorBorder, const COLORREF colorFill) {
    HDC hdc = GetDC(hWnd);
    HPEN hPen = CreatePen(PS_SOLID, 1, colorBorder);
    if (hPen == NULL) {
        ReleaseDC(hWnd, hdc);
        return;
    }
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));

    HBRUSH hBrush = CreateSolidBrush(colorFill);
    if (hBrush == NULL) {
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        ReleaseDC(hWnd, hdc);
        return;
    }
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));

    // ?????????????????
    Rectangle(hdc, Rect.left, Rect.top, Rect.right, Rect.bottom); // Rectangle????????????

    // ???????????????
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    ReleaseDC(hWnd, hdc); // ????豸??????
}

void Draw_Fill_Rect(const HWND hWnd, const RECT Rect, const COLORREF colorBorder, const COLORREF colorFill) {
    HDC hdc = GetDC(hWnd);
    HPEN hPen = CreatePen(PS_SOLID, 1, colorBorder);
    if (hPen == NULL) {
        ReleaseDC(hWnd, hdc);
        return;
    }
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));

    HBRUSH hBrush = CreateSolidBrush(colorFill);
    if (hBrush == NULL) {
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        ReleaseDC(hWnd, hdc);
        return;
    }
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));

    Rectangle(hdc, Rect.left, Rect.top, Rect.right, Rect.bottom); 

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    ReleaseDC(hWnd, hdc); 
}

void Draw_Text(HWND hWnd, bool Using_Dirt_Backpack, int x, int y, Gdiplus::Color color, const wchar_t* Font, int size, const wchar_t *str) {
    Gdiplus::Graphics graphics(hWnd); // 假设hWnd为窗口句柄

    // 加载并绘制PNG背景
    if (Using_Dirt_Backpack) {
        Gdiplus::Image image(L"Blocks/dirt.png");
        graphics.DrawImage(&image, 0, 0, image.GetWidth(), image.GetHeight());
        for (int i = 0; i < MAP_WIDTH / IMAGE_SIZE - MAP_WIDTH / IMAGE_SIZE + 8; i++) {
            for (int j = 0; j < MAP_HEIGHT / IMAGE_SIZE; j++) {
                graphics.DrawImage(&image, i * IMAGE_SIZE, j * IMAGE_SIZE, image.GetWidth(), image.GetHeight());
            }
        }
    }

    // 设置文本和笔刷颜色
    Gdiplus::SolidBrush brush(color); // 白色

    Gdiplus::Font font(Font, size);

    // 绘制文本
    graphics.DrawString(
        str, -1,
        &font, // 假设已创建Gdiplus::Font对象
        Gdiplus::PointF(x, y),
        &brush);
}

void Flood_Fill(const HWND hWnd, const POINT point, const COLORREF color, const DWORD model) {
    // model: FLOODFILLSURFACE ?? FLOODFILLBORDER
    // FLOODFILLSURFACE?? ???????????????????硣
    // FLOODFILLBORDER??  ?????????????????????????????????????????硣
    HDC hdc = GetDC(hWnd);
    ExtFloodFill(hdc, point.x, point.y, color, model);
    ReleaseDC(hWnd, hdc);
}

bool Point_In_Rect(const RECT& scope, const POINT& Pt) {
    if (Pt.x >= scope.left and Pt.x <= scope.right) {
        if (Pt.y >= scope.top and Pt.y <= scope.bottom) {
            return true;
        }
    }
    return false;
}

void Load_Button_Images(HWND hwnd)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    buttonNormalImage = new Image(L"button.png");
    buttonActiveImage = new Image(L"button_active.png");

    GdiplusShutdown(gdiplusToken);
}

void Unload_Button_Images()
{
    delete buttonNormalImage;
    delete buttonActiveImage;
}

void Draw_Image(HWND hWnd, int x, int y, const wchar_t* imagePath) {
    HDC hdc = GetDC(hWnd);

    Gdiplus::Graphics graphics(hdc);
    Gdiplus::Image image(imagePath);
    Gdiplus::RectF rect(x, y, image.GetWidth(), image.GetHeight());
    graphics.DrawImage(&image, rect);

    ReleaseDC(hWnd, hdc);
}

void Draw_Image_Twice(HWND hWnd, int x, int y, const wchar_t* first_imagePath, const wchar_t* second_imagePath) {
    HDC hdc = GetDC(hWnd);

    Gdiplus::Graphics graphics(hdc);
    Gdiplus::Image fimage(first_imagePath);
    Gdiplus::Image simage(second_imagePath);
    Gdiplus::RectF frect(x, y, fimage.GetWidth(), fimage.GetHeight());
    Gdiplus::RectF srect(x, y, simage.GetWidth(), simage.GetHeight());
    graphics.DrawImage(&fimage, frect);
    graphics.DrawImage(&simage, srect);
    ReleaseDC(hWnd, hdc);
}

void Circulate_Draw_Image(HWND hWnd, int x, int y, int max_x, int max_y, const wchar_t* imagePath) {
    HDC hdc = GetDC(hWnd);
    Gdiplus::Graphics graphics(hdc);

    Gdiplus::Image image(imagePath);
    for (int i = x; i < max_x; i++) {
        for (int j = y; j < max_y; j++) {
            graphics.DrawImage(&image, i * IMAGE_SIZE, j * IMAGE_SIZE, image.GetWidth(), image.GetHeight());
        }
    }
}

void Play_Media(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    PlaySoundW(pszSound, hmod, fdwSound);
    /*
    fdwSound: 
        SND_SYNC: 等待声音播放完毕后才返回。
        SND_ASYNC: 在后台播放声音，不会阻塞当前线程。
        SND_ALIAS: 指定第一个参数为系统定义的声音别名。
        SND_FILENAME: 指定第一个参数为文件名。
    */
}

void Stop_Media() {
    PlaySoundW(NULL, NULL, 0);
}

COLORREF Get_Pixel_Color(HWND hWnd, POINT point) {
    HDC hdc = GetDC(hWnd);
    return GetPixel(hdc, point.x, point.y);
}

place LEFT(place location) {
    return Translate(location.x - 1, location.y);
}
place RIGHT(place location) {
    return Translate(location.x + 1, location.y);
}
place DOWN(place location) {
    return Translate(location.x, location.y + 1);
}
place UP(place location) {
    return Translate(location.x, location.y - 1);
}

void RetroSnake_Initialize(HWND hwnd, bool Using_Grid, bool Spawning_Foods, int FA, int Len, int Lvl) {
    Use_Grid = Using_Grid;
    Spawn_Foods = Spawning_Foods;
    Foods_Amount = FA;
    Length = Len;
    Level = Lvl;
    Fortune = 0;
    Fortune_Times = 0;
    Theoretical_Score = 0;
    Max_Steps = RELATIVE_WIDTH * RELATIVE_HEIGHT;
    Multiply = 1;
    BackGround_Color = WHITE;
    Grid_Color = RGB(200, 200, 200);
    Snake_Head_Color = GREEN;
    Snake_Color = RGB(0, 245, 255);
    Food_Color = RED;
    Frame_Color = BLACK;
    TextColor = BLACK;
    BreakTime = 400;
    GameHwnd = hwnd;
    Forward = '\0';
    Health = 20;
    Oxygen = 10;
    RetroSnake_Hashes.resize(RELATIVE_WIDTH + 1, vector<int>(RELATIVE_HEIGHT + 1, 0));
    Running = true;

    int Music = 0;
    int Music_Order = 0;
    int Speed = 0;
    int Mode = 0;

    Owe_Length = 0;
    Max_Owe_Length = 0;
    Existing_Ores = 0;
    Max_Score_In_This_Layer = 0;
    Curr_Score_In_This_Layer = 0;
    Depth = 100;
    Level = 0;
    Total_Steps = 0;
    Max_Steps = Depth * RELATIVE_WIDTH * RELATIVE_HEIGHT;
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    Circulate_Draw_Image(hwnd, 0, 0, MAP_WIDTH / IMAGE_SIZE - 8, MAP_HEIGHT / IMAGE_SIZE, L"Blocks/stone.png");

    Random_Draw_Snake();
    //int temp = 1;
    int temp = Random(1, 4);
    for (int i = 0; i < temp; i++) {
        Random_Draw_Obstacle();
    }

    if (Spawn_Foods) {
        Foods_Spawn();
    }

    OutputDebugStringA("[System] Game initialization completed.\n");
};

void RetroSnake_Destruction() {
    Running = false;
    Save_Score();
    Ranking_Calculate();
    SnakePlace.clear();
    RetroSnake_Hashes.clear();
    ObstaclePlace.clear();
    ObstacleAbove.clear();
    DryingBlock.clear();
    Hash_Temp.clear();
    WaterBlockVisited.clear();;

    KillTimer(NULL, AutoMovingTimer);
    KillTimer(NULL, LiquidRefresh);
    KillTimer(NULL, LiquidFlowing);
    KillTimer(NULL, AutoDmg);

    while(!WaterBlockToHandle.empty())
        WaterBlockToHandle.pop();

    for (auto it = Liq.begin(); it != Liq.end(); it++)
        (*it).clear();
}

int Is_Ores(place loc) {
    if (WITHIN_CLOSED_INTERVAL(RetroSnake_Hashes[loc.x][loc.y], 10, 20)) {
        return RetroSnake_Hashes[loc.x][loc.y];
    }
    return 0;
}
int Is_Ores(int Hash_Value) {
    if (WITHIN_CLOSED_INTERVAL(Hash_Value, 10, 20)) {
        return Hash_Value;
    }
    return 0;
}
int Is_Liquid(place loc) {
    if (RetroSnake_Hashes[loc.x][loc.y] >= 10000) {
        return RetroSnake_Hashes[loc.x][loc.y] / 10000;
    }
    return 0;
}
int Is_Liquid(int Hash_Value) {
    if (Hash_Value >= 10000) {
        return Hash_Value;
    }
    return 0;
}
int Is_Water(place loc) {
    if (WITHIN_CLOSED_OPEN_INTERVAL(RetroSnake_Hashes[loc.x][loc.y], 20000, 30000)) {
        return (RetroSnake_Hashes[loc.x][loc.y] % 20000) / 100 + 1;
    }
    return 0;
}
int Is_Water(int Hash_Value) {
    if (Hash_Value >= 20000) {
        return ((Hash_Value % 20000) / 100) + 1;
    }
    return 0;
}
int Is_Lava(place loc) {
    if (WITHIN_CLOSED_INTERVAL(RetroSnake_Hashes[loc.x][loc.y], 10000, 20000)) {
        return (RetroSnake_Hashes[loc.x][loc.y] % 10000) / 100 + 1;
    }
    return 0;
}
int Is_Lava(int Hash_Value) {
    if (WITHIN_CLOSED_INTERVAL(Hash_Value, 10000, 20000)) {
        return ((Hash_Value % 10000) / 100) + 1;
    }
    return 0;
}
bool Is_Empty(place loc) {
    if (RetroSnake_Hashes[loc.x][loc.y] == -1) {
        return 1;
    }
    return 0;
}
bool Is_Empty(int Hash_Value) {
    if (Hash_Value == -1) {
        return 1;
    }
    return 0;
}
bool Is_Stone(place loc) {
    if (RetroSnake_Hashes[loc.x][loc.y] == 0) {
        return 1;
    }
    return 0;
}
bool Is_Stone(int Hash_Value) {
    if (Hash_Value == 0) {
        return 1;
    }
    return 0;
}
bool Is_Gravel(place loc) {
    if (RetroSnake_Hashes[loc.x][loc.y] == 30000) {
        return 1;
    }
    return 0;
}
bool Is_Gravel(int Hash_Value) {
    if (Hash_Value == 9999) {
        return 1;
    }
    return 0;
}
bool Is_Under_Sth(place loc, unordered_map<place, int> tmp) {
    if (tmp.find(loc) != tmp.end()) {
        return 1;
    }
    return 0;
}

void Foods_Create(int type, int max) {
    while (1) {
        place food;
        food.x = Random(1, RELATIVE_WIDTH - 2);
        food.y = Random(1, RELATIVE_HEIGHT - 2);
        //int type = value;
        if (RetroSnake_Hashes[food.x][food.y] == 0 && Adjacent_Unit_Amount(food, 0) > 3) {
            Max_Score_In_This_Layer += Reward[type];
            Draw_Image(GameHwnd, food.x * IMAGE_SIZE, food.y * IMAGE_SIZE, aw[type]);
            RetroSnake_Hashes[food.x][food.y] = 10 + type;
            break;
        }
        else {
            continue;
        }
    }
}
void Foods_Create(int* queue, int max) {
    while (1) {
        place food;
        food.x = Random(1, RELATIVE_WIDTH - 2);
        food.y = Random(1, RELATIVE_HEIGHT - 2);
        int T = Random(0, min(Depth, max - 1));
        int type = queue[T];

        if (RetroSnake_Hashes[food.x][food.y] == 0 && Adjacent_Unit_Amount(food, 0) > 3) {
            Max_Score_In_This_Layer += Reward[type];
            Draw_Image(GameHwnd, food.x * IMAGE_SIZE, food.y * IMAGE_SIZE, aw[type]);
            RetroSnake_Hashes[food.x][food.y] = 10 + type;
            break;
        }
        else {
            continue;
        }
    }
}
void Foods_Create(place loc, int type) {
    if (RetroSnake_Hashes[loc.x][loc.y] == 0 && Adjacent_Unit_Amount(loc, 0) > 3) {
        Max_Score_In_This_Layer += Reward[type];
        Draw_Image(GameHwnd, loc.x * IMAGE_SIZE, loc.y * IMAGE_SIZE, aw[type]);
        RetroSnake_Hashes[loc.x][loc.y] = 10 + type;
    }
}

void Foods_Spawn(int Amount) {
    //Max_Score_In_This_Layer = 0;
    Current_Foods_Amount = Amount;
    for (int i = 0; i < Foods_Amount; i++) {
        int queue[6] = { 0, 1, 2, 4, 5, 7 };
        Foods_Create(queue, 6);
    }
    if (Depth >= 10) {
        int amount = Random(1, 2);
        for (int i = 0; i <= amount; i++) {
            Foods_Create(3, 1);
        }
    }
    if (Depth >= 30) {
        int amount = Random(1, 2);
        for (int i = 0; i <= amount; i++) {
            int roll = Random(0, 100);
            if (roll < 30) {
                Foods_Create(6, 1);
            }
        }
    }
}

void Update_Snake_Head(HWND hWnd) {
    if (SnakePlace.size() < 2) 
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_Head.png");
    else if (SnakePlace[1] == LEFT(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_RightHead.png");
    else if (SnakePlace[1] == RIGHT(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_LeftHead.png");
    else if (SnakePlace[1] == UP(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_UpHead.png");
    else if (SnakePlace[1] == DOWN(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_DownHead.png");
}

void Update_Snake_Rail(HWND hWnd) {
    if (SnakePlace.size() < 2) return;
    if (SnakePlace[SnakePlace.size() - 1] == LEFT(SnakePlace[SnakePlace.size() - 2]))
        Draw_Image(hWnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Snake_LeftRail.png");
    else if (SnakePlace[SnakePlace.size() - 1] == RIGHT(SnakePlace[SnakePlace.size() - 2]))
        Draw_Image(hWnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Snake_RightRail.png");
    else if (SnakePlace[SnakePlace.size() - 1] == UP(SnakePlace[SnakePlace.size() - 2]))
        Draw_Image(hWnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Snake_UpRail.png");
    else if (SnakePlace[SnakePlace.size() - 1] == DOWN(SnakePlace[SnakePlace.size() - 2]))
        Draw_Image(hWnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Snake_DownRail.png");
}

void Check_Snake_To_Update(HWND hWnd, place Now, place Next, place Before) {
    if (Now.x == Before.x and Now.x == Next.x) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_UpDown.png");
    }
    else if (Now.y == Before.y and Now.y == Next.y) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftRight.png");
    }
    else if (Before == LEFT(Now) and Next == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftDown.png");
    }
    else if (Next == LEFT(Now) and Before == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftDown.png");
    }
    else if (Before == LEFT(Now) and Next == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftUp.png");
    }
    else if (Next == LEFT(Now) and Before == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftUp.png");
    }
    else if (Before == RIGHT(Now) and Next == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightDown.png");
    }
    else if (Next == RIGHT(Now) and Before == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightDown.png");
    }
    else if (Before == RIGHT(Now) and Next == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightUp.png");
    }
    else if (Next == RIGHT(Now) and Before == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightUp.png");
    }
    else
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"magma.png");
}

void Draw_Snake_According_To_Vector(HWND hWnd) {
    Update_Snake_Head(hWnd);

    for (int i = 1; i < SnakePlace.size() - 1; i++) {
        Check_Snake_To_Update(hWnd, SnakePlace[i], SnakePlace[i + 1], SnakePlace[i - 1]);
    }

    Update_Snake_Rail(hWnd);
}

void Random_Draw_Snake() {
    int head_x;
    int head_y;
    while (true) {
        head_x = Random(RELATIVE_WIDTH / 4, RELATIVE_WIDTH - RELATIVE_WIDTH / 4);
        head_y = Random(RELATIVE_HEIGHT / 4, RELATIVE_HEIGHT - RELATIVE_HEIGHT / 4);
        if (Adjacent_Unit_Amount(Translate(head_x, head_y), 0) == 4) break;
    }

    int x = head_x;
    int y = head_y;
    RetroSnake_Hashes[x][y] = 1;
    SnakePlace.push_back(Translate(x, y));

    for (int i = 1; i < Length; i++) {
        int cnt = 0;
        int list[4] = { 2, 1, 3, 4 };
        while (1) {
            if (cnt >= 4) break;
            int forward = Detect(Translate(x, y), 0, list[cnt]);
            if (forward == 1 && x - 1 >= 0)
                if (RetroSnake_Hashes[x - 1][y] == 0)
                    if (Adjacent_Unit_Amount(Translate(x - 1, y), 0) > 1) {
                        SnakePlace.push_back(Translate(--x, y));
                        RetroSnake_Hashes[x][y] = 1;
                        break;
                    }
            if (forward == 2 && x + 2 < RELATIVE_WIDTH)
                if (RetroSnake_Hashes[x + 1][y] == 0)
                    if (Adjacent_Unit_Amount(Translate(x + 1, y), 0) > 1) {
                        SnakePlace.push_back(Translate(++x, y));
                        RetroSnake_Hashes[x][y] = 1;
                        break;
                    }
            if (forward == 3 && y - 1 >= 0)
                if (RetroSnake_Hashes[x][y - 1] == 0)
                    if (Adjacent_Unit_Amount(Translate(x, y - 1), 0) > 1) {
                        SnakePlace.push_back(Translate(x, --y));
                        RetroSnake_Hashes[x][y] = 1;
                        break;
                    }
            if (forward == 4 && y + 2 < RELATIVE_HEIGHT)
                if (RetroSnake_Hashes[x][y + 1] == 0)
                    if (Adjacent_Unit_Amount(Translate(x, y + 1), 0) > 1) {
                        SnakePlace.push_back(Translate(x, ++y));
                        RetroSnake_Hashes[x][y] = 1;
                        break;
                    }
            cnt++;
        }
    }

    Draw_Snake_According_To_Vector(GameHwnd);
}

void Random_Draw_Obstacle() {
    place center;
    while (1) {
        center.x = Random(0, RELATIVE_WIDTH - 2);
        center.y = Random(0, RELATIVE_HEIGHT - 2);
        if (RetroSnake_Hashes[center.x][center.y] == 0) break;
    }
    int Type = Random(1, 3);
    switch (Type) {
    case 1: Draw_Image(GameHwnd, center.x * IMAGE_SIZE, center.y * IMAGE_SIZE, L"Lava/Lava_1.png"); break;
    case 2: Draw_Image(GameHwnd, center.x * IMAGE_SIZE, center.y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); break;
    case 3: Draw_Image(GameHwnd, center.x * IMAGE_SIZE, center.y * IMAGE_SIZE, L"Blocks/Gravel.png"); break;
    }
    
    int x = center.x;
    int y = center.y;
    int cnt = 0;
    int list[4] = { 1, 2, 3, 4 };
    int Obstacle_Size = Random(1, 8);
    if (Type == 3) 
        Obstacle_Size = Random(8, 64);
    else 
        Obstacle_Size = Random(4, 32);
    
    

    place temp = { x, y };
    temp.flag = 0;

    Liq[0].emplace(std::pair<place, int>(temp, Type));
    RetroSnake_Hashes[x][y] = 10000 * Type;
    ObstaclePlace.push_back(Translate(x, y));

    for (int i = 1; i < Obstacle_Size && cnt <= 2 * Obstacle_Size; i++) {
        while (1) {
            if (cnt >= 4) break;
            int forward = Detect(Translate(x, y), Type * 10000, list[cnt]);
            if (forward == 1 && x - 1 >= 0 && RetroSnake_Hashes[x - 1][y] == 0) {
                cnt = 0;
                ObstaclePlace.push_back(LEFT(Translate(x, y)));
                RetroSnake_Hashes[x - 1][y] = 10000 * Type;
                x -= 1;
                switch (Type) {
                case 1: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Lava/Lava_1.png"); break;
                case 2: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); break;
                case 3: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Blocks/Gravel.png"); break;
                }
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 2 && x + 2 < RELATIVE_WIDTH && RetroSnake_Hashes[x + 1][y] == 0) {
                cnt = 0;
                ObstaclePlace.push_back(RIGHT(Translate(x, y)));
                RetroSnake_Hashes[x + 1][y] = 10000 * Type;
                x += 1;
                switch (Type) {
                case 1: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Lava/Lava_1.png"); break;
                case 2: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); break;
                case 3: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Blocks/Gravel.png"); break;
                }
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 3 && y - 1 >= 0 && RetroSnake_Hashes[x][y - 1] == 0) {

                cnt = 0;
                ObstaclePlace.push_back(UP(Translate(x, y)));
                RetroSnake_Hashes[x][y - 1] = 10000 * Type;
                y -= 1;
                switch (Type) {
                case 1: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Lava/Lava_1.png"); break;
                case 2: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); break;
                case 3: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Blocks/Gravel.png"); break;
                }
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 4 && y + 2 < RELATIVE_HEIGHT && RetroSnake_Hashes[x][y + 1] == 0) {

                cnt = 0;
                ObstaclePlace.push_back(DOWN(Translate(x, y)));
                RetroSnake_Hashes[x][y + 1] = 10000 * Type;
                y += 1;
                switch (Type) {
                case 1: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Lava/Lava_1.png"); break;
                case 2: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); break;
                case 3: Draw_Image(GameHwnd, x * IMAGE_SIZE, y * IMAGE_SIZE, L"Blocks/Gravel.png"); break;
                }
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            cnt++;
            break;
        }
    }
}

int Adjacent_Unit_Amount(place location, int type) {
    int cnt = 0;
    if (location.x + 1 < RELATIVE_WIDTH)
        if (RetroSnake_Hashes[location.x + 1][location.y] == type)
            cnt++;
    if (location.x - 1 >= 0)
        if (RetroSnake_Hashes[location.x - 1][location.y] == type)
            cnt++;
    if (location.y + 1 < RELATIVE_HEIGHT)
        if (RetroSnake_Hashes[location.x][location.y + 1] == type)
            cnt++;
    if (location.y - 1 >= 0)
        if (RetroSnake_Hashes[location.x][location.y - 1] == type)
            cnt++;
    return cnt;
}

int Surrounding_Unit_Amount(place location, int type) {
    int cnt = 0;
    if (location.x - 1 >= 0 && location.y - 1 >= 0)
        if (RetroSnake_Hashes[location.x - 1][location.y - 1] == type)
            cnt++;
    if (location.x + 2 < RELATIVE_WIDTH && location.y - 1 >= 0)
        if (RetroSnake_Hashes[location.x + 1][location.y - 1] == type)
            cnt++;
    if (location.x - 1 >= 0 && location.y + 2 < RELATIVE_HEIGHT)
        if (RetroSnake_Hashes[location.x - 1][location.y + 1] == type)
            cnt++;
    if (location.x + 2 < RELATIVE_WIDTH && location.y + 2 < RELATIVE_HEIGHT)
        if (RetroSnake_Hashes[location.x + 1][location.y + 1] == type)
            cnt++;
    if (location.x + 1 < RELATIVE_WIDTH && location.y >= 0)
        if (RetroSnake_Hashes[location.x + 1][location.y] == type)
            cnt++;
    if (location.x - 1 >= 0 && location.y >= 0)
        if (RetroSnake_Hashes[location.x - 1][location.y] == type)
            cnt++;
    if (location.y + 2 < RELATIVE_WIDTH && location.x >= 0)
        if (RetroSnake_Hashes[location.x][location.y + 1] == type)
            cnt++;
    if (location.y - 1 >= 0 && location.x >= 0)
        if (RetroSnake_Hashes[location.x][location.y - 1] == type)
            cnt++;
    return cnt;
}

void Shorten(int begin, int model) {
    // Need to debug
    if (Length < begin and Owe_Length) {
        Owe_Length -= begin;
        return;
    }

    if (begin >= SnakePlace.size() - 1) { 
        begin = SnakePlace.size() - 1; 
        Draw_Image(GameHwnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_Head.png");
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
    }
    if (SnakePlace.size() == 1) return;
    int i = 1;
    auto it = SnakePlace.end() - 1;
    while (i++) {
        //putimage((*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, &bp);
        if (model == 1)
            Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, L"Blocks/Stone.png");
        else if (model == 2)
            Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        RetroSnake_Hashes[(*it).x][(*it).y] = 0;
        SnakePlace.pop_back();
        it = SnakePlace.end() - 1;
        Length--;
        if (i > begin) break;
    }
    return;
    /*setlinecolor(Snake_Color);
    if (commonsize.model) line(commonsize.x * UNIT_SIZE, commonsize.y * UNIT_SIZE, (commonsize.x + 1) * UNIT_SIZE, commonsize.y * UNIT_SIZE);
    else line(commonsize.x * UNIT_SIZE, commonsize.y * UNIT_SIZE, commonsize.x * UNIT_SIZE, (commonsize.y + 1) * UNIT_SIZE);*/
}

void Moving_Upward(bool lengthen, int type = -1) {
    if (lengthen) {
        Length++;
        SnakePlace.push_back(SnakePlace[0]);
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }

        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
        SnakePlace[0].y--;
        if (WITHIN_CLOSED_INTERVAL(type, 10, 20))
            Eating(type);
    }
    else {
        RetroSnake_Hashes[SnakePlace[SnakePlace.size() - 1].x][SnakePlace[SnakePlace.size() - 1].y] = -1;
        Draw_Image(GameHwnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        SnakePlace[0].y--;
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
    }
    if (Length > 2)
        Check_Snake_To_Update(GameHwnd, SnakePlace[1], SnakePlace[2], SnakePlace[0]);
    Update_Snake_Head(GameHwnd);
    Update_Snake_Rail(GameHwnd);
}
void Moving_Downward(bool lengthen, int type = -1) {
    if (lengthen) {
        Length++;
        SnakePlace.push_back(SnakePlace[0]);
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
        SnakePlace[0].y++;
        if (WITHIN_CLOSED_INTERVAL(type, 10, 20))
            Eating(type);
    }
    else {
        RetroSnake_Hashes[SnakePlace[SnakePlace.size() - 1].x][SnakePlace[SnakePlace.size() - 1].y] = -1;
        Draw_Image(GameHwnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        SnakePlace[0].y++;
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
    }
    if (Length > 2)
        Check_Snake_To_Update(GameHwnd, SnakePlace[1], SnakePlace[2], SnakePlace[0]);
    Update_Snake_Head(GameHwnd);
    Update_Snake_Rail(GameHwnd);
}
void Moving_Left(bool lengthen, int type = -1) {
    if (lengthen) {
        Length++;
        SnakePlace.push_back(SnakePlace[0]);
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
        SnakePlace[0].x--;
        if (WITHIN_CLOSED_INTERVAL(type, 10, 20))
            Eating(type);
    }
    else {
        RetroSnake_Hashes[SnakePlace[SnakePlace.size() - 1].x][SnakePlace[SnakePlace.size() - 1].y] = -1;
        Draw_Image(GameHwnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        SnakePlace[0].x--;
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
    }
    if (Length > 2)
        Check_Snake_To_Update(GameHwnd, SnakePlace[1], SnakePlace[2], SnakePlace[0]);
    Update_Snake_Head(GameHwnd);
    Update_Snake_Rail(GameHwnd);
}
void Moving_Right(bool lengthen, int type = -1) {
    if (lengthen) {
        Length++;
        SnakePlace.push_back(SnakePlace[0]);
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
        SnakePlace[0].x++;
        if (WITHIN_CLOSED_INTERVAL(type, 10, 20))
            Eating(type);
    }
    else {
        RetroSnake_Hashes[SnakePlace[SnakePlace.size() - 1].x][SnakePlace[SnakePlace.size() - 1].y] = -1;
        Draw_Image(GameHwnd, SnakePlace[SnakePlace.size() - 1].x * IMAGE_SIZE, SnakePlace[SnakePlace.size() - 1].y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        for (int i = Length - 1; i > 0; i--) {
            SnakePlace[i] = SnakePlace[i - 1];
        }
        SnakePlace[0].x++;
        RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y] = 1;
    }
    if (Length > 2)
        Check_Snake_To_Update(GameHwnd, SnakePlace[1], SnakePlace[2], SnakePlace[0]);
    Update_Snake_Head(GameHwnd);
    Update_Snake_Rail(GameHwnd);
}

void Water_Clear(place loc) {
    for (int i = 0; i < Liq.size(); i++) {
        auto it = Liq[i].find({ loc });
        if (it != Liq[i].end()) 
            Liq[i].erase(it);
    }
}

void Water_Spreading() {
    queue<place> ToHandle;
    queue<place> ToClear;
    // lvl 水的高度，从 7 到 0
    // dist 水蔓延的距离 从 0 到 7

    for (int i = Liq.size() - 1; i >= 0; i--) {
        int cnt = 0;
        for (auto it = Liq[i].begin(); it != Liq[i].end() and cnt <= Liq[i].size() - 1; ++it, ++cnt) {
            place pos = it->first;
            int curr_x = pos.x;
            int curr_y = pos.y;
            int distance = i;
            int Type = it->second;
            if (Type == 1)
                ToClear.push(pos);
            // 1 Lava; 2 Water
            if (RetroSnake_Hashes[curr_x][curr_y] == -1 and i > 0) {
                RetroSnake_Hashes[curr_x][curr_y] = Type * 10000 + distance * 100;
                if (Type == 2)
                    Draw_Image(GameHwnd, curr_x * IMAGE_SIZE, curr_y * IMAGE_SIZE, KidsWaterList[9][i - 1]);
                else if (Type == 1 and i < 4)
                    Draw_Image(GameHwnd, curr_x * IMAGE_SIZE, curr_y * IMAGE_SIZE, KidsLavaList[0][i - 1]);
            }

            std::vector<std::pair<int, int>> directions = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
            for (const auto& dir : directions) {
                int dx = dir.first;
                int dy = dir.second;
                int new_x = curr_x + dx;
                int new_y = curr_y + dy;
                
                if (new_x >= 0 && new_x < RELATIVE_WIDTH && new_y >= 0 && new_y < RELATIVE_HEIGHT)
                    if (RetroSnake_Hashes[new_x][new_y] == -1) {
                        if (Type == 1) {
                            if (i < Liq.size() - 5) {
                                int flag = true;
                                auto nit = Liq[0].find({ new_x, new_y });
                                if (nit != Liq[0].end()) flag = false;

                                for (int i = Liq.size() - 1; i > 0; i--) {
                                    nit = Liq[i].find({ new_x, new_y });
                                    if (nit != Liq[i].end()) flag = false;
                                }
                                if (flag)
                                    Liq[i + 1].insert(std::pair<place, int>({ new_x, new_y }, Type));
                            }
                        }
                        else if (Type == 2) {
                            if (i < Liq.size() - 1) {
                                int flag = true;
                                auto nit = Liq[0].find({ new_x, new_y });
                                if (nit != Liq[0].end()) flag = false;

                                for (int i = Liq.size() - 1; i > 0; i--) {
                                    nit = Liq[i].find({ new_x, new_y });
                                    if (nit != Liq[i].end()) flag = false;
                                }
                                if (flag)
                                    Liq[i + 1].insert(std::pair<place, int>({ new_x, new_y }, Type));
                            }
                        }
                    }

                    else if (RetroSnake_Hashes[new_x][new_y] >= 10000)  {
                        if (Type == 1 and RetroSnake_Hashes[curr_x][curr_y] < 20000) {
                            int new_lvl = 7 - (RetroSnake_Hashes[new_x][new_y] % 10000) / 100;
                            int before_lvl = 7 - (RetroSnake_Hashes[curr_x][curr_y] % 10000) / 100;
                            int test_lvl = 7 - i;
                            // i 是蔓延距离
                            //int new_lvl = Liq
                            if (new_lvl < test_lvl - 1 and i < Liq.size() - 5) {
                                for (int i = Liq.size() - 1; i >= 0; i--)
                                    Liq[i].erase({ new_x, new_y });

                                //Liq[7 - new_lvl].erase({ new_x, new_y });
                                Liq[i + 1].insert(std::pair<place, int>({ new_x, new_y }, Type));
                                RetroSnake_Hashes[new_x][new_y] = 10000 + distance * 100 + 100;
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, KidsLavaList[0][i]);
                            }
                        }
                        else if (Type == 2) {
                            int new_lvl = 7 - (RetroSnake_Hashes[new_x][new_y] % 10000) / 100;
                            int before_lvl = 7 - (RetroSnake_Hashes[curr_x][curr_y] % 10000) / 100;
                            int test_lvl = 7 - i;
                            // i 是蔓延距离
                            //int new_lvl = Liq
                            if (new_lvl < test_lvl - 1 and i < Liq.size() - 1) {
                                for (int i = Liq.size() - 1; i >= 0; i--)
                                    Liq[i].erase({ new_x, new_y });

                                //Liq[7 - new_lvl].erase({ new_x, new_y });
                                Liq[i + 1].insert(std::pair<place, int>({ new_x, new_y }, Type));
                                RetroSnake_Hashes[new_x][new_y] = 20000 + distance * 100 + 100;
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, KidsWaterList[0][i]);
                            }
                        }
                    }
                        
            }
        }
    }

    while (!ToClear.empty()) {
        place pos = ToClear.front();
        Solidification(pos);
        ToClear.pop();
    }
}

void Check_Drying_Blocks() {
    vector<vector<int>> NewRetroSnake_Hashes(RELATIVE_WIDTH + 1, vector<int>(RELATIVE_HEIGHT + 1, -1));
    vector< unordered_map<place, int>> NewLiquid(8);
    NewLiquid[0].insert(Liq[0].begin(), Liq[0].end());

    for (int i = 0; i < NewLiquid.size(); i++) {
        for (auto it = NewLiquid[i].begin(); it != NewLiquid[i].end(); it++) {
            int curr_x = it->first.x;
            int curr_y = it->first.y;
                
            int distance = i;

            std::vector<std::pair<int, int>> directions = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
            for (const auto& dir : directions) {
                int dx = dir.first;
                int dy = dir.second;
                int new_x = curr_x + dx;
                int new_y = curr_y + dy;
                if (new_x >= 0 && new_x < RELATIVE_WIDTH && new_y >= 0 && new_y < RELATIVE_HEIGHT) 
                    if (NewRetroSnake_Hashes[new_x][new_y] == -1 and RetroSnake_Hashes[new_x][new_y] >= 10000) {
                        NewRetroSnake_Hashes[new_x][new_y] = 20000 + distance * 100;
                        if (i < Liq.size() - 1) NewLiquid[i + 1].insert(std::pair<place, int>({ new_x, new_y }, i + 1));
                    }
            }
        }
    }

    //for (int i = 0; i < RELATIVE_WIDTH; i++) {
    //    for (int j = 0; j < RELATIVE_HEIGHT; j++) {
    //        if (RetroSnake_Hashes[i][j] > 10000) {
    //            if (NewRetroSnake_Hashes[i][j] == -1) {
    //                int lvl = 7 - ((RetroSnake_Hashes[i][j]) % 10000) / 100;
    //                RetroSnake_Hashes[i][j] = -1;
    //                DryingBlock.insert(std::pair<place, int>({ i, j }, lvl));
    //                Liq[7 - lvl].erase({i, j});
    //            }
    //        }
    //    }
    //}

    for (int i = 0; i < Liq.size(); i++) {
        auto it = Liq[i].begin();
        while (it != Liq[i].end()) {
            auto newit = NewLiquid[i].find(it->first);
            if (newit == NewLiquid[i].end() and RetroSnake_Hashes[it->first.x][it->first.y] >= 10000) {
                RetroSnake_Hashes[it->first.x][it->first.y] = -1;
                DryingBlock.insert(std::pair<place, int>(it->first, i));
                it = Liq[i].erase(it);
            }
            else it++;
        }
    }

}

void Blocks_Drying() {
    if (DryingBlock.empty()) return;
    auto it = DryingBlock.begin();
    while (it != DryingBlock.end()) {
        const auto& key = it->first;
        if (RetroSnake_Hashes[key.x][key.y] != 1) {
            Draw_Image(GameHwnd, key.x * IMAGE_SIZE, key.y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        }
        it = DryingBlock.erase(it); // 删除当前元素，并自动更新迭代器到下一个
    }
}

int KeyBoard_Input(int userKey, bool Passive) {
    int value = 0;
    switch (userKey)
    {
    case 'w':
    case 'W':
    case -72: {
        if (SnakePlace[0].y != 0) {
            if (Owe_Length) {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] <= 0) {
                    Owe_Length--;
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                    if (In_The_Water) { 
                        In_The_Water = false; 
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10000 and Passive) {
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] < 1000) {
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y - 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] << endl;
                    return 0;
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] <= 0) {
                    Moving_Upward(false);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10000 and Passive) {
                    Moving_Upward(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] < 1000) {
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y - 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] << endl;
                    return 0;
                }
            }
            Total_Steps++;
            value = 1;
        }
        //else GameOver();
        break;
    }
    case 's':
    case 'S':
    case -80: {
        if (SnakePlace[0].y < RELATIVE_HEIGHT - 2) {
            if (Owe_Length) {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] <= 0) {
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                    Owe_Length--;
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10000 and Passive) {
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] < 1000) {
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                    
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y + 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] << endl;
                    return 0;
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] <= 0) {
                    Moving_Downward(false);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10000 and Passive) {
                    Moving_Downward(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] < 1000) {
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                    
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y + 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] << endl;
                    return 0;
                }
            }
            Total_Steps++;
            value = 1;
        }  
        //else GameOver();
        break;
    }
    case 'A':
    case 'a':
    case -75: {
        if (SnakePlace[0].x != 0) {
            if (Owe_Length){
                if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] <= 0) {
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                    Owe_Length--;
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10000 and Passive) {
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] < 1000) {
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x - 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] << endl;
                    return 0;
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] <= 0) {
                    Moving_Left(false);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10000 and Passive) {
                    Moving_Left(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] < 1000) {
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x - 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] << endl;
                    return 0;
                }
            }
            Total_Steps++;
            value = 1;
        }
        //else GameOver();
        break;
    }
    case 'd':
    case 'D':
    case -77: {
        if (SnakePlace[0].x < RELATIVE_WIDTH - 1) {
            if (Owe_Length) {
                if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] <= 0) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    Owe_Length--;
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10000 and Passive) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] < 1000) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x + 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] << endl;
                    return 0;
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] <= 0) {
                    Moving_Right(false);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10000 and Passive) {
                    Moving_Right(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    if (!In_The_Water) In_The_Water = true;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] < 1000) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    if (In_The_Water) {
                        In_The_Water = false;
                        Oxygen = 10;
                        Refresh_ScoreBoard(10);
                    }
                }
                else {
                    cout << "You dead in " << SnakePlace[0].x + 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] << endl;
                    return 0;
                }
            }
            Total_Steps++;
            value = 1;
        }
        //else GameOver();
        break;
    }
    }
    return value;
}

void GameOver() {
    Running = false;
    RetroSnake_Destruction();
}

void RetroSnake_Hashes_Info() {
    char buffer[256];
    int x, y;
    OutputDebugStringA("[System] Enter the location to info\n");
}

int Fortune_Enchantment(int amount) {
    if (Fortune_Times <= 0) return amount;
    switch (Fortune) {
    case 0: return amount;
    case 1: {
        int roll = Random(0, 30);
        if (roll < 10) return 2 * amount;
        else return amount;
    }
    case 2: {
        int roll = Random(0, 100);
        if (roll < 25) return 3 * amount;
        else if (roll < 50) return 2 * amount;
        else return amount;
    }
    case 3: {
        int roll = Random(0, 100);
        if (roll < 20) return 4 * amount;
        else if (roll < 40) return 3 * amount;
        else if (roll < 60) return 2 * amount;
        else return amount;
    }
    }
    return 0;
}

void Eating(int type) {
    Current_Foods_Amount--;
    int amount = 1;
    if (type != 13 and type != 16 and Fortune and Fortune_Times > 0 and Awards[5] > 0) {
        Fortune_Times--;
        Awards[5]--;
        Refresh_ScoreBoard(5);
        Refresh_ScoreBoard(11);
        amount = Fortune_Enchantment(1);
    }
    if (Fortune == 0) Fortune_Times = 0;
    Awards[type - 10] += amount;
    Theoretical_Score += Reward[type - 10] * amount;
    Curr_Score_In_This_Layer += Reward[type - 10] * amount;
    
    if (type == 11) {
        int extra = Random(1, 4);
        extra = Fortune_Enchantment(extra);
        Awards[type - 10] += extra;
        Theoretical_Score += 2 * extra;
    }
    else if (type == 13)
        Event();
    else if (type == 15) {
        int amount = Fortune_Enchantment(Random(3, 8));
        Awards[type - 10] += amount;
    }
    else if (type == 16) {
        Shorten(SnakePlace.size() - 3);
        Owe_Length = 0;
    }

    Refresh_ScoreBoard(type - 10);
}

void Level_Up() {
    Level++;
    Depth++;
    Refresh_ScoreBoard(8);

    BreakTime *= 0.8;

    Max_Steps = Depth * RELATIVE_WIDTH * RELATIVE_HEIGHT;
    //Multiply += 0.5;
    for(int i = 0; i < Liq.size(); i++)
        Liq[i].clear();
    unordered_map<place, int> Temp_Map(ObstacleAbove);
    ObstacleAbove.clear();
    Inherit(Temp_Map);
    
    
    //Clear_Windows();
    Owe_Length = Length - 1 + Owe_Length;
    Shorten(SnakePlace.size() - 1, 1);
    //Length = 1;
    int temp = Random(1, 4);
    for (int i = 0; i < temp; i++) {
        Random_Draw_Obstacle();
    }
    Curr_Score_In_This_Layer = 0;
    Max_Score_In_This_Layer = 0;
    Foods_Spawn(Foods_Amount - Existing_Ores);
}

int Detect(place location, int type, int model) {
    place ans;
    int cnt = 0;

    vector<pathfinding> loc;
    if (location.x + 2 < RELATIVE_WIDTH) {
        pathfinding T;
        T.forward = 1;
        T.value = Surrounding_Unit_Amount(Translate(location.x + 1, location.y), type);
        loc.push_back(T);
    }
    if (location.x - 1 >= 0) {
        pathfinding T;
        T.forward = 2;
        T.value = Surrounding_Unit_Amount(Translate(location.x - 1, location.y), type);
        loc.push_back(T);
    }
    if (location.y + 2 < RELATIVE_HEIGHT) {
        pathfinding T;
        T.forward = 3;
        T.value = Surrounding_Unit_Amount(Translate(location.x, location.y + 1), type);
        loc.push_back(T);
    }
    if (location.y - 1 >= 0) {
        pathfinding T;
        T.forward = 4;
        T.value = Surrounding_Unit_Amount(Translate(location.x, location.y - 1), type);
        loc.push_back(T);
    }

    //int SecondLargestValue = quickSelect(loc, 0, loc.size() - 1, model);
    Quick_Sort(loc, 0, loc.size() - 1);
    if (model > loc.size()) model = loc.size();
    int SecondLargestValue = loc[loc.size() - model].value;

    vector<std::function<place(place)>> functions = {
        [](place loc) -> place { return LEFT(loc); },
        [](place loc) -> place { return RIGHT(loc); },
        [](place loc) -> place { return DOWN(loc); },
        [](place loc) -> place { return UP(loc); }
    };

    random_device rd;
    mt19937 g(rd());
    std::shuffle(functions.begin(), functions.end(), g);
    // ???????????????

    for (const auto& func : functions) {
        place T = func(location);
        if (Surrounding_Unit_Amount(Translate(T.x, T.y), type) == SecondLargestValue) {
            ans = T;
            break;
        }
    }

    if (ans.x == location.x - 1 && ans.y == location.y) {
        return 1;
    }
    else if (ans.x == location.x + 1 && ans.y == location.y) {
        return 2;
    }
    else if (ans.x == location.x && ans.y == location.y - 1) {
        return 3;
    }
    else if (ans.x == location.x && ans.y == location.y + 1) {
        return 4;
    }

}

void Inherit(unordered_map<place, int> temp) {
    for (int i = 0; i < RELATIVE_WIDTH; i++) {
        for (int j = 0; j < RELATIVE_HEIGHT; j++) {
            if (Is_Empty({ i, j }) or Is_Stone({ i, j }) or Is_Under_Sth({i, j}, temp)) {
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                RetroSnake_Hashes[i][j] = 0;
            }
            else if (Is_Ores({ i, j })) {
                int roll = Random(0, 100);
                if (roll <= 20) {
                    //To_Handle.push(std::pair<place, int>({ i, j }, RetroSnake_Hashes[i][j]));
                    Existing_Ores++;
                }
                else {
                    Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                    RetroSnake_Hashes[i][j] = 0;
                }
            }
            else if (Is_Water({ i, j })) {
                RetroSnake_Hashes[i][j] = -2;
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/Stone_Under_Water.png");
                ObstacleAbove.insert(std::pair<place, int>({i, j}, 2));
            }
            else if (Is_Lava({ i, j })) {
                RetroSnake_Hashes[i][j] = -2;
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/Stone_Under_Lava.png");
                ObstacleAbove.insert(std::pair<place, int>({ i, j }, 1));
            }
            else if (Is_Gravel({ i, j })) {
                RetroSnake_Hashes[i][j] = -2;
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/Stone_Under_Gravel.png");
                ObstacleAbove.insert(std::pair<place, int>({ i, j }, 3));
            }
        }
    }
}

void Clear_Windows() {
    for (int i = 0; i < RELATIVE_WIDTH; i++) {
        for (int j = 0; j < RELATIVE_HEIGHT; j++) {
            if (i == SnakePlace[0].x and j == SnakePlace[0].y)
                continue;
            Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
            RetroSnake_Hashes[i][j] = 0;
        }
    }
}

void Gdiplus_Startup_Wrapper() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
}

void Gdiplus_Shutdown_Wrapper() {
    Gdiplus::GdiplusShutdown(g_gdiplusToken);
}

void Water_Vertical_Flow() {
    for (auto it = ObstacleAbove.begin(); it != ObstacleAbove.end(); it++) {
        if (RetroSnake_Hashes[it->first.x][it->first.y] == -1) {
            RetroSnake_Hashes[it->first.x][it->first.y] = it->second * 10000;
            switch (it->second) {
            case 1: {
                Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, L"Lava/Lava_1.png"); 
                Liq[0].insert(std::pair<place, int>(it->first, it->second));
                break; 
            }
            case 2: {
                Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, L"Water_Still/Water_Still_1.png"); 
                Liq[0].insert(std::pair<place, int>(it->first, it->second));
                break; 
            }
            case 3: {
                Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, L"Blocks/Gravel.png");
                it = ObstacleAbove.erase(it);
                break;
            }
            }
        }
        if (it == ObstacleAbove.end()) break;
    }
}

void InitD2D(HWND hwnd) {
    // 创建D2D工厂
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

    // 获取窗口大小
    RECT rc;
    GetClientRect(hwnd, &rc);

    // 创建渲染目标
    pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        &pRenderTarget);

    // 创建DWrite工厂
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

    // 创建文本格式
    pDWriteFactory->CreateTextFormat(
        L"Gabriola",                // 字体名称
        nullptr,                    // 字体集合
        DWRITE_FONT_WEIGHT_REGULAR, // 字体粗细
        DWRITE_FONT_STYLE_NORMAL,   // 字体样式
        DWRITE_FONT_STRETCH_NORMAL, // 字体拉伸
        50.0f,                      // 字体大小
        L"en-us",                   // 本地化
        &pTextFormat);

    // 创建画刷
    pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &pBlackBrush);
}

void CleanupD2D() {
    if (pBlackBrush) pBlackBrush->Release();
    if (pTextFormat) pTextFormat->Release();
    if (pDWriteFactory) pDWriteFactory->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pD2DFactory) pD2DFactory->Release();
}

void OnPaint(HWND hwnd) {
    if (pRenderTarget == nullptr) {
        // 如果渲染目标没有创建成功，直接返回
        return;
    }

    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);

    pRenderTarget->BeginDraw();

    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Red, 0.0f));

    // 设置文本的颜色
    //ComPtr<ID2D1SolidColorBrush> pBrush;
    //pRenderTarget->CreateSolidColorBrush(
    //    D2D1::ColorF(D2D1::ColorF::White),
    //    &pBrush);
    // 创建一个矩形区域作为文本的背景
    D2D1_RECT_F backgroundRect = D2D1::RectF(0, 0, 272, 600);

    // 创建画刷并设置为半透明黑色
    ID2D1SolidColorBrush* pTransparentBlackBrush;
    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(0, 0, 0)), &pTransparentBlackBrush);

    // 绘制文本背景
    pRenderTarget->FillRectangle(&backgroundRect, pTransparentBlackBrush);

    // 释放画刷
    pTransparentBlackBrush->Release();

    // 创建一个矩形区域
    D2D1_RECT_F layoutRect = D2D1::RectF(0, 0, 272, 600);

    // 绘制文本
    pRenderTarget->DrawText(
        L"Hello, Direct2D!",        // 文本
        wcslen(L"Hello, Direct2D!"),// 文本长度
        pTextFormat,                // 文本格式
        layoutRect,                 // 布局矩形
        pBlackBrush);               // 画刷

    HRESULT hr = pRenderTarget->EndDraw();
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to draw text", L"Error", MB_OK);
    }

    EndPaint(hwnd, &ps);
}

void Refresh_ScoreBoard(int Type) {
    switch (Type) {
    case -1: {
        Draw_Image(ScoreBoardHwnd, 16, 32 + 64, L"Ores/coal.png");
        char coal[256] = "\0";
        sprintf(coal, "%d", Awards[0]);
        wchar_t wcoal[256];
        mbstowcs(wcoal, coal, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 32 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wcoal);

        Draw_Image(ScoreBoardHwnd, 16, 64 + 64, L"Ores/copper_ingot.png");
        char copper[256] = "\0";
        sprintf(copper, "%d", Awards[0]);
        wchar_t wcopper[256];
        mbstowcs(wcopper, copper, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 64 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wcopper);

        Draw_Image(ScoreBoardHwnd, 16, 96 + 64, L"Ores/iron_ingot.png");
        char iron[256] = "\0";
        sprintf(iron, "%d", Awards[0]);
        wchar_t wiron[256];
        mbstowcs(wiron, iron, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 96 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wiron);

        Draw_Image(ScoreBoardHwnd, 16, 128 + 64, L"Ores/gold_ingot.png");
        char gold[256] = "\0";
        sprintf(gold, "%d", Awards[0]);
        wchar_t wgold[256];
        mbstowcs(wgold, gold, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 128 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wgold);

        Draw_Image(ScoreBoardHwnd, 16, 160 + 64, L"Ores/lapis_lazuli.png");
        char lapis[256] = "\0";
        sprintf(lapis, "%d", Awards[0]);
        wchar_t wlapis[256];
        mbstowcs(wlapis, lapis, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 160 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wlapis);

        Draw_Image(ScoreBoardHwnd, 16, 192 + 64, L"Ores/diamond.png");
        char diamond[256] = "\0";
        sprintf(diamond, "%d", Awards[0]);
        wchar_t wdiamond[256];
        mbstowcs(wdiamond, diamond, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 192 + 4 + 64, Color(255, 255, 255, 255), L"Monaco", 16, wdiamond);

        char depth[256] = "\0";
        sprintf(depth, "%d", Depth);
        wchar_t wdepth[256];
        mbstowcs(wdepth, depth, 256);
        Draw_Text(ScoreBoardHwnd, false, 8, 480, Color(255, 255, 255, 255), L"Monaco", 16, wdepth);

        for (int i = 0; i < Health / 2; i++) {
            Draw_Image(ScoreBoardHwnd, 38 + 18 * i, 544, L"Health.png");
        }
        if (Health % 2)
            Draw_Image(ScoreBoardHwnd, 38 + ((Health / 2) + 1) * 18, 544, L"Half_Health.png");

        for (int i = 0; i < Oxygen; i++) {
            Draw_Image(ScoreBoardHwnd, 38 + 18 * i, 18 * IMAGE_SIZE, L"Oxygen.png");
        }
        break;
    }
    case 0: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 3 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char coal[256] = "\0";
        sprintf(coal, "%d", Awards[0]);
        wchar_t wcoal[256];
        mbstowcs(wcoal, coal, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 96 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wcoal);
        break;
        //Actrual_Score
    }
    case 1: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 4 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char copper[256] = "\0";
        sprintf(copper, "%d", Awards[1]);
        wchar_t wcopper[256];
        mbstowcs(wcopper, copper, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 128 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wcopper);
        break;
    }
    case 2: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 5 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char iron[256] = "\0";
        sprintf(iron, "%d", Awards[2]);
        wchar_t wiron[256];
        mbstowcs(wiron, iron, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 160 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wiron);
        break;
    }
    case 4: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 6 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char gold[256] = "\0";
        sprintf(gold, "%d", Awards[4]);
        wchar_t wgold[256];
        mbstowcs(wgold, gold, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 192 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wgold);
        break;
    }
    case 5: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 7 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char lapis[256] = "\0";
        sprintf(lapis, "%d", Awards[5]);
        wchar_t wlapis[256];
        mbstowcs(wlapis, lapis, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 224 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wlapis);
        break;
    }
    case 7: {
        for (int i = 0; i < 6; i++) {
            Draw_Image(ScoreBoardHwnd, (2 + i) * IMAGE_SIZE, 8 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char diamond[256] = "\0";
        sprintf(diamond, "%d", Awards[7]);
        wchar_t wdiamond[256];
        mbstowcs(wdiamond, diamond, 256);
        Draw_Text(ScoreBoardHwnd, false, 64, 256 + 4, Color(255, 255, 255, 255), L"Monaco", 16, wdiamond);
        break;
    }
    case 8: {
        for (int i = 0; i < 8; i++) {
            Draw_Image(ScoreBoardHwnd, i * IMAGE_SIZE, 15 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char depth[256] = "\0";
        sprintf(depth, "%d", Depth);
        wchar_t wdepth[256];
        mbstowcs(wdepth, depth, 256);
        Draw_Text(ScoreBoardHwnd, false, 8, 384 + 96, Color(255, 255, 255, 255), L"Monaco", 16, wdepth);
        break;
    }
    case 9: {
        for (int i = 0; i < 8; i++) {
            Draw_Image(ScoreBoardHwnd, i * IMAGE_SIZE, 17 * IMAGE_SIZE, L"Blocks/dirt.png");
        }

        for (int i = 0; i < Health / 2; i++) {
            Draw_Image(ScoreBoardHwnd, 38 + 18 * i, 17 * IMAGE_SIZE, L"Health.png");
        }
        if (Health % 2)
            Draw_Image(ScoreBoardHwnd, 38 + ((Health / 2)) * 18, 17 * IMAGE_SIZE, L"Half_Health.png");
        break;
    }
    case 10: {
        for (int i = 0; i < 8; i++) {
            Draw_Image(ScoreBoardHwnd, i * IMAGE_SIZE, 18 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        for (int i = 0; i < Oxygen; i++) {
            Draw_Image(ScoreBoardHwnd, 38 + 18 * i, 18 * IMAGE_SIZE, L"Oxygen.png");
        }
    }
    case 11: {
        for (int i = 0; i < 8; i++) {
            Draw_Image(ScoreBoardHwnd, i * IMAGE_SIZE, 21 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        if (Fortune_Times) {
            char ench[256] = "\0";
            sprintf(ench, "Fortune %d %d", Fortune, Fortune_Times);
            wchar_t wench[256];
            mbstowcs(wench, ench, 256);
            Draw_Text(ScoreBoardHwnd, false, 8, 672, Color(255, 255, 255, 255), L"Monaco", 16, wench);
        }

        break;
    }
    }

    if (Type < 8) {
        for (int i = 0; i < 8; i++) {
            Draw_Image(ScoreBoardHwnd, i * IMAGE_SIZE, 12 * IMAGE_SIZE, L"Blocks/dirt.png");
        }
        char ac[256] = "\0";
        if (Total_Steps != 0)
            Actrual_Score = (Theoretical_Score * Max_Steps) / (Total_Steps * 5);
        sprintf(ac, "%.2lf", Actrual_Score);
        wchar_t wac[256];
        mbstowcs(wac, ac, 256);
        Draw_Text(ScoreBoardHwnd, false, 8, 256 + 128, Color(255, 255, 255, 255), L"Monaco", 16, wac);
    }

}

void ScoreBoard_Painting() {
    Draw_Text(ScoreBoardHwnd, true, 16, 32, Color(255, 255, 255, 255), L"微软雅黑", 16, Name);
    Refresh_ScoreBoard(-1);

    Draw_Text(ScoreBoardHwnd, false, 64 - 8, 352, Color(255, 255, 255, 255), L"Monaco", 16, L"Total Score");
    Draw_Text(ScoreBoardHwnd, false, 64 - 10, 448, Color(255, 255, 255, 255), L"Monaco", 16, L"Current Depth");
    
    Draw_Text(ScoreBoardHwnd, false, 64 - 10, 640, Color(255, 255, 255, 255), L"Monaco", 16, L"Enchantment");
    //for (int i = 0; i < 160; i += 16) {
    //    Draw_Image(ScoreBoardHwnd, 8 + i, 640, L"Health.png");
    //}   
}

void Save_Score() {
    // 获取当前时间的时间戳
    std::time_t currentTime = std::time(nullptr);

    // 将时间戳转换为本地时间结构 tm
    std::tm* localTime = std::localtime(&currentTime);

    // 使用 stringstream 格式化时间为字符串
    std::wstringstream wss;
    wss << std::put_time(localTime, L"%Y-%m-%d %H:%M:%S");

    // 将格式化后的字符串转换为 std::wstring
    std::wstring wstr = wss.str();

    std::wofstream outFile("log.txt", std::ios::app);

    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        std::wcerr << L"Error opening file!" << std::endl;
        return;
    }

    // 将宽字符数组写入文件
    outFile << Name << " " << Actrual_Score << L" [" << wstr << L"]" << endl;

    // 关闭文件
    outFile.close();
}

void Ranking_Calculate() {
    std::wifstream inFile("Ranking.txt"); // 打开文件输入流
    list<std::pair<wstring, int>> Ranking_List;

    // 检查文件是否成功打开
    if (inFile.is_open()) {
        std::wstring data;

        while (std::getline(inFile, data)) { // 从文件逐行读取数据
            std::wistringstream iss(data);
            // 读取所需的值
            wstring name;
            int value;
            iss >> name >> value;
            if (name != Name)
                Ranking_List.push_back(std::pair<wstring, int>(name, value));
            else if (value > Actrual_Score) Actrual_Score = value;
        }

        bool LessThanBefore = true;
        bool successfully_Inserted = false;
        for (auto it = Ranking_List.begin(); it != Ranking_List.end(); it++) {
            if (it->second >= Actrual_Score) {
                LessThanBefore = true;
            }
            else if (LessThanBefore and it->second < Actrual_Score) {
                Ranking_List.insert(it, std::pair<wstring, int>(Name, Actrual_Score));
                LessThanBefore = false;
                successfully_Inserted = true;
            }
        }
        if (!successfully_Inserted)
            Ranking_List.push_back(std::pair<wstring, int>(Name, Actrual_Score));
        inFile.close();
    }
    else {
        Ranking_List.push_back(std::pair<wstring, int>(Name, Actrual_Score));
    }

    std::wofstream outFile("Ranking.txt");

    for (const auto& element : Ranking_List) {
        outFile << element.first << " " << element.second << endl;
    }

    // 关闭文件
    outFile.close();
}

void Burn() {
    bool Have_Lava = false;
    for (const auto& element : SnakePlace) {
        if (element.x + 1 < RELATIVE_WIDTH)
            if (Is_Lava(RIGHT(element)))
                Have_Lava = true;
        if (element.x - 1 >= 0)
            if (Is_Lava(LEFT(element)))
                Have_Lava = true;
        if (element.y + 1 < RELATIVE_HEIGHT)
            if (Is_Lava(DOWN(element)))
                Have_Lava = true;
        if (element.y - 1 >= 0)
            if (Is_Lava(UP(element)))
                Have_Lava = true;
    }
    if (Have_Lava) {
        Health -= 4;
        Refresh_ScoreBoard(9);
    }
}

void Music_Player() {
    Stop_Media();
    KillTimer(NULL, MusicPlayer);
    if (Music == 0) {
        Play_Media(MediaList[Music_Order].Media, NULL, SND_ASYNC | SND_FILENAME);
        MusicPlayer = SetTimer(GameHwnd, 4, MediaList[Music_Order].Length, (TIMERPROC)Music_Player);
        Music_Order++;
        if (Music_Order == 6) Music_Order = 0;
    } else if (WITHIN_CLOSED_INTERVAL(Music, 1, 6)) {
        Play_Media(MediaList[Music - 1].Media, NULL, SND_ASYNC | SND_FILENAME);
        MusicPlayer = SetTimer(GameHwnd, 4, MediaList[Music - 1].Length, (TIMERPROC)Music_Player);
    }
    /*
    fdwSound: 
        SND_SYNC: 等待声音播放完毕后才返回。
        SND_ASYNC: 在后台播放声音，不会阻塞当前线程。
        SND_ALIAS: 指定第一个参数为系统定义的声音别名。
        SND_FILENAME: 指定第一个参数为文件名。
    */
}

void Diving() {
    if (Oxygen > 0) { 
        Oxygen--; 
        Refresh_ScoreBoard(10);
    }
    else { 
        Health--; 
        Refresh_ScoreBoard(9);
    }
}

void Damage() {
    if (Forward != '\0')
        Burn();
    if (In_The_Water)
        Diving();
    if (Health <= 0) GameOver();
}

void ClearAll() {
    for (int i = 0; i < RELATIVE_WIDTH; i++) {
        for (int j = 0; j < RELATIVE_HEIGHT; j++) {
            if (RetroSnake_Hashes[i][j] == 0) {
                RetroSnake_Hashes[i][j] = -1;
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
            }
        }
    }
}

void Solidification(place location) {
    if (!Is_Lava(location)) return;
    bool Have_Water = false;
    if (location.x + 1 < RELATIVE_WIDTH)
        if (Is_Water(RIGHT(location)))
            Have_Water = true;
    if (location.x - 1 >= 0)
        if (Is_Water(LEFT(location)))
            Have_Water = true;
    if (location.y + 1 < RELATIVE_HEIGHT)
        if (Is_Water(DOWN(location)))
            Have_Water = true;
    if (location.y - 1 >= 0)
        if (Is_Water(UP(location)))
            Have_Water = true;
    int level = (RetroSnake_Hashes[location.x][location.y] - 10000) / 100;
    if (Have_Water) {
        if (level) {
            Draw_Image(GameHwnd, location.x * IMAGE_SIZE, location.y * IMAGE_SIZE, L"Blocks/cobblestone.png");
            RetroSnake_Hashes[location.x][location.y] = 0;
            Liq[level].erase(location);
            
        }
        else{
            Draw_Image(GameHwnd, location.x * IMAGE_SIZE, location.y * IMAGE_SIZE, L"Blocks/obsidian.png");
            RetroSnake_Hashes[location.x][location.y] = 1000;
            Liq[level].erase(location);
            
        }
    }
}

void Random_Fortune(int level) {
    if (Fortune < level) {
        Fortune = level;
        Fortune_Times = 10;
    }
    else if (Fortune == level) {
        Fortune_Times += 10;
    }
    Refresh_ScoreBoard(11);
}

void Random_Shorten(int min, int max) {
    int n = Random(min, max);
    Shorten(n);
}

void Random_Lengthen(int min, int max) {
    int n = Random(min, max);
    Owe_Length += n;
}

void Random_DH(int min, int max, int mult) {
    int n = Random(min, max);
    Health += n * mult;
    Refresh_ScoreBoard(9);
}

void Event() {
    int roll = Random(0, 1000);

    if (Depth < 10) return;
    else if (Depth < 30) {
        if (roll < 200) {
            Random_Fortune(1);
        }
        else if (roll < 400) {
            Random_Shorten(1, 4);
        }
        else if (roll < 600) {
            Random_Lengthen(1, 4);
        }
        else if (roll < 800) {
            Random_DH(1, 2, -1);
        } 
        else if (roll < 1000) {
            Random_DH(1, 2, 1);
        }
    }
    else if (Depth < 60) {
        if (roll < 200) {
            Random_Fortune(1);
        } 
        else if (roll < 400) {
            Random_Fortune(2);
        }
        else if (roll < 550) {
            Random_Shorten(2, 4);
        }
        else if (roll < 700) {
            Random_Lengthen(2, 4);
        }
        else if (roll < 850) {
            Random_DH(1, 4, -1);
        }
        else if (roll < 1000) {
            Random_DH(1, 4, 1);
        }
    }
    else if (Depth < 100) {
        if (roll < 150) {
            Random_Fortune(1);
        }
        else if (roll < 300) {
            Random_Fortune(2);
        }
        else if (roll < 450) {
            Random_Fortune(3);
        }
        else if (roll < 550) {
            Random_Shorten(2, 8);
        }
        else if (roll < 700) {
            Random_Lengthen(2, 8);
        }
        else if (roll < 850) {
            Random_DH(2, 4, -1);
        }
        else if (roll < 1000) {
            Random_DH(2, 4, 1);
        }
    }
    else {
        if (roll < 200) {
            Random_Fortune(2);
        }
        else if (roll < 400) {
            Random_Fortune(3);
        }
        else if (roll < 550) {
            Random_Shorten(4, 8);
        }
        else if (roll < 700) {
            Random_Lengthen(4, 8);
        }
        else if (roll < 850) {
            Random_DH(2, 6, -1);
        }
        else if (roll < 1000) {
            Random_DH(2, 6, 1);
        }
    }

    /*
    10 级以前 无事件
    10 ~ 30
        时运 I 20%
        缩短 (1 ~ 4) 20%
        延长(1 ~ 4) 20%
        回复生命 (1 ~ 2) 20%
        损失生命 (1 ~ 2) 20%
    30 ~ 60
        时运 I 20%
        时运 II 20%
        缩短 (2 ~ 4) 15%
        延长(2 ~ 4) 15%
        回复生命 (1 ~ 4) 15%
        损失生命 (1 ~ 4) 15%
    60 ~ 100
        时运 I 15%
        时运 II 15%
        时运 III 10%
        缩短 (2 ~ 8) 15%
        延长(2 ~ 8) 15%
        回复生命 (2 ~ 4) 15%
        损失生命 (2 ~ 4) 15%
    100 or bigger
        时运 II 20%
        时运 III 20%
        缩短 (4 ~ 8) 15%
        延长(4 ~ 8) 15%
        回复生命 (2 ~ 6) 15%
        损失生命 (2 ~ 6) 15%
    */
}

VOID CALLBACK Liquid_Flowing_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    Water_Spreading();
    Blocks_Drying();
    Water_Vertical_Flow();
}

VOID CALLBACK Auto_Moving(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (BannedAutoMoving == false)
        if (Forward != '\0')
            KeyBoard_Input(Forward, true);
}

VOID CALLBACK Liquid_Refresh_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    for (int i = 0; i < Liq.size(); i++) {
        for (auto it = Liq[i].begin(); it != Liq[i].end(); it++) {
            if (RetroSnake_Hashes[it->first.x][it->first.y] >= 10000 and RetroSnake_Hashes[it->first.x][it->first.y] < 10100) {
                int chance = Random(0, 100);
                if (chance > 50) {
                    RetroSnake_Hashes[it->first.x][it->first.y]++;
                    if (RetroSnake_Hashes[it->first.x][it->first.y] == 10038) RetroSnake_Hashes[it->first.x][it->first.y] = 10000;
                    if (RetroSnake_Hashes[it->first.x][it->first.y] < 10019)
                        Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, LavaList[RetroSnake_Hashes[it->first.x][it->first.y] - 10000]);
                    else
                        Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, LavaList[10037 - RetroSnake_Hashes[it->first.x][it->first.y]]);
                }
            }
            else if (RetroSnake_Hashes[it->first.x][it->first.y] >= 20000 and RetroSnake_Hashes[it->first.x][it->first.y] < 20100) {
                RetroSnake_Hashes[it->first.x][it->first.y]++;
                if (RetroSnake_Hashes[it->first.x][it->first.y] == 20064) RetroSnake_Hashes[it->first.x][it->first.y] = 20000;
                if (RetroSnake_Hashes[it->first.x][it->first.y] < 20032)
                    Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, WaterList[RetroSnake_Hashes[it->first.x][it->first.y] - 20000]);
                else
                    Draw_Image(GameHwnd, it->first.x * IMAGE_SIZE, it->first.y * IMAGE_SIZE, WaterList[20064 - RetroSnake_Hashes[it->first.x][it->first.y]]);
            }
            if (RetroSnake_Hashes[it->first.x][it->first.y] >= 21000) {
            }
        }
    }
}

LRESULT CALLBACK NewGameProc(HWND GamehWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(GamehWnd);
        return 0;
    case WM_DESTROY:
        if (GameHwnd == GamehWnd) {
            GameHwnd = NULL;
        }
        ShowWindow(hWnd, SW_SHOW);
        RetroSnake_Destruction();
        CleanupD2D();
        break;
    case WM_KEYDOWN: {
        char FLAG = '\0';
        bool Invaild = false;
        if (Running)
            switch (wParam) {
            case 0x57: {
                FLAG = 'W';
                if (Forward == 'S') Invaild = true;
                break;
            }
            case 0x41: {
                FLAG = 'A';
                if (Forward == 'D') Invaild = true;
                break;
            }
            case 0x53: {
                FLAG = 'S';
                if (Forward == 'W') Invaild = true;
                break;
            }
            case 0x44: {
                FLAG = 'D';
                if (Forward == 'A') Invaild = true;
                break;
            }
            case VK_SPACE: {
                if (Curr_Score_In_This_Layer * 2 >= Max_Score_In_This_Layer)
                    Level_Up();
            }
        }
        if (!Invaild) {
            Forward = FLAG;
            int value = KeyBoard_Input(FLAG, false);
            if (value) {
                KillTimer(NULL, AutoMovingTimer);
                AutoMovingTimer = SetTimer(NULL, 1, Auto_Moving_Cooldown[Speed], (TIMERPROC)Auto_Moving);
            }

            break;
        }
    }
    default:
        return DefWindowProc(GamehWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK MainMenuProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
    case WM_MOUSEMOVE: {
        POINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);

        // buttonRect = { 100, 100, 300, 140 };

        for (int i = 0; i < 4; i++) {
            if (Point_In_Rect(ButtonRect[i], point))
            {
                if (!isMouseOverButton[i])
                {
                    isMouseOverButton[i] = true;
                    Draw_Image(Button[i], 0, 0, ButtonList[i].Actived);
                }
            }
            else
            {
                if (isMouseOverButton[i])
                {
                    isMouseOverButton[i] = false;
                    Draw_Image(Button[i], 0, 0, ButtonList[i].Non_Actived);
                }
            }
        }


        return 0;
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    case WM_DRAWITEM: {
        for (int i = 0; i < 4; i++) {
            Draw_Image(Button[i], 0, 0, ButtonList[i].Non_Actived);
        }
        return TRUE;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 0 && HIWORD(wParam) == BN_CLICKED) {
            //StartNewGameWindow();
            Enter_Name();
        }
        else if (LOWORD(wParam) == 1 && HIWORD(wParam) == BN_CLICKED) {
            LPCWSTR filePath = L"Ranking.txt";

            // 使用 ShellExecute 打开文件
            HINSTANCE result = ShellExecute(
                NULL,         // 无父窗口
                L"open",       // 操作
                L"notepad.exe",// 要启动的程序
                filePath,     // 程序参数（要打开的文件路径）
                NULL,         // 默认目录
                SW_SHOWNORMAL // 显示窗口方式
            );

            // 检查结果
            if ((int)result <= 32) {
                MessageBox(NULL, L"Failed to open file with Notepad", L"Error", MB_ICONERROR);
            }
        }
        else if (LOWORD(wParam) == 2 && HIWORD(wParam) == BN_CLICKED) {
            OpenSetting();
            ShowWindow(SettingsHwnd, SW_SHOW);
        }
        else if (LOWORD(wParam) == 3 && HIWORD(wParam) == BN_CLICKED) {
            PostQuitMessage(0);
            break;
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(255, 255, 255)));
#ifdef USING_BACKPACK
        Image bp(L"Global/bp.png");
        Graphics graphics(hdc);
        graphics.DrawImage(&bp, 0, 0, 1280, 896);
        Image title(L"Global/GAMING.png");
        Draw_Image(hWnd, CENTER(title.GetWidth()), 16, L"Global/GAMING.png");
#endif
        EndPaint(hWnd, &ps);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK SettingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(hWnd);  // 只销毁窗口 B
        return 0;

    case WM_DESTROY:
        return 0;
    case WM_MOUSEMOVE: {
        POINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);

        // buttonRect = { 100, 100, 300, 140 };
        if (Point_In_Rect(Settings_Button_Rect[0], point))
        {
            if (!isMouseOverSettingsButton[0]){
                isMouseOverSettingsButton[0] = true;
                Draw_Image(Settings_Button[0], 0, 0, MusicList[Music].Actived);
            }
        }
        else
        {
            if (isMouseOverSettingsButton[0]) {
                isMouseOverSettingsButton[0] = false;
                Draw_Image(Settings_Button[0], 0, 0, MusicList[Music].Non_Actived);
            }
        }

        if (Point_In_Rect(Settings_Button_Rect[1], point))
        {
            if (!isMouseOverSettingsButton[1]) {
                isMouseOverSettingsButton[1] = true;
                Draw_Image(Settings_Button[1], 0, 0, SpeedList[Speed].Actived);
            }
        }
        else
        {
            if (isMouseOverSettingsButton[1]) {
                isMouseOverSettingsButton[1] = false;
                Draw_Image(Settings_Button[1], 0, 0, SpeedList[Speed].Non_Actived);
            }
        }

        if (Point_In_Rect(Settings_Button_Rect[2], point))
        {
            if (!isMouseOverSettingsButton[2]) {
                isMouseOverSettingsButton[2] = true;
                Draw_Image(Settings_Button[2], 0, 0, ModeList[Mode].Actived);
            }
        }
        else
        {
            if (isMouseOverSettingsButton[2]) {
                isMouseOverSettingsButton[2] = false;
                Draw_Image(Settings_Button[2], 0, 0, ModeList[Mode].Non_Actived);
            }
        }

        if (Point_In_Rect(Settings_Button_Rect[3], point))
        {
            if (!isMouseOverSettingsButton[3]) {
                isMouseOverSettingsButton[3] = true;
                Draw_Image(Settings_Button[3], 0, 0, L"Buttons/Settings/Save_Actived.png");
            }
        }
        else
        {
            if (isMouseOverSettingsButton[3]) {
                isMouseOverSettingsButton[3] = false;
                Draw_Image(Settings_Button[3], 0, 0, L"Buttons/Settings/Save_Non-Actived.png");
            }
        }
        
        return 0;
        break;
    }
    case WM_DRAWITEM: {
        if (Settings_Init == false) {
            for (int i = 0; i < 4; i++) {
                Draw_Image(Settings_Button[i], 0, 0, Setting_Buttons_List[i]);
            }
            Settings_Init = true;
        }

        return TRUE;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 4 && HIWORD(wParam) == BN_CLICKED) {
            Music++;
            if (Music == 8) Music = 0;
            Draw_Image(Settings_Button[0], 0, 0, MusicList[Music].Actived);

            Music_Player();
        }
        else if (LOWORD(wParam) == 5 && HIWORD(wParam) == BN_CLICKED) {
            Speed++;
            if (Speed == 3) Speed = 0;
            Draw_Image(Settings_Button[1], 0, 0, SpeedList[Speed].Actived);
        }
        else if (LOWORD(wParam) == 6 && HIWORD(wParam) == BN_CLICKED) {
            Mode++;
            if (Mode == 2) Mode = 0;
            Draw_Image(Settings_Button[2], 0, 0, ModeList[Mode].Actived);
        }
        else if (LOWORD(wParam) == 7 && HIWORD(wParam) == BN_CLICKED) {
            DestroyWindow(hWnd);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(255, 255, 255)));

        for (int i = 0; i < SETTING_WIDTH / IMAGE_SIZE; i++) {
            for (int j = 0; j < SETTING_HEIGHT / IMAGE_SIZE; j++) {
                Draw_Image(hWnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/dirt.png");
            }
        }

        EndPaint(hWnd, &ps);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK NameEnterProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit, hButton;

    switch (message)
    {
    case WM_CREATE:
        // 创建文本框
        hEdit = CreateWindow(TEXT("EDIT"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 10, 200, 24,
            hwnd, (HMENU)0x104, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        // 创建按钮
        hButton = CreateWindow(TEXT("BUTTON"), TEXT("OK"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            60, 50, 110, 25,
            hwnd, (HMENU)0x101, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case 0x101:
            {
                HWND hedit = GetDlgItem(hwnd, 0x104); // 获取文本框控件的句柄
                GetWindowTextW(hedit, Name, 100);  // 获取文本框内容
                if (Name[0] == L'\0') {
                    wcscpy(Name, L"Steve");
                }
                else if (Name[0] == L' ') {
                    int i = 0;
                    while (Name[i++] == L' ');
                    if (Name[i - 1] == L'\0')
                        wcscpy(Name, L"Steve");
                }
                DestroyWindow(hwnd);
                StartNewGameWindow();
                break;
            }
            }
        return 0;

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Gdiplus_Startup_Wrapper(); // Initialize GDI+

    wc.lpfnWndProc = MainMenuProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Forgotten Realms: Whispers Of Hidden Hoard";

    ng.lpfnWndProc = NewGameProc;
    ng.hInstance = hInstance;
    ng.lpszClassName = L"NewGameWindow";

    settings.lpfnWndProc = SettingsProc;
    settings.hInstance = hInstance;
    settings.lpszClassName = L"GameSettings";

    nh.lpfnWndProc = NameEnterProc;
    nh.hInstance = hInstance;
    nh.lpszClassName = L"EnterYourName";

    RegisterClass(&ng);
    RegisterClass(&wc);
    RegisterClass(&settings);
    RegisterClass(&nh);

    int x = (screenWidth - MAP_WIDTH) / 2;
    int y = (screenHeight - MAP_HEIGHT) / 2 - 16;

    hWnd = CreateWindow(wc.lpszClassName, L"Forgotten Realms: Whispers Of Hidden Hoard",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, x, y,
        MAP_WIDTH, MAP_HEIGHT, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error!", MB_ICONERROR | MB_OK);
        return 1;
    }
    Load_Button_Images(hWnd);

    for (int i = 0; i < 4; i++) {
        Button[i] = CreateWindow(TEXT("BUTTON"), TEXT("??????"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            CENTER(400), 320 + i * 60, 400, 40, hWnd, (HMENU)i, hInstance, NULL);
        ButtonRect[i] = { CENTER(400) - 8, 320 + i * 60 - 8, CENTER(400) + 400 + 8, 320 + i * 60 + 40 + 8 };
    }

    ShowWindow(hWnd, nCmdShow);
    Music_Player();
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Unload_Button_Images();
    Gdiplus_Shutdown_Wrapper(); // GDI+ Destruction
    return 0;
}

void Enter_Name() {
    ShowWindow(hWnd, SW_HIDE);
    int x = (screenWidth - 240) / 2;
    int y = (screenHeight - 120) / 2;
    NameHwnd = CreateWindow(nh.lpszClassName, L"What should we call you? ",
        WS_POPUP & WS_THICKFRAME, x, y,
        240, 120, NULL, NULL, nh.hInstance, NULL);

    ShowWindow(NameHwnd, SW_SHOW);
}

void StartNewGameWindow() {
    if (GameHwnd) {
        DestroyWindow(GameHwnd);
        GameHwnd = NULL;
        RetroSnake_Destruction();
    }

    int x = (screenWidth - MAP_WIDTH) / 2;
    int y = (screenHeight - MAP_HEIGHT) / 2 - 16;
    GameHwnd = CreateWindow(ng.lpszClassName, L"Forgotten Realms: Whispers Of Hidden Hoard",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, x, y,
        MAP_WIDTH, MAP_HEIGHT, NULL, NULL, wc.hInstance, NULL);
    ScoreBoardHwnd = CreateWindow(TEXT("BUTTON"), TEXT("??????"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        1024, 0, 272, 896, GameHwnd, NULL, ng.hInstance, NULL);


    ShowWindow(GameHwnd, SW_SHOW);

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(GameHwnd);

    RetroSnake_Initialize(GameHwnd, false, true, 8, 16, 1);
    ScoreBoard_Painting();

    AutoMovingTimer = SetTimer(NULL, 1, Auto_Moving_Cooldown[Speed], (TIMERPROC)Auto_Moving);
    LiquidRefresh = SetTimer(NULL, 2, WATER_REFRESHING_COOLDOWN, (TIMERPROC)Liquid_Refresh_Function);
    LiquidFlowing = SetTimer(NULL, 3, WATER_SPREADING_COOLDOWN, (TIMERPROC)Liquid_Flowing_Function);
    AutoDmg = SetTimer(NULL, 4, DMG_COOLDOWN, (TIMERPROC)Damage);
}

void OpenSetting() {
    int x = (screenWidth - SETTING_WIDTH) / 2;
    int y = (screenHeight - SETTING_HEIGHT) / 2 - 16;
    SettingsHwnd = CreateWindow(settings.lpszClassName, L"Game Settings",
        WS_POPUP & WS_THICKFRAME, x, y,
        SETTING_WIDTH, SETTING_HEIGHT, hWnd, NULL, settings.hInstance, NULL);

    for (int i = 0; i < 4; i++) {
        Settings_Button[i] = CreateWindow(TEXT("BUTTON"), TEXT("???"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            (SETTING_WIDTH - 400) / 2, 120 + i * 60, 400, 40, SettingsHwnd, (HMENU)(i + 4), settings.hInstance, NULL);
        Settings_Button_Rect[i] = { (SETTING_WIDTH - 400) / 2 - 8, 120 + i * 60 - 8, ((SETTING_WIDTH - 400) / 2) + 400 + 8, 120 + i * 60 + 40 + 8 };
        if (!Settings_Button[i]) {
            MessageBox(SettingsHwnd, _T("Failed to create settings button!"), _T("Error"), MB_ICONERROR);
            return;
        }
    }
}
// RetroSnake_Hashes Value:
// Snake: 1
// Obstcale
// iron: score + 3;
// gold: score + 8;
// diamond: score + 16;
// redstone: Length - 2;
// emerald: Length reset to region
// lapis: Length - 4;