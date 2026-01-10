#include <stdio.h>
#include <easyx.h>
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#include <graphics.h>
#include <gdiplus.h>
#include "png_library.h"
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"gdiplus.lib")

constexpr auto SCREEN_WIDTH = 645;
constexpr auto SCREEN_HEIGHT = 735;
constexpr auto PLAYER_SIZE = 85;
constexpr auto BORDER_SIZE = 130;
constexpr auto ENEMY_SIZE = 75;
constexpr auto ENEMY_BULLET_SIZE = 30;
constexpr auto ENEMY_NUM_MAX = 10;
constexpr auto SPAWN_SPEED_ENE = 0.5;
constexpr auto SPAWN_SPEED_ENEBUL = 0.5;
constexpr auto SPAWN_SPEED_BULLET = 100;
constexpr auto BULLET_NUM_MAX = 50;
constexpr auto E_BULLET_NUM_MAX = 50;
constexpr int FPS = 15;


typedef struct vectorposition
{
    int dx;
    int dy;
}vectpos;//定义向量（其实不是，应该叫增量，但我要取这个名），用来实现自机狙

typedef struct position
{
    int x;
    int y;
    vectpos vectpos;
    int color;//弹幕颜色，放在这改动最小
}pos;
//坐标

typedef	struct plane
{
    pos planepos;
    pos bulletpos[BULLET_NUM_MAX + 20];
    int bulletlen;
    int bulletspeed;
}plane;//自机，本来也用来表示敌机的，但做到后面不方便

typedef	struct enemyplane
{
    pos planepos;
    DWORD lastshottime, begintime;
    //pos bulletpos[E_BULLET_NUM_MAX];
    double shotpassedtime;
}enemyplane;//敌机

typedef struct enemyBullet
{
    pos bulletpos;
    int ownerIndex;     // 发射这颗子弹的敌机索引
}enemyBullet;

plane player;//自机变量
//plane enermy[ENEMY_NUM_MAX];//敌机变量
enemyplane enemy[ENEMY_NUM_MAX] ;//敌机变量
enemyBullet enemyBullets[E_BULLET_NUM_MAX];  // 独立的弹幕数组
int enemyBulletLen = 0; // 当前存在的敌机弹幕鼠粮
int enemylen ;//存活敌机数量
static time_t EstartTime, EendTime,startplaytime;
DWORD starttime, endtime, B_starttime,B_endtime;//, B_endtime;
IMAGE img[5];
IMAGE_PNG img2[11];
int power = 0;
int* pPower = & power;
int playerspeed ;
int Ebulletspeed;
int borderangle;
int borderalpha ;
int playtime ;
int* palpha = &borderalpha;



void initGame();
void drawGame();
void updateGame();
void initEnermy();
void destroy();
int areInierSecting(pos c1, pos c2, int radius);
vectpos getdxdy(int x, int y);
int WTF_invsqrt(int x);


int main()//主循环
{
    init_png_library();
    LoadPNG(&img2[0], L"Resources\img\background.png");
    LoadPNG(&img2[2], L"Resources\img\enermy1.png");
    LoadPNG(&img2[1], L"Resources\img\player.png");
    LoadPNG(&img2[3], L"Resources\img\player1.png");
    LoadPNG(&img2[4], L"Resources\img\ply_border_01.png"); //这个东西到底叫啥啊
    LoadPNG(&img2[5], L"Resources\img\player_bullet.png");
    LoadPNG(&img2[6], L"Resources\img\enemy_bullet0.png");
    LoadPNG(&img2[7], L"Resources\img\enemy_bullet1.png");
    LoadPNG(&img2[8], L"Resources\img\enemy_bullet2.png");
    LoadPNG(&img2[9], L"Resources\img\enemy_bullet3.png");
    LoadPNG(&img2[10], L"Resources\img\enemy_bullet4.png");

    
    
    initGame();
    *palpha = 0;
    while (1) {
        updateGame();
        drawGame();
        destroy();
    }

    mciSendString(L"close bgm", NULL, 0, NULL);

    getchar();
    getchar();
    return 0;
}

void initGame()//初始化
{
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    power = 0;
    *palpha = 0;  

    srand((unsigned)time(NULL));
    player.bulletlen = 0;
    player.bulletspeed = 15;
    Ebulletspeed = 5;
    player.planepos = { SCREEN_WIDTH / 2 ,600 };

    for (int i = 0; i <BULLET_NUM_MAX + 20; i++)
    {
        player.bulletpos[i] = { 0, 0 };
    }

     enemylen = 0;
    for (int i = 0; i <ENEMY_NUM_MAX;  i++)
    {
        enemy[i].planepos = { 0, 0 };
        enemy[i].shotpassedtime = 0;
        enemy[i].lastshottime = 0;
        enemy[i].begintime = 0;
    }

    enemyBulletLen = 0;
    for (int i = 0; i < E_BULLET_NUM_MAX; i++)
    {
        enemyBullets[i].bulletpos = { 0, 0, {0, 0}, 0 };
        enemyBullets[i].ownerIndex = -1;
    }

    //PlaySound(L"bgm.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NOWAIT);
    mciSendString(L"open \".\\Resources\\music\\bgm.mp3\" alias bgm", NULL, 0, NULL);
    mciSendString(L"play bgm repeat", NULL, 0, NULL);

    EstartTime = time(NULL);
    starttime = GetTickCount();
    B_starttime = GetTickCount();
    startplaytime = time(NULL);
    borderangle = 0;  // 重置角度


}

void drawGame()
{
    BeginBatchDraw();
    PutPNGEX(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, &img2[0], 255);
    for (int i = 0; i < player.bulletlen; i++)
    {
        PutPNGEX(player.bulletpos[i].x - 20, player.bulletpos[i].y - 74, 0, 0, &img2[5], 168);
    }
    if (playerspeed == 3)
    {
        PutPNGEX(player.planepos.x - PLAYER_SIZE / 2,
            player.planepos.y - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, &img2[3], 255);
    }
    if (playerspeed == 6)
    {
        PutPNGEX(player.planepos.x - PLAYER_SIZE / 2,
            player.planepos.y - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, &img2[1], 255);
    }
    if (*palpha > 10)
    {
        PutPNGRotateEx(player.planepos.x - BORDER_SIZE / 2, player.planepos.y - BORDER_SIZE / 2,
            BORDER_SIZE, BORDER_SIZE, borderangle, &img2[4], borderalpha, 0x00FFFFFF);
    }

    // 绘制所有弹幕
    for (int i = 0; i < enemyBulletLen; i++)
    {
        PutPNGEX(enemyBullets[i].bulletpos.x - ENEMY_BULLET_SIZE / 2,
            enemyBullets[i].bulletpos.y - ENEMY_BULLET_SIZE / 2,
            ENEMY_BULLET_SIZE, ENEMY_BULLET_SIZE,
            &img2[enemyBullets[i].bulletpos.color], 255);
    }

    for (int i = 0; i < enemylen; i++)
    {
        PutPNGEX(enemy[i].planepos.x - ENEMY_SIZE / 2,
            enemy[i].planepos.y - ENEMY_SIZE / 2, ENEMY_SIZE, ENEMY_SIZE, &img2[2], 255);
    }
    RECT rect = { 0,10,SCREEN_WIDTH,SCREEN_HEIGHT };
    setbkmode(TRANSPARENT);
    wchar_t str1[15] = { 0 };
    wchar_t str2[15] = { 0 };
    swprintf_s(str1, L"Power:%d", power);
    drawtext(str1, &rect, DT_TOP | DT_TOP);
    swprintf_s(str2, L"Time:%ld", (long)playtime);
    drawtext(str2, &rect, DT_TOP | DT_RIGHT);
    EndBatchDraw();
}

void updateGame()//更新游戏
{
    DWORD currentTime = GetTickCount();
    DWORD deltaTime = currentTime - starttime;  // 时间差，锁帧用

    if (deltaTime >=FPS)
    {
        if (GetAsyncKeyState(VK_SHIFT) &  0x8000)
        {
            playerspeed = 3;
            if (*palpha <= 150)//魔法阵动画
                *palpha += 14;
        }
        else
        {
            playerspeed = 6;
            if (*palpha>10)
                *palpha -= 10;
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000)
        {
            player.planepos.y -=  playerspeed;
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)
        {
            player.planepos.y += playerspeed ;
        }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && (GetAsyncKeyState(VK_LEFT) & 0x8000))
        {
            player.planepos.x -= playerspeed ;
        }
        //原汁原味这一块，ZUN史山代码的这个bug也小小复现一下
        else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        {
            player.planepos.x -= playerspeed ;
        }
        else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        {
            player.planepos.x += playerspeed ;
        }

        B_endtime = GetTickCount();
        if (GetAsyncKeyState('Z') & 0x8000)
        {
            if ((B_endtime - B_starttime) >= SPAWN_SPEED_BULLET    )
            {
                if (power < 60)
                {
                    if (player.bulletlen < BULLET_NUM_MAX)
                    {
                        player.bulletpos[player.bulletlen].x = player.planepos.x;
                        player.bulletpos[player.bulletlen].y = player.planepos.y;
                        player.bulletlen++;
                    }
                    B_starttime = B_endtime;
                }
                else
                {
                    if (player.bulletlen < BULLET_NUM_MAX)
                    {
                        player.bulletpos[player.bulletlen].x = player.planepos.x + PLAYER_SIZE / 7;
                        player.bulletpos[player.bulletlen].y = player.planepos.y - PLAYER_SIZE / 2.5;
                        player.bulletlen++;
                        player.bulletpos[player.bulletlen].x = player.planepos.x - PLAYER_SIZE / 7;
                        player.bulletpos[player.bulletlen].y = player.planepos.y - PLAYER_SIZE / 2.5;
                        player.bulletlen++;
                    }
                    B_starttime = B_endtime;
                }
            }
        }

        if (player.planepos.x > SCREEN_WIDTH - 4)
        {
            player.planepos.x = SCREEN_WIDTH - 4;
        }
        if (player.planepos.x < 4)
        {
            player.planepos.x = 4;
        }
        if (player.planepos.y > SCREEN_HEIGHT - 15)
        {
            player.planepos.y = SCREEN_HEIGHT - 15;
        }
        if (player.planepos.y < 4)
        {
            player.planepos.y = 4;
        }

        for (int i = 0; i < enemylen; i++)
        {
            enemy[i].planepos.y += 2;
        }

        for (int i = 0; i < player.bulletlen; i++)
        {
            player.bulletpos[i].y -= player.bulletspeed;
        }

        // 更新所有弹幕位置
        for (int i = 0; i < enemyBulletLen; i++)
        {
            enemyBullets[i].bulletpos.x -= enemyBullets[i].bulletpos.vectpos.dx;
            enemyBullets[i].bulletpos.y -= enemyBullets[i].bulletpos.vectpos.dy;
        }

        borderangle += 1.5;
        if (borderangle >= 360) borderangle = 0;

        initEnermy();

        deltaTime -= FPS;  
        starttime += FPS;//更新时间  
    }
    playtime = time(NULL) - startplaytime;
}





void initEnermy()//敌机
{
    EendTime = time(NULL);
    double elapsedTime = difftime(EendTime,EstartTime);
    if (elapsedTime >= SPAWN_SPEED_ENE)
    {
        if (enemylen <   ENEMY_NUM_MAX  )
        {
            int x = ((rand() % (SCREEN_WIDTH - ENEMY_SIZE)) + ENEMY_SIZE / 2);
            int y = (-ENEMY_SIZE / 2);

            enemy[enemylen].planepos.x =  x;
            enemy[enemylen].planepos.y = y;
            enemy[enemylen].begintime = GetTickCount () ;

            enemylen++;

        }
        EstartTime = EendTime;
    }



    /*EstartTime = EendTime;
    }
    int h = 0; int* ph = &h;
    int t = (rand() % (enemylen + 1)) - 1;
    if (t >= 0)
    {
        *ph = t;
    }
    float bullet_bettween_time = 100;
    for (int f = 0; f < 10; f++)
    {
        enemy[h].Ebulletend[f] = GetTickCount();
        if ((enemy[h].Ebulletend[f] - enemy[h].Ebulletstart[f]) > ((rand() % (SPAWN_SPEED_ENEBUL)) + 50))
        {
            enemy[h].bulletpos[f].x = enemy[h].planepos.x;
            enemy[h].bulletpos[f].y = enemy[h].planepos.y;
        }*/ //完全错误的思路，不删以反思



    for (int i = 0; i < enemylen; i++)
    {
        int a = (SPAWN_SPEED_ENEBUL * 10);
        double shotime = ((rand() % (a)) + 1) / 10;
        enemy[i].lastshottime = GetTickCount();
        enemy[i].shotpassedtime = (enemy[i].lastshottime - enemy[i].begintime) / 1000;
        if ((enemy[i].shotpassedtime > shotime) && (enemyBulletLen < E_BULLET_NUM_MAX))
        {
            //】发射弹幕
            enemyBullets[enemyBulletLen].bulletpos.x = enemy[i].planepos.x;
            enemyBullets[enemyBulletLen].bulletpos.y = enemy[i].planepos.y + ENEMY_SIZE / 4;
            enemyBullets[enemyBulletLen].bulletpos.vectpos = getdxdy(
                enemyBullets[enemyBulletLen].bulletpos.x,
                enemyBullets[enemyBulletLen].bulletpos.y);
            enemyBullets[enemyBulletLen].bulletpos.color = (rand() % 5) + 6;
            enemyBullets[enemyBulletLen].ownerIndex = i;    // 记录发射的敌机
            enemyBulletLen++;
            enemy[i].begintime = enemy[i].lastshottime;
        }
    }
}

vectpos getdxdy(int x, int y)
{
    int a = x - player.planepos.x ;
    int b = y - player.planepos.y;
    int dist_sq = a * a + b * b;
    int norm = WTF_invsqrt(dist_sq); 
    if (norm == 0) norm = 1;

    vectpos dxy =
    {
        (Ebulletspeed * a) / norm,
        (Ebulletspeed * b) / norm
    };
    return dxy;
}//获取增量

//float quicksqrt(int m, int n)
//{
//    int max = (m > n ? m : n);
//    int min = (m < n ? m : n);
//    return(max + (min >> 2));
//}
//豆包给的辣鸡算法让我子弹时快时慢


int WTF_invsqrt(int x) //传奇这一块
{
    if (x <= 1)
    {
        return 1;
    }
    float x2 = x * 0.5f;
    float y = (float)x;
    int i = *(int*)&y;
    i = 0x5f3759df - (i >> 1);  // what the fxxk? 
                                //魔法数字这一块
    y = *(float*)&i;
    y = y * (1.5f - (x2 * y * y));
    return (int)(1.0f / y);
}


void destroy()
{
    for (int i = 0; i < player.bulletlen;)//自机子弹，边界
    {
        if (player.bulletpos[i].y < -1)
        {
            for (int j = i; j < player.bulletlen - 1; j++)
            {
                player.bulletpos[j] = player.bulletpos[j + 1];
            }
            player.bulletlen--;
            continue;  
        }
        i++;
    }

    for (int i = 0; i < enemylen;)
    {
        // 敌机碰撞
        if (areInierSecting(player.planepos, enemy[i].planepos, 196))
        {
            playerspeed = 3;
            if (IDYES == MessageBox(GetHWnd(), L"游戏结束，要重来吗？", L"提示", MB_YESNO))
            {
                initGame();
                return;  
            }
            else {
                exit(0);
            }
        }

        // 敌机边界检测
        if (enemy[i].planepos.y > SCREEN_HEIGHT + (ENEMY_SIZE / 2))
        {
             for (int j = i; j < enemylen - 1; j++)
            {
                enemy[j] = enemy[j + 1];
            }
            enemylen--;
            continue;  
        }

        // 击破检测
        for (int l = 0; l < player.bulletlen; l++)
        {
            if (areInierSecting(player.bulletpos[l], enemy[i].planepos, 196))
            {
                for (int j = i; j < enemylen - 1; j++)
                {
                    enemy[j] = enemy[j + 1];
                }
                enemylen--;
                for (int j = l; j < player.bulletlen - 1; j++)
                {
                    player.bulletpos[j] = player.bulletpos[j + 1];
                }
                player.bulletlen--;

                i-- ; 
                if (power < 100)
                {
                    *pPower +=  4 ;
                }
                break;
            }
        }

        i++;
    }

    // 弹幕边界检查
    for (int i = 0; i < enemyBulletLen;)
    {
         if (enemyBullets[i].bulletpos.x < -(ENEMY_SIZE >> 2) ||
            enemyBullets[i].bulletpos.x >(SCREEN_WIDTH + (ENEMY_SIZE >> 2)) ||
            enemyBullets[i].bulletpos.y < -(ENEMY_SIZE >> 2) ||
            enemyBullets[i].bulletpos.y >(SCREEN_HEIGHT + (ENEMY_SIZE >> 2)))
        {
            for (int j = i; j < enemyBulletLen - 1; j++) {
                enemyBullets[j] = enemyBullets[j + 1];
            }
            enemyBulletLen--;
        }
        else
        {
             //、、miss检查
            if (areInierSecting(player.planepos, enemyBullets[i].bulletpos, 100))
            {
                playerspeed = 3;
                for (int j = i; j < enemyBulletLen - 1; j++) {
                    enemyBullets[j] = enemyBullets[j + 1];
                }
                enemyBulletLen--;

                mciSendString(L"close bgm", NULL, 0, NULL);

                if (IDYES == MessageBox(GetHWnd(), L"游戏结束，要重来吗？", L"提示", MB_YESNO))
                {
                    initGame();
                    return;
                }
                else {
                    exit(0);
                }
            }
            else
            {
                i++; 
            }
        }
    }
}

int areInierSecting(pos c1, pos c2, int radius)//检测相交。
{
    return

        ( c1.x -c2.x) *( c1.x -c2.x) +( c1.y -c2.y) *( c1.y -c2.y)
        <
        radius
        ;
}