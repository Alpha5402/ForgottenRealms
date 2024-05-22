#pragma comment (lib,"Gdiplus.lib")
#include "ForgottenRealms.h"
#include "Config.cpp"
using namespace Gdiplus;
using namespace std;
#define GAME

bool isMouseOverButton[4] = { false, false, false, false };

place head;
HINSTANCE g_hInstance;
WNDCLASS wc;
WNDCLASS ng;
HWND GameHwnd;
HWND hWnd;
HWND Button[4];
RECT ButtonRect[4];
UINT_PTR AutoMovingTimer;
UINT_PTR LiquidRefresh;
UINT_PTR LiquidFlowing;
ULONG_PTR g_gdiplusToken;

COLORREF BackGround_Color;
COLORREF Grid_Color;
COLORREF Snake_Head_Color;
COLORREF Snake_Color;
COLORREF Food_Color;
COLORREF Frame_Color;
COLORREF TextColor;

vector<vector<int>> RetroSnake_Hashes;
vector<vector<int>> Hash_Temp;
vector<place> SnakePlace;
vector<place> ObstaclePlace;
std::queue<place> WaterBlockToHandle;
std::set<place> WaterBlockVisited;
unordered_map<place, int > DryingBlock;
vector<unordered_map<place, int>> Liq(8);

const wchar_t* aw[8] = ORES;
const BUTTON ButtonList[4] = BUTTON_LIST;
const wchar_t* LavaList[19] = LAVA_LIST;
const wchar_t* WaterList[32] = WATER_LIST;
const wchar_t* KidsWaterList[16][7] = KIDS_WATER_LIST;
const wchar_t* KidsLavaList[8][3] = KIDS_LAVA_LIST;
const Media MediaList[6] = MEDIA_LIST;

Image* buttonNormalImage = nullptr;
Image* buttonActiveImage = nullptr;
char Forward = '\0';
bool Use_Grid = false;
bool Spawn_Foods = true;
bool Running;
int Owe_Length = 0;
int Existing_Ores = 0;
int Foods_Amount;
int Current_Foods_Amount;
int Length;
int Level = 0;
int BreakTime;
double Score;
double Multiply;
bool BannedAutoMoving;

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
    HDC hdc = GetDC(hWnd);// ??????????????????
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
    Score = 0;
    Multiply = 1;
    BackGround_Color = WHITE;
    Grid_Color = RGB(200, 200, 200);
    Snake_Head_Color = GREEN;
    Snake_Color = RGB(0, 245, 255);
    Food_Color = RED;
    Frame_Color = BLACK;
    TextColor = BLACK;
    BreakTime = 400;
    Running = 1;
    GameHwnd = hwnd;
    Forward = '\0';
    RetroSnake_Hashes.resize(RELATIVE_WIDTH + 1, vector<int>(RELATIVE_HEIGHT + 1, 0));

    Image title(L"GAMING.png");
    for (int i = 0; i < MAP_WIDTH / IMAGE_SIZE - 8; i++) {
        for (int j = 0; j < MAP_HEIGHT / IMAGE_SIZE; j++) {
            Draw_Image(hwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
        }
    }

    for (int i = MAP_WIDTH / IMAGE_SIZE - 8; i < MAP_WIDTH / IMAGE_SIZE; i++) {
        for (int j = 0; j < MAP_HEIGHT / IMAGE_SIZE; j++) {
            Draw_Image(hwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/dirt.png");
        }
    }

    Random_Draw_Snake();
    //int temp = 1;
    int temp = Random(1, 8);
    for (int i = 0; i < temp; i++) {
        Random_Draw_Obstacle();
    }

    if (Spawn_Foods) {
        Foods_Spawn();
    }

    OutputDebugStringA("[System] Game initialization completed.\n");
};

void RetroSnake_Destruction() {
    SnakePlace.clear();
    RetroSnake_Hashes.clear();
    ObstaclePlace.clear();
    KillTimer(NULL, AutoMovingTimer);
    KillTimer(NULL, LiquidRefresh);
    KillTimer(NULL, LiquidFlowing);
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
    if (RetroSnake_Hashes[loc.x][loc.y] >= 20000) {
        return (RetroSnake_Hashes[loc.x][loc.y] % 20000) / 100;
    }
    return 0;
}
int Is_Water(int Hash_Value) {
    if (Hash_Value >= 20000) {
        return (Hash_Value % 20000) / 100;
    }
    return 0;
}
int Is_Lava(place loc) {
    if (WITHIN_CLOSED_INTERVAL(RetroSnake_Hashes[loc.x][loc.y], 10000, 20000)) {
        return (RetroSnake_Hashes[loc.x][loc.y] % 10000) / 100;
    }
    return 0;
}
int Is_Lava(int Hash_Value) {
    if (WITHIN_CLOSED_INTERVAL(Hash_Value, 10000, 20000)) {
        return (Hash_Value % 10000) / 100;
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
    if (RetroSnake_Hashes[loc.x][loc.y] == 9999) {
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

void Foods_Create(int type, int max) {
    while (1) {
        place food;
        food.x = Random(1, RELATIVE_WIDTH - 2);
        food.y = Random(1, RELATIVE_HEIGHT - 2);
        //int type = value;
        if (RetroSnake_Hashes[food.x][food.y] == 0 && Adjacent_Unit_Amount(food, 0) > 3) {
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
        int T = Random(0, min(Level, max - 1));
        int type = queue[T];

        if (RetroSnake_Hashes[food.x][food.y] == 0 && Adjacent_Unit_Amount(food, 0) > 3) {
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
        Draw_Image(GameHwnd, loc.x * IMAGE_SIZE, loc.y * IMAGE_SIZE, aw[type]);
        RetroSnake_Hashes[loc.x][loc.y] = 10 + type;
    }
}

void Foods_Spawn(int Amount) {
    Current_Foods_Amount = Amount;
    for (int i = 0; i < Foods_Amount; i++) {
        int queue[5] = { 0, 1, 2, 4, 7 };
        Foods_Create(queue, 5);
    }
    if (Level >= 3) {
        int amount = Random(1, 2);
        for (int i = 0; i <= amount; i++) {
            Foods_Create(3, 1);
        }
    }
    if (Level >= 4) {
        int amount = Random(1, 2);
        for (int i = 0; i <= amount; i++) {
            int roll = Random(0, 100);
            if (roll < 50) {
                Foods_Create(5, 1);
            }
        }
    }
    if (Level >= 5) {
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
    if (SnakePlace[1] == LEFT(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_RightHead.png");
    else if (SnakePlace[1] == RIGHT(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_LeftHead.png");
    else if (SnakePlace[1] == UP(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_UpHead.png");
    else if (SnakePlace[1] == DOWN(SnakePlace[0]))
        Draw_Image(hWnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_DownHead.png");
}

void Update_Snake_Rail(HWND hWnd) {
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
        // ???
    }
    else if (Now.y == Before.y and Now.y == Next.y) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftRight.png");
        // ??
    }
    else if (Before == LEFT(Now) and Next == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftDown.png");
    }
    else if (Next == LEFT(Now) and Before == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftDown.png");
    }
    // ????
    else if (Before == LEFT(Now) and Next == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftUp.png");
    }
    else if (Next == LEFT(Now) and Before == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_LeftUp.png");
    }
    // ????
    else if (Before == RIGHT(Now) and Next == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightDown.png");
    }
    else if (Next == RIGHT(Now) and Before == DOWN(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightDown.png");
    }
    // ????
    else if (Before == RIGHT(Now) and Next == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightUp.png");
    }
    else if (Next == RIGHT(Now) and Before == UP(Now)) {
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"Snake_RightUp.png");
    }
    else
        Draw_Image(hWnd, Now.x * IMAGE_SIZE, Now.y * IMAGE_SIZE, L"magma.png");
    // ????
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
    int Type = Random(1, 2);
    Draw_Image(GameHwnd, center.x * IMAGE_SIZE, center.y * IMAGE_SIZE, Type == 1 ? L"Lava/Lava_1.png" : L"Water_Still/Water_Still_1.png");
    int x = center.x;
    int y = center.y;
    int cnt = 0;
    int list[4] = { 1, 2, 3, 4 };
    int Obstacle_Size = Random(1, 6);
    place temp = { x, y };
    temp.flag = 0;

    Liq[0].emplace(std::pair<place, int>(temp, Type));
    RetroSnake_Hashes[x][y] = 10000 * Type;
    ObstaclePlace.push_back(Translate(x, y));

    for (int i = 1; i < Obstacle_Size && cnt <= 2 * Obstacle_Size; i++) {
        while (1) {
            if (cnt >= 4) break;
            int forward = Detect(Translate(x, y), 4, list[cnt]);
            if (forward == 1 && x - 1 >= 0 && RetroSnake_Hashes[x - 1][y] == 0) {
                Draw_Image(GameHwnd, (x - 1) * IMAGE_SIZE, y * IMAGE_SIZE, Type == 1 ? L"Lava/Lava_1.png" : L"Water_Still/Water_Still_1.png");
                cnt = 0;
                ObstaclePlace.push_back(LEFT(Translate(x, y)));
                RetroSnake_Hashes[x - 1][y] = 10000 * Type;
                x -= 1;
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 2 && x + 2 < RELATIVE_WIDTH && RetroSnake_Hashes[x + 1][y] == 0) {
                Draw_Image(GameHwnd, (x + 1) * IMAGE_SIZE, y * IMAGE_SIZE, Type == 1 ? L"Lava/Lava_1.png" : L"Water_Still/Water_Still_1.png");
                cnt = 0;
                ObstaclePlace.push_back(RIGHT(Translate(x, y)));
                RetroSnake_Hashes[x + 1][y] = 10000 * Type;
                x += 1;
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 3 && y - 1 >= 0 && RetroSnake_Hashes[x][y - 1] == 0) {
                Draw_Image(GameHwnd, x * IMAGE_SIZE, (y - 1) * IMAGE_SIZE, Type == 1 ? L"Lava/Lava_1.png" : L"Water_Still/Water_Still_1.png");
                cnt = 0;
                ObstaclePlace.push_back(UP(Translate(x, y)));
                RetroSnake_Hashes[x][y - 1] = 10000 * Type;
                y -= 1;
                Liq[0].emplace(std::pair<place, int>({ x, y }, Type));
                break;
            }
            if (forward == 4 && y + 2 < RELATIVE_HEIGHT && RetroSnake_Hashes[x][y + 1] == 0) {
                Draw_Image(GameHwnd, x * IMAGE_SIZE, (y + 1) * IMAGE_SIZE, Type == 1 ? L"Lava/Lava_1.png" : L"Water_Still/Water_Still_1.png");
                cnt = 0;
                ObstaclePlace.push_back(DOWN(Translate(x, y)));
                RetroSnake_Hashes[x][y + 1] = 10000 * Type;
                y += 1;
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

place Find_Common_Side(place a, place b) {
    place temp;
    if (a.x == b.x + 1) {
        temp.x = a.x;
        temp.y = a.y;
        temp.model = 0;
    }
    else if (a.x + 1 == b.x) {
        temp.x = b.x;
        temp.y = b.y;
        temp.model = 0;
    }
    else if (a.y == b.y + 1) {
        temp.x = a.x;
        temp.y = a.y;
        temp.model = 1;
    }
    else if (a.y + 1 == b.y) {
        temp.x = b.x;
        temp.y = b.y;
        temp.model = 1;
    }
    return temp;
}

void Lengthen(place source, place target, COLORREF GridColor, COLORREF InsideColor) {
    //DrawUnitBlock(target, GridColor, InsideColor, true);
    place commonsize = Find_Common_Side(source, target);
    //InsideColor = RGB(255, 0, 0);
    if (commonsize.model)
        Draw_Line(GameHwnd, { commonsize.x * UNIT_SIZE, commonsize.y * UNIT_SIZE },
            { (commonsize.x + 1) * UNIT_SIZE, commonsize.y * UNIT_SIZE }, InsideColor, 2);
    else
        Draw_Line(GameHwnd, { commonsize.x * UNIT_SIZE, commonsize.y * UNIT_SIZE },
            { commonsize.x * UNIT_SIZE, (commonsize.y + 1) * UNIT_SIZE }, InsideColor, 2);
}

void Shorten(int begin) {
    if (begin >= SnakePlace.size() - 1) { 
        begin = SnakePlace.size() - 1; 
        Draw_Image(GameHwnd, SnakePlace[0].x * IMAGE_SIZE, SnakePlace[0].y * IMAGE_SIZE, L"Snake_Head.png");
    }

    int i = 1;
    auto it = SnakePlace.end() - 1;
    while (i++) {
        //putimage((*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, &bp);
        Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, L"Blocks/Stone_Dark.png");
        RetroSnake_Hashes[(*it).x][(*it).y] = -1;
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
        Eating(type);

        if (Length > 2)
            Check_Snake_To_Update(GameHwnd, SnakePlace[1], SnakePlace[2], SnakePlace[0]);
        Update_Snake_Head(GameHwnd);
        Update_Snake_Rail(GameHwnd);
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
    // lvl 水的高度，从 7 到 0
    // dist 水蔓延的距离 从 0 到 7

    for (int i = Liq.size() - 1; i >= 0; i--) {
        int cnt = 0;
        for (auto it = Liq[i].begin(); it != Liq[i].end() and cnt <= Liq[i].size() - 1; ++it, ++cnt) {
            //if (RetroSnake_Hashes[it->first.x][it->first.y] < 10000) continue;
            place pos = it->first;
            int curr_x = pos.x;
            int curr_y = pos.y;
            int distance = i;
            int Type = it->second;
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
                        if (Type == 1) {
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
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, KidsLavaList[9][i]);
                            }
                        }
                        else if (Type == 2) {
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
                                Draw_Image(GameHwnd, new_x * IMAGE_SIZE, new_y * IMAGE_SIZE, KidsWaterList[0][i]);
                            }
                        }
                    }
                        
            }
        }
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

void KeyBoard_Input(int userKey) {
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
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10000) {
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] < 1000)
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y - 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] << endl;
                    GameOver();
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] <= 0)
                    Moving_Upward(false);
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10000) {
                    Moving_Upward(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] < 1000)
                    Moving_Upward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1]);
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y - 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y - 1] << endl;
                    GameOver();
                }
            }

        }
        else GameOver();
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
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10000) {
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] < 1000)
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y + 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] << endl;
                    GameOver();
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] <= 0)
                    Moving_Downward(false);
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10000) {
                    //LiquidClearAccordingCoordinate(DOWN(SnakePlace[0]));
                    Moving_Downward(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] >= 10 and RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] < 1000)
                    Moving_Downward(true, RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1]);
                else {
                    cout << "You dead in " << SnakePlace[0].x << SnakePlace[0].y + 1 << " for " << RetroSnake_Hashes[SnakePlace[0].x][SnakePlace[0].y + 1] << endl;
                    GameOver();
                }
            }
        }  
        else GameOver();
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
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10000) {
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] < 1000)
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                else {
                    cout << "You dead in " << SnakePlace[0].x - 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] << endl;
                    GameOver();
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] <= 0)
                    Moving_Left(false);
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10000) {
                    Moving_Left(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] < 1000)
                    Moving_Left(true, RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y]);
                else {
                    cout << "You dead in " << SnakePlace[0].x - 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x - 1][SnakePlace[0].y] << endl;
                    GameOver();
                }
            }

        }
        else GameOver();
        break;
    }
    case 'd':
    case 'D':
    case -77: {
        if (SnakePlace[0].x < RELATIVE_WIDTH - 2) {
            if (Owe_Length) {
                if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] <= 0) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    Owe_Length--;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10000) {
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                    Owe_Length--;
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] < 1000)
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                else {
                    cout << "You dead in " << SnakePlace[0].x + 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] << endl;
                    GameOver();
                }
            }
            else {
                if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] <= 0)
                    Moving_Right(false);
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10000) {
                    Moving_Right(false);
                    Water_Clear({ SnakePlace[0].x, SnakePlace[0].y });
                    Check_Drying_Blocks();
                }
                else if (RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] >= 10 and RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] < 1000)
                    Moving_Right(true, RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y]);
                else {
                    cout << "You dead in " << SnakePlace[0].x + 1 << SnakePlace[0].y << " for " << RetroSnake_Hashes[SnakePlace[0].x + 1][SnakePlace[0].y] << endl;
                    GameOver();
                }
            }

        }
        else GameOver();
        break;
    }

    }
}

void Auto_Moving() {
    while (Running) {
        KeyBoard_Input(Forward);
        //if (Running)
        //    Message_Renovate();
        Sleep(BreakTime);
    }
}

void GameOver() {
    //cout << "################################" << endl;
    //cout << "#                              #" << endl;
    //cout << "#           GAMEOVER           #" << endl;
    //cout << "#                              #" << endl;
    //cout << "################################" << endl;

    //cleardevice();
    //for (int i = 0; i < (MAP_SIZE * 3 / 2) / IMAGE_SIZE; i++) {
    //    for (int j = 0; j < MAP_SIZE / IMAGE_SIZE; j++) {
    //        putimage(i * IMAGE_SIZE, j * IMAGE_SIZE, &sb);
    //    }
    //}

    //LOGFONT f;
    //gettextstyle(&f);
    //f.lfHeight = 96;
    //_tcscpy(f.lfFaceName, _T("Consolas"));
    //f.lfQuality = ANTIALIASED_QUALITY;

    //settextstyle(&f);
    //setbkmode(TRANSPARENT);
    ////outtextxy(MAP_SIZE * 3 / 4, MAP_SIZE / 2, );
    ////RECT r = { 0, 0, MAP_SIZE * 3 / 2, MAP_SIZE};
    //char msg[] = "GAME OVER";
    //int w = textwidth(_T(msg));
    //int h = textheight(_T(msg));
    //int x = (MAP_SIZE * 3 / 2) / 2 - 1.25 * w / 2;
    //int y = (MAP_SIZE) / 2 - h / 2;
    ////drawtext(_T("GAME OVER"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    //outtextxy(x, y, msg);
    //f.lfHeight = 32;
    //settextstyle(&f);
    //outtextxy(x, y + 96, "Press any key to continue...");
    //char str_score[128];
    //sprintf(str_score, "Your final score: %.2lf", Score);
    //outtextxy(x + 256, y + 192, str_score);

    //Running = false;
    ////exit(0);
}

void RetroSnake_Hashes_Info() {
    char buffer[256];
    int x, y;
    OutputDebugStringA("[System] Enter the location to info\n");
}

void Eating(int type) {
    //strcpy(str_latest, "Mining!");
    switch (type) {
        case 10:
            Current_Foods_Amount--;
            Score += 1;
            break;
        case 11: {
            Current_Foods_Amount--;
            Score += 2;
            break;
        }
        case 12: {
            Current_Foods_Amount--;
            Score += 3;
            break;
        }
        case 13: {
            Shorten(3);
            break;
        }
        case 14: {
            Current_Foods_Amount--;
            Score += 5;
            break;
        }
        case 15: {
            Shorten(5);
            break;
        }
        case 16: {
            Shorten(SnakePlace.size() - 3);
            break;
        }
        case 17: {
            Current_Foods_Amount--;
            Score += 8;
            break;
        }
    }


    if (!Current_Foods_Amount)
        Level_Up();

    //Message_Renovate();
}

void Level_Up() {
    Level++;
    BreakTime *= 0.8;
    Multiply += 0.5;
    Clear_Windows();
    Owe_Length = Length - 1;
    Shorten(SnakePlace.size() - 1);
    //Length = 1;
    int temp = Random(1, 4);
    for (int i = 0; i < temp; i++) {
        Random_Draw_Obstacle();
    }

    Foods_Spawn(Foods_Amount - Existing_Ores);
    cout << "################################" << endl;
    cout << "#                              #" << endl;
    cout << "#           LEVEL UP           #" << endl;
    cout << "#                              #" << endl;
    cout << "################################" << endl;
    cout << "[System] Now the LEVEL is " << Level << endl;
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

void Inherit(queue<std::pair<place, int>>& To_Handle) {
    for (int i = 0; i < RELATIVE_WIDTH; i++) {
        for (int j = 0; j < RELATIVE_HEIGHT; j++) {
            if (Is_Empty({ i, j }) or Is_Stone({ i, j })) {
                Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                RetroSnake_Hashes[i][j] = 0;
            }
            else if (Is_Ores({ i, j })) {
                int roll = Random(0, 100);
                if (roll <= 20) {
                    To_Handle.push(std::pair<place, int>({ i, j }, RetroSnake_Hashes[i][j]));
                    Existing_Ores++;
                }
                else {
                    Draw_Image(GameHwnd, i * IMAGE_SIZE, j * IMAGE_SIZE, L"Blocks/stone.png");
                    RetroSnake_Hashes[i][j] = 0;
                }
            }
            else if (Is_Liquid({ i, j }) or Is_Gravel({i, j})) {
                To_Handle.push(std::pair<place, int>({ i, j }, RetroSnake_Hashes[i][j]));
            }
        }
    }
}

void Draw_Again(queue<std::pair<place, int>>& To_Handle) {
    while (!To_Handle.empty()) {
        place loc = To_Handle.front().first;
        int type = To_Handle.front().second;

        if (Is_Water(type)) {
            RetroSnake_Hashes[loc.x][loc.y] = 2000;
            Liq[0].insert(std::pair<place, int>(loc, 2));
        }
        else if (Is_Lava(type)) {
            RetroSnake_Hashes[loc.x][loc.y] = 1000;
            Liq[0].insert(std::pair<place, int>(loc, 2));
        }
        else if (Is_Gravel(type)) {
            RetroSnake_Hashes[loc.x][loc.y] = 300;
        }
        else if (Is_Ores(type)) {

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

// GDI+ Initialization
void Gdiplus_Startup_Wrapper() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
}

void Gdiplus_Shutdown_Wrapper() {
    Gdiplus::GdiplusShutdown(g_gdiplusToken);
}

VOID CALLBACK Liquid_Flowing_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    Water_Spreading();
    Blocks_Drying();
}

VOID CALLBACK Auto_Moving(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (BannedAutoMoving == false)
        if (Forward != '\0')
            KeyBoard_Input(Forward);
}

VOID CALLBACK Liquid_Refresh_Function(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    for (auto it = ObstaclePlace.begin(); it != ObstaclePlace.end(); it++) {
        if (RetroSnake_Hashes[it->x][it->y] >= 10000 and RetroSnake_Hashes[it->x][it->y] < 10100) {
            int chance = Random(0, 100);
            if (chance > 50) {
                RetroSnake_Hashes[it->x][it->y]++;
                if (RetroSnake_Hashes[it->x][it->y] == 10038) RetroSnake_Hashes[it->x][it->y] = 10000;
                if (RetroSnake_Hashes[it->x][it->y] < 10019)
                    Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, LavaList[RetroSnake_Hashes[it->x][it->y] - 10000]);
                else
                    Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, LavaList[10037 - RetroSnake_Hashes[it->x][it->y]]);
            }
        }
        else if (RetroSnake_Hashes[it->x][it->y] >= 20000 and RetroSnake_Hashes[it->x][it->y] < 20100) {
            RetroSnake_Hashes[it->x][it->y]++;
            if (RetroSnake_Hashes[it->x][it->y] == 20064) RetroSnake_Hashes[it->x][it->y] = 20000;
            if (RetroSnake_Hashes[it->x][it->y] < 20032)
                Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, WaterList[RetroSnake_Hashes[it->x][it->y] - 20000]);
            else
                Draw_Image(GameHwnd, (*it).x * IMAGE_SIZE, (*it).y * IMAGE_SIZE, WaterList[20064 - RetroSnake_Hashes[it->x][it->y]]);
        }
        if (RetroSnake_Hashes[it->x][it->y] >= 21000) {
            //if (RetroSnake_Hashes[it->x][it->y] % 10000 <= 900) {
            //    RetroSnake_Hashes[it->x][it->y] += 100;
            //    int SpreadDistance = (RetroSnake_Hashes[it->x][it->y] % 10000) / 100;
            //    DrawImage(GameHwnd, it->x * IMAGE_SIZE, it->y * IMAGE_SIZE, KidsWaterList[9][SpreadDistance]);
            //}
            //else {
                //RetroSnake_Hashes[it->x][it->y] = -1;
                //DrawImage(GameHwnd, it->x * IMAGE_SIZE, it->y * IMAGE_SIZE, L"redstone_block.png");
        }
    }
    // ???????????????????е????
    // hwnd ??????????Щ??????????ò???
    // uMsg ????WM_TIMER???
    // idEvent ?????????????ID
    // dwTime ?????????????
}

LRESULT CALLBACK NewGameProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
        //if (MessageBox(hWnd, L"Really quit?", L"My application", MB_OKCANCEL) == IDOK)
            DestroyWindow(hWnd);
        return 0;
    case WM_DESTROY:
        if (GameHwnd == hWnd) {
            GameHwnd = NULL;
        }
        RetroSnake_Destruction();
        break;
    case WM_KEYDOWN: {
        char FLAG = '\0';
        bool Invaild = false;
         
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
            Level_Up();
        }
        }
        if (!Invaild) {
            Forward = FLAG;
            KeyBoard_Input(FLAG);
            KillTimer(NULL, AutoMovingTimer);

            AutoMovingTimer = SetTimer(NULL, 1, AUTO_MOVING_COOLDOWN, (TIMERPROC)Auto_Moving);
            break;
        }


    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK MainMenuProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        //case WM_KEYDOWN:{
    case WM_MOUSEMOVE: {
        POINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);
        // ??? ????

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
            StartNewGameWindow();
        }
        else if (LOWORD(wParam) == 1 && HIWORD(wParam) == BN_CLICKED) {
            MessageBox(hWnd, TEXT("??? 2 ????????"), TEXT("???"), MB_OK);
        }
        else if (LOWORD(wParam) == 2 && HIWORD(wParam) == BN_CLICKED) {
            MessageBox(hWnd, TEXT("??? 3 ????????"), TEXT("???"), MB_OK);
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Gdiplus_Startup_Wrapper(); // ?????GDI+


    wc.lpfnWndProc = MainMenuProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Forgotten Realms: Whispers Of Hidden Hoard";

    ng.lpfnWndProc = NewGameProc;
    ng.hInstance = hInstance;
    ng.lpszClassName = L"NewGameWindow";
    if (!RegisterClass(&ng)) {
        MessageBox(NULL, L"??????????????????", L"????", MB_ICONERROR | MB_OK);
        return 1;
    }

    RegisterClass(&wc);

    hWnd = CreateWindow(wc.lpszClassName, L"Forgotten Realms: Whispers Of Hidden Hoard",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT,
        MAP_WIDTH, MAP_HEIGHT, NULL, NULL, hInstance, NULL);

    //Play_Media(MediaList[4].Media);
    //PlaySound(TEXT("Sound/Danny.wav"), NULL, SND_FILENAME);

    if (!hWnd) {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error!", MB_ICONERROR | MB_OK);
        return 1;
    }
    Load_Button_Images(hWnd);

    for (int i = 0; i < 4; i++) {
        Button[i] = CreateWindow(TEXT("BUTTON"), TEXT("??????"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            CENTER(400), 320 + i * 60, 400, 40, hWnd, (HMENU)i, hInstance, NULL);
        ButtonRect[i] = { CENTER(400) - 8, 320 - 8 + i * 60, CENTER(400) + 400 + 8, 320 + i * 60 + 40 + 8 };
    }

    ShowWindow(hWnd, nCmdShow);
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

void StartNewGameWindow() {
    if (GameHwnd) {
        DestroyWindow(GameHwnd);
        GameHwnd = NULL;
        RetroSnake_Destruction();
    }
    GameHwnd = CreateWindow(ng.lpszClassName, L"Forgotten Realms: Whispers Of Hidden Hoard",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT,
        MAP_WIDTH, MAP_HEIGHT, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(GameHwnd, SW_SHOW);
    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(GameHwnd);

    RetroSnake_Initialize(GameHwnd, false, true, 8, 16, 1);
    AutoMovingTimer = SetTimer(NULL, 1, AUTO_MOVING_COOLDOWN, (TIMERPROC)Auto_Moving);
    LiquidRefresh = SetTimer(NULL, 1, WATER_REFRESHING_COOLDOWN, (TIMERPROC)Liquid_Refresh_Function);
    LiquidFlowing = SetTimer(NULL, 1, WATER_SPREADING_COOLDOWN, (TIMERPROC)Liquid_Flowing_Function);
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