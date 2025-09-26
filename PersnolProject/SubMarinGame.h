#pragma once

#include <cstdio>
#include <cstdlib>

const int MapHeight = 31;
const int MapWidth = 41;

static inline int Clamp(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

struct Player {
    int Height;
    int Width;
};

struct Enemy {
    int Height;
    int Width;
    bool Alive;
};

enum ItemType {
    ITEM_TORPEDO,
    ITEM_SONAR,
    ITEM_MINE,
    ITEM_UPGRADE
};

struct Item {
    int H;
    int W;
    ItemType Type;
    int Life;
    bool Active;
};

struct Mine {
    int H;
    int W;
    int Life; // ≥≤¿∫ ≈œ
    bool Armed;
};

struct Ammo {
    int Torpedo;
    int Sonar;
    int MineStock;
    int BombStock;
};

class Game {
public:
    Game();
    void Init();
    void Run(); // main loop

private:
    // map & render
    char Map[MapHeight][MapWidth];

    // player & world state
    Player P;

    Enemy gEnemies[5];
    int gEnemyCount;

    Item gItems[20];
    int gItemCount;

    Mine gMines[32];
    int gMineCount;

    int gSonar[MapHeight][MapWidth];
    bool gRubble[MapHeight][MapWidth];
    bool gTrail[MapHeight][MapWidth];
    bool gSonarTrail[MapHeight][MapWidth];

    Ammo gAmmo;

    int gTurn;
    int gMoveLeft;

    // upgrade / delay-explosion scheduling
    bool gHasUglyPatch;
    bool gDelayBoomArmed;
    int gDelayH;
    int gDelayW;
    int gDelayExplodeTurn; // scheduled turn number

    // helpers & game logic
    void ClearMap(char fill);
    void BuildSea();
    void BuildWall();
    void BuildWallW();
    void RebuildFrame();
    void PlayerPosition();
    int PlayerMove();

    bool AnyEnemyAlive();
    int DoExplosionAt(int h, int w, int Radius);

    int ShotTorpedo(int RangeX);
    int ShootMineRight(int RangeX);
    int ShootSonarRight();

    void TickSonar();
    void ApplySonarOverlay();

    void EnemySpawn();
    bool IsInside(int h, int w);
    bool IsEnemyAt(int h, int w, int exceptIndex);
    bool CanEnemyEnter(int h, int w);
    void EnemyMovePhase(int maxSteps, bool randomSteps);

    void StartTurn();
    void EndTurn();

    void ClearTrail();
    void ClearSonarTrail();

    bool IsItemAt(int h, int w);
    void RandomItemSpawn();
    void TryPickupItem();

    // mines/triggers
    bool IsCellFreeForMine(int h, int w);
    void CheckMinesTrigger();
    void TickMines();

    // HUD / IO
    void DrawMap();
    void PrintHUD();
    void PrintMenu();

    // utility
    void PlaceInitialEnemy();
};