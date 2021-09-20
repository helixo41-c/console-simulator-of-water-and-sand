#include <stdio.h>
#include <stdlib.h>

#define _WIN32_WINNT 0x0501
#include <windows.h>

#define width 80
#define height 24
#define c_sand (char)176
#define c_water (char)219
#define c_wall '#'
#define c_space ' '

typedef char Tmap[height][width];
Tmap map;
POINT mousePos;
POINT cellSize;

enum {s_sand = 0, s_water, s_wall, s_last} substance = s_sand;
char subChar[] = {c_sand, c_water, c_wall};
char *subName[] = {"sand", "water", "wall"};


POINT GetMousePos(HWND hwnd, POINT cellSize)
{
    static POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
    pt.x /= cellSize.x;
    pt.y /= cellSize.y;
    return pt;
}

POINT GetSellSize(HWND hwnd)
{
    RECT rct;
    GetClientRect(hwnd, &rct);
    POINT sellSz;
    sellSz.x = (rct.right - rct.left) / width;
    sellSz.y = (rct.bottom - rct.top) / height;
    return sellSz;
}

void ClearMap()
{
    memset(map, c_space, sizeof(map));
    map[height - 1][width] = '\0';
}

void SetCurPos(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ShowMap(Tmap map)
{
    SetCurPos(0, 0);
    printf("%s", map[0]);
}

void ShowInfo()
{
    SetCurPos(0, height);
    printf(">");
    for(int i = 0; i < s_last; i++)
    {
        printf("%d-%s\t", i + 1, subName[i]);
    }
    printf("LMB-put %s\tRMB-clear cell or SPACE for all map", subName[substance]);
}

void SelectSubstance()
{
    for(int i = 0; i < s_last; i++)
    {
        if(GetKeyState('1' + i) < 0) substance = i;
    }
}

void PutLine(POINT a, POINT b, char sub)
{
    float dx = (b.x - a.x) / (float)width;
    float dy = (b.y - a.y) / (float)width;
    for(int i = 0; i < width; i++)
    {
        map[(int)(a.y + dy * i)][(int)(a.x + dx * i)] = sub;
    }
}

void PutSubstance(POINT pt)
{
    static POINT old;

    if(GetKeyState(VK_LBUTTON) < 0)
    {
        PutLine(old, pt, subChar[substance]);
    }
    else if(GetKeyState(VK_RBUTTON) < 0)
    {
        PutLine(old, pt, c_space);
    }

    old = pt;
}

char IfPointInMap(int x, int y)
{
    return !((x < 0) || (y < 0) || (x >= width) || (y >= height));
}

void MoveSand(int x, int y)
{
    for(int i = 0; i <= 1; i+= (i == 0 ? -1 : 2))
    {
        if(IfPointInMap(x + 1, y + 1))
        {
            if((map[y + 1][x + i] == c_space) ||
               (map[y + 1][x + i] == c_water))
            {
                map[y][x] = map[y + 1][x + i];
                map[y + 1][x + i] = c_sand;
                break;
            }
        }
    }
}

Tmap mapTmp;
char waterLevel;
POINT foundPoint;

void FindWaterPath(int x, int y)
{
    if(!IfPointInMap(x, y)) return;

    if((y >= waterLevel) && (y > foundPoint.y))
    {
        if(mapTmp[y][x] == c_space)
        {
            foundPoint.x = x;
            foundPoint.y = y;
        }
    }
    if(mapTmp[y][x] == c_water)
    {
        mapTmp[y][x] = '#';
        FindWaterPath(x, y - 1);
        FindWaterPath(x, y + 1);
        FindWaterPath(x - 1, y);
        FindWaterPath(x + 1, y);
    }
}

void MoveWater(int x, int y)
{
    if(!IfPointInMap(x, y + 1)) return;
    if(map[y + 1][x] == c_space)
    {
        map[y][x] = c_space;
        map[y + 1][x] = c_water;
    }
    else if(map[y + 1][x] == c_water)
    {
        waterLevel = y + 1;
        foundPoint.y = -1;
        memcpy(mapTmp, map, sizeof(map));
        FindWaterPath(x, y + 1);
        if(foundPoint.y >= 0)
        {
            map[foundPoint.y][foundPoint.x] = c_water;
            map[y][x] = c_space;
        }
    }
}

void MoveSubstance()
{
    for(int j = height - 1; j >= 0; j--)
    {
        for(int i = 0; i < width; i++)
        {
            if(map[j][i] == c_sand) MoveSand(i, j);
            if(map[j][i] == c_water) MoveWater(i, j);
        }
    }
}

int main()
{
    HWND hwnd = GetConsoleWindow();
    cellSize = GetSellSize(hwnd);
    ClearMap();
    do
    {
        mousePos = GetMousePos(hwnd, cellSize);
        SelectSubstance();
        PutSubstance(mousePos);

        MoveSubstance();

        ShowMap(map);
        ShowInfo();
        if(GetKeyState(VK_SPACE) < 0)
        {
            ClearMap();
        }
        Sleep(50);
    }
    while(GetKeyState(VK_ESCAPE) >= 0);

    return 0;
}
