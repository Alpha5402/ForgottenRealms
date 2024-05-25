#pragma once
#ifndef UNICODE
#define UNICODE
#endif 
#include <bits/stdc++.h>
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <objidl.h>
#include <graphics.h>
#include <conio.h>
#include <random>
#include <wchar.h>
#include <thread>
#include <mutex>
#include <commctrl.h>
#include <unordered_map>
#include <mmsystem.h>
// Direct2D Æô¶¯£¡
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

struct place {
    int x;
    int y;
    int model;
    int type;
    int flag;
    bool operator ==(const place& other) const {
        return x == other.x && y == other.y;
    }
    bool operator != (const place& a) {
        if (!(*this == a))
            return true;
        return false;
    }
};

namespace std {
    template<>
    struct hash<place> {
        std::size_t operator()(const place& p) const {
            return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
        }
    };
}

#define MAP_SIZE           768
#define MAP_HEIGHT         904
#define MAP_WIDTH          1296
#define IDI_MYICON 101
#define USING_BACKPACK

#define UNIT_SIZE          32
#define IMAGE_SIZE         32
#define ID_BUTTON_CLICK    1001
#define RELATIVE_WIDTH           (((MAP_WIDTH - 16) / UNIT_SIZE) - 8)
#define RELATIVE_HEIGHT          (MAP_HEIGHT / UNIT_SIZE)
#define WM_CUSTOM_BUTTON_CLICK   (WM_USER + 1)
#define CENTER(WIDTH)            MAP_WIDTH / 2 - (WIDTH) / 2
#define AUTO_MOVING_COOLDOWN 800
#define WATER_SPREADING_COOLDOWN 600
#define WATER_REFRESHING_COOLDOWN 200

#define WITHIN_OPEN_INTERVAL(a, b, c) ((a) > (b) && (a) < (c))
#define WITHIN_CLOSED_INTERVAL(a, b, c) ((a) >= (b) && (a) <= (c))
#define WITHIN_OPEN_CLOSED_INTERVAL(a, b, c) ((a) > (b) && (a) <= (c))
#define WITHIN_CLOSED_OPEN_INTERVAL(a, b, c) ((a) >= (b) && (a) < (c))

struct pathfinding {
    int forward;
    int value;
};
struct shape {
    int next;
    int last;
};
struct Pickaxe {
    int Material;
    int Durability;
};
struct BUTTON {
    const wchar_t* Actived;
    const wchar_t* Non_Actived;
};
struct Media {
    const LPCTSTR Media;
    int Length;
};