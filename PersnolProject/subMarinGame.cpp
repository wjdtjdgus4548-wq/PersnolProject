#include "SubMarinGame.h"
#include <iostream>
#include <ctime>

// 생성자
Game::Game()
{
    // 기본 초기화
    gEnemyCount = 0;
    gItemCount = 0;
    gMineCount = 0;

    for (int r = 0; r < MapHeight; r = r + 1) {
        for (int c = 0; c < MapWidth; c = c + 1) {
            gSonar[r][c] = 0;
            gRubble[r][c] = false;
            gTrail[r][c] = false;
            gSonarTrail[r][c] = false;
        }
    }

    gAmmo.Torpedo = 3;
    gAmmo.Sonar = 5;
    gAmmo.MineStock = 1;
    gAmmo.BombStock = 0;

    gTurn = 1;
    gMoveLeft = 5;

    gHasUglyPatch = false;
    gDelayBoomArmed = false;
    gDelayH = -1;
    gDelayW = -1;
    gDelayExplodeTurn = -1;
}


void Game::Init()
{
    std::srand((unsigned)std::time(NULL));

    P.Height = MapHeight / 2;
    P.Width = MapWidth / 4 - 5;

    ClearMap('.');
    BuildWallW();
    BuildWall();

    PlaceInitialEnemy();
    StartTurn();
}

// 메인루프
void Game::Run()
{
    while (true)
    {
        RebuildFrame();
        DrawMap();
        PrintHUD();
        PrintMenu();

        int sel = 0;
        std::cin >> sel;
        if (!std::cin.good())
        {
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            continue;
        }

        if (sel == 9)
        {
            printf("게임 종료.\n");
            break;
        }
        else if (sel == 1) // 이동
        {
            if (gMoveLeft <= 0)
            {
                printf("남은 이동횟수가 0입니다. 턴을 넘겨주세요.\n");
            }
            else
            {
                int r = PlayerMove();
                if (r == -1)
                {
                    printf("게임 종료.\n");
                    break;
                }
                if (r == 1)
                {
                    gMoveLeft = gMoveLeft - 1;
                    TryPickupItem();
                    if (gMoveLeft == 0)
                    {
                        printf("이동 횟수 0! 턴을 넘겨주세요\n");
                    }
                }
            }

            // 플레이어 행동에 따른 적 반응 (간단)
            EnemyMovePhase(1, true);
        }
        else if (sel == 2) // 어뢰
        {
            if (gAmmo.Torpedo <= 0) {
                printf("[어뢰 없음]\n");
            }
            else {
                int rangeX = 0;
                std::printf("어뢰가 날라갈 거리를 입력해 주세요 (2~30): ");
                std::cin >> rangeX;
                if (!std::cin.good()) {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::printf("잘못된 입력입니다. 명령 취소.\n");
                }
                else {
                    rangeX = Clamp(rangeX, 2, 30);
                    int kills = ShotTorpedo(rangeX);
                    if (kills > 0) std::printf("[격침 %d]\n", kills);
                    else std::printf("[격침 0]\n");
                    EndTurn();
                }
            }
        }
        else if (sel == 3) // 소나
        {
            if (gAmmo.Sonar <= 0) {
                printf("[소나 없음]\n");
            }
            else {
                ShootSonarRight();
                EndTurn();
            }
        }
        else if (sel == 4) // 디버그 적 소환
        {
            EnemySpawn();
        }
        else if (sel == 5) // 턴 종료
        {
            EndTurn();
        }
        else if (sel == 7) // 기뢰 발사
        {
            if (gAmmo.MineStock <= 0) {
                printf("[기뢰 없음]\n");
            }
            else {
                int rangeX = 0;
                std::printf("기뢰가 날아갈 거리를 입력해 주세요 (2~30): ");
                std::cin >> rangeX;
                if (!std::cin.good()) {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::printf("잘못된 입력입니다. 명령 취소.\n");
                }
                else {
                    rangeX = Clamp(rangeX, 2, 30);
                    ShootMineRight(rangeX);
                    EndTurn();
                }
            }
        }
        else
        {
            printf("알 수 없는 입력입니다.\n");
        }

        // 승/패 검사
        int Alive = 0;
        for (int i = 0; i < gEnemyCount; i = i + 1) {
            if (gEnemies[i].Alive) Alive = Alive + 1;
        }
        if (Alive == 0 && gEnemyCount > 0) {
            printf("[모든 적 격침 - 승리!]\n");
            break;
        }
        if (gAmmo.Torpedo == 0 && Alive > 0) {
            printf("[어뢰소진 적이 %d 남았습니다 - 패배]\n", Alive);
            break;
        }
        if (Alive >= 5) {
            printf("[적이 5마리 이상 소환됬습니다 - 패배]\n");
            break;
        }
    }

    printf("엔터를 누르면 게임이 종료됩니다");
    std::cin.clear();
    std::cin.ignore(1024, '\n');
    std::cin.get();
}

// 맵 랜더링
void Game::ClearMap(char fill)
{
    for (int h = 0; h < MapHeight; h = h + 1) {
        for (int w = 0; w < MapWidth; w = w + 1) {
            Map[h][w] = fill;
        }
    }
}

void Game::BuildSea()
{
    ClearMap('.');
}

void Game::BuildWallW()
{
    for (int h = 0; h < MapHeight; h = h + 1) {
        for (int w = 0; w < MapWidth; w = w + 1) {
            if (h == 0 || w == 0 || h == MapHeight - 1 || w == MapWidth - 1) {
                Map[h][w] = '#';
            }
        }
    }
}

void Game::BuildWall()
{
    int Wall = MapWidth / 4;
    for (int h = 0; h < MapHeight; h = h + 1) {
        Map[h][Wall] = '#';
    }
}

void Game::RebuildFrame()
{
    BuildSea();
    BuildWallW();
    BuildWall();

    // 잔해 / 트레일 / 소나 그리기
    for (int h = 0; h < MapHeight; h = h + 1) {
        for (int w = 0; w < MapWidth; w = w + 1) {
            if (gRubble[h][w] && Map[h][w] != '#') {
                Map[h][w] = '*';
            }
            else if (gTrail[h][w] && Map[h][w] != '#') {
                Map[h][w] = '>';
            }
            else if (gSonar[h][w] > 0 && Map[h][w] == '.') {
                Map[h][w] = '~';
            }
        }
    }

    // 아이템 그리기
    for (int i = 0; i < gItemCount; i = i + 1) {
        if (!gItems[i].Active || gItems[i].Life <= 0) continue;
        int ih = gItems[i].H;
        int iw = gItems[i].W;
        if (ih < 0 || ih >= MapHeight || iw < 0 || iw >= MapWidth) continue;
        if (Map[ih][iw] == '#') continue;
        if (Map[ih][iw] == '*') continue;
        char ch = '?';
        switch (gItems[i].Type) {
        case ITEM_TORPEDO: ch = 'T'; break;
        case ITEM_SONAR: ch = 'S'; break;
        case ITEM_MINE: ch = 'M'; break;
        case ITEM_UPGRADE: ch = 'U'; break;
        }
        Map[ih][iw] = ch;
    }

    // 지뢰 표시
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (!gMines[i].Armed) continue;
        int mh = gMines[i].H;
        int mw = gMines[i].W;
        if (mh < 0 || mh >= MapHeight || mw < 0 || mw >= MapWidth) continue;
        if (Map[mh][mw] == '#') continue;
        if (Map[mh][mw] == '*') continue;
        Map[mh][mw] = 'm';
    }

    // 소나 오버레이 (적 표시)
    ApplySonarOverlay();

    // 플레이어 위치
    PlayerPosition();
}

void Game::PlayerPosition()
{
    Map[P.Height][P.Width] = 'P';
}

void Game::DrawMap()
{
    for (int h = 0; h < MapHeight; h = h + 1) {
        for (int w = 0; w < MapWidth; w = w + 1) {
            printf("%c", Map[h][w]);
        }
        printf("\n");
    }
}

// 플레이어
int Game::PlayerMove()
{
    char Input;
    std::printf("WASD를 입력해 주세요(Q)는 종료 : ");
    std::cin >> Input;
    if (Input == 'q' || Input == 'Q') return -1;
    int NextH = P.Height;
    int NextW = P.Width;

    if ((Input == 'w' || Input == 'W') && P.Height > 0) {
        NextH = P.Height - 1;
    }
    else if ((Input == 's' || Input == 'S') && P.Height < MapHeight - 1) {
        NextH = P.Height + 1;
    }
    else if ((Input == 'd' || Input == 'D') && P.Width < MapWidth - 1) {
        NextW = P.Width + 1;
    }
    else if ((Input == 'a' || Input == 'A') && P.Width > 0) {
        NextW = P.Width - 1;
    }
    else {
        return 0;
    }

    if (Map[NextH][NextW] == '#') {
        printf("여긴 벽이야 정신차려;;\n");
        return 0;
    }
    P.Height = NextH;
    P.Width = NextW;
    return 1;
}

// 적하고 적 이동 하는거
bool Game::AnyEnemyAlive()
{
    for (int i = 0; i < gEnemyCount; i = i + 1) {
        if (gEnemies[i].Alive) return true;
    }
    return false;
}

int Game::DoExplosionAt(int h, int w, int Radius)
{
    int Kills = 0;
    for (int a = -Radius; a <= Radius; a = a + 1) {
        for (int b = -Radius; b <= Radius; b = b + 1) {
            int th = h + a;
            int tw = w + b;
            if (th < 0 || th >= MapHeight || tw < 0 || tw >= MapWidth) {
                continue;
            }
            gRubble[th][tw] = true;
            for (int i = 0; i < gEnemyCount; i = i + 1) {
                if (!gEnemies[i].Alive) continue;
                if (gEnemies[i].Height == th && gEnemies[i].Width == tw) {
                    gEnemies[i].Alive = false;
                    Kills = Kills + 1;
                }
            }
        }
    }

    if (Kills > 0) {
        gAmmo.Torpedo = gAmmo.Torpedo + Kills;
        printf("[보상] 적 %d마리 격침! 어뢰 +%d (현재 %d)\n", Kills, Kills, gAmmo.Torpedo);
    }

    return Kills;
}

int Game::ShotTorpedo(int RangeX)
{
    if (gAmmo.Torpedo <= 0) {
        printf("[어뢰 없음]\n");
        return 0;
    }
    gAmmo.Torpedo = gAmmo.Torpedo - 1;

    int h = P.Height;
    int w = P.Width;

    int steps = Clamp(RangeX, 2, 30);

    for (int s = 0; s < steps; s = s + 1) {
        int nw = w + 1;
        if (nw >= MapWidth) {
            break;
        }
        if (Map[h][nw] == '#') {
            // 중간벽은 통과됨
            int wallCol = MapWidth / 4;
            if (nw == wallCol && nw + 1 < MapWidth && Map[h][nw + 1] != '#') {
                w = nw + 1;
                if (s < steps - 1) gTrail[h][w] = true;
                continue;
            }
            else {
                break;
            }
        }
        w = nw;
        if (s < steps - 1) gTrail[h][w] = true;
    }

    // 업그레이드 처리: 지연폭발 예약
    if (gHasUglyPatch) {
        gHasUglyPatch = false;
        gDelayBoomArmed = true;
        gDelayH = h;
        gDelayW = w;
        gDelayExplodeTurn = gTurn + 2; // 예약 턴
        printf("[지연폭발예약] (%d,%d) -> 폭발 예정 턴: %d\n", gDelayH, gDelayW, gDelayExplodeTurn);
        return 0;
    }
    else {
        int Kills = DoExplosionAt(h, w, 1);
        return Kills;
    }
}

int Game::ShootMineRight(int RangeX)
{
    if (gAmmo.MineStock <= 0) {
        printf("[기뢰없음]");
        return 0;
    }
    gAmmo.MineStock = gAmmo.MineStock - 1;

    int h = P.Height;
    int w = P.Width;

    int steps = Clamp(RangeX, 2, 30);

    int wallCol = MapWidth / 4;
    for (int s = 0; s < steps; s = s + 1) {
        int nw = w + 1;
        if (nw >= MapWidth) {
            w = MapWidth - 1;
            break;
        }
        if (Map[h][nw] == '#') {
            if (nw == wallCol) {
                int after = nw + 1;
                if (after < MapWidth && Map[h][after] != '#' && !gRubble[h][after]) {
                    w = after;
                    if (s < steps - 1) gTrail[h][w] = true;
                    continue;
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        w = nw;
        if (s < steps - 1) gTrail[h][w] = true;
    }

    if (Map[h][w] == '#') {
        if (w == wallCol && w + 1 < MapWidth && Map[h][w + 1] != '#') {
            w = w + 1;
        }
        else if (w - 1 >= 0 && Map[h][w - 1] != '#') {
            w = w - 1;
        }
    }

    if (gMineCount < 32) {
        gMines[gMineCount] = { h, w, 5, true };
        gMineCount = gMineCount + 1;
        printf("[기뢰 착지] (%d,%d) 남은 수명 5턴\n", h, w);
    }
    else {
        printf("[저장한도 초과 - 설치실패]\n");
    }
    return 1;
}

int Game::ShootSonarRight()
{
    if (gAmmo.Sonar <= 0) {
        printf("[소나 없음]\n");
        return 0;
    }
    gAmmo.Sonar = gAmmo.Sonar - 1;

    int CenterR = P.Height;
    int rMin = (CenterR - 1 >= 0 ? CenterR - 1 : 0);
    int rMax = (CenterR + 1 < MapHeight ? CenterR + 1 : MapHeight - 1);

    int cStart = P.Width + 1;
    int cEnd = MapWidth - 2;

    if (cStart <= cEnd) {
        for (int r = rMin; r <= rMax; r = r + 1) {
            for (int c = cStart; c <= cEnd; c = c + 1) {
                gSonar[r][c] = 3;
            }
        }
    }

    printf("소나 도착!\n");
    return 1;
}

void Game::TickSonar()
{
    for (int r = 0; r < MapHeight; r = r + 1) {
        for (int c = 0; c < MapWidth; c = c + 1) {
            if (gSonar[r][c] > 0) gSonar[r][c] = gSonar[r][c] - 1;
        }
    }
}

void Game::ApplySonarOverlay()
{
    bool Found = false;
    for (int i = 0; i < gEnemyCount; i = i + 1) {
        if (!gEnemies[i].Alive) continue;
        int er = gEnemies[i].Height;
        int ec = gEnemies[i].Width;
        if (er < 0 || er >= MapHeight || ec < 0 || ec >= MapWidth) continue;
        if (gSonar[er][ec] > 0 && Map[er][ec] != '#') {
            Map[er][ec] = 'E';
            Found = true;
        }
    }
    if (Found) printf("적이 발견됬습니다!\n");
}

void Game::EnemySpawn()
{
    if (gEnemyCount >= 5) return;
    int Wall = MapWidth / 4;
    int RightStart = Wall + 1;
    int WidthSpan = MapWidth - RightStart;

    for (int tries = 0; tries < 100; tries = tries + 1) {
        int h = rand() % MapHeight;
        int w = RightStart + (rand() % WidthSpan);
        if (Map[h][w] == '#') continue;
        if (gRubble[h][w]) continue;
        if (h == P.Height && w == P.Width) continue;
        bool Occupied = false;
        for (int i = 0; i < gEnemyCount; i = i + 1) {
            if (!gEnemies[i].Alive) continue;
            if (gEnemies[i].Height == h && gEnemies[i].Width == w) {
                Occupied = true;
                break;
            }
        }
        if (Occupied) continue;

        gEnemies[gEnemyCount].Height = h;
        gEnemies[gEnemyCount].Width = w;
        gEnemies[gEnemyCount].Alive = true;
        gEnemyCount = gEnemyCount + 1;
        break;
    }
}

bool Game::IsInside(int h, int w)
{
    return (0 <= h && h < MapHeight && 0 <= w && w < MapWidth);
}

bool Game::IsEnemyAt(int h, int w, int exceptIndex)
{
    for (int i = 0; i < gEnemyCount; i = i + 1) {
        if (i == exceptIndex) continue;
        if (!gEnemies[i].Alive) continue;
        if (gEnemies[i].Height == h && gEnemies[i].Width == w) return true;
    }
    return false;
}

bool Game::CanEnemyEnter(int h, int w)
{
    if (!IsInside(h, w)) return false;
    if (Map[h][w] == '#') return false;
    if (gRubble[h][w]) return false;
    if (h == P.Height && w == P.Width) return false;
    if (w < MapWidth / 4 + 1) return false;
    return true;
}

void Game::EnemyMovePhase(int maxSteps, bool randomSteps)
{
    for (int i = 0; i < gEnemyCount; i = i + 1) {
        if (!gEnemies[i].Alive) continue;

        int steps = 0;
        if (randomSteps) {
            steps = rand() % (maxSteps + 1);
        }
        else {
            steps = maxSteps;
        }

        for (int s = 0; s < steps; s = s + 1) {
            int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
            for (int k = 0; k < 4; k = k + 1) {
                int r = rand() % 4;
                int tmp0 = dirs[k][0];
                int tmp1 = dirs[k][1];
                dirs[k][0] = dirs[r][0];
                dirs[k][1] = dirs[r][1];
                dirs[r][0] = tmp0;
                dirs[r][1] = tmp1;
            }

            bool moved = false;
            for (int k = 0; k < 4 && !moved; k = k + 1) {
                int nh = gEnemies[i].Height + dirs[k][0];
                int nw = gEnemies[i].Width + dirs[k][1];
                if (!CanEnemyEnter(nh, nw)) continue;
                if (IsEnemyAt(nh, nw, i)) continue;
                gEnemies[i].Height = nh;
                gEnemies[i].Width = nw;
                moved = true;
            }
            if (!moved) break;
        }
    }

    // 적 이동 후 기뢰 트리거 검사
    CheckMinesTrigger();
}

// 턴
void Game::StartTurn()
{
    gMoveLeft = 5;
    ClearTrail();
    ClearSonarTrail();
}

void Game::EndTurn()
{
    // 적 엔드무브
    EnemyMovePhase(2, false);

    TickSonar();
    TickMines();

    // 아이템 수명 감소
    for (int i = 0; i < gItemCount; i = i + 1) {
        if (!gItems[i].Active) continue;
        gItems[i].Life = gItems[i].Life - 1;
        if (gItems[i].Life <= 0) gItems[i].Active = false;
    }

    // 지연폭발 예약 처리 (턴번호 기반)
    gTurn = gTurn + 1;
    if (gDelayBoomArmed && gDelayExplodeTurn > 0) {
        if (gTurn >= gDelayExplodeTurn) {
            int Kills = DoExplosionAt(gDelayH, gDelayW, 2);
            printf("[지연폭발] (%d,%d) 5x5 폭발! 격침 %d (턴 %d)\n", gDelayH, gDelayW, Kills, gTurn);
            gDelayBoomArmed = false;
            gDelayExplodeTurn = -1;
            gDelayH = gDelayW = -1;
        }
        else {
            int remain = gDelayExplodeTurn - gTurn;
            if (remain < 0) remain = 0;
            printf("[지연폭발대기] (%d,%d) 남은 턴: %d (현재 턴 %d)\n", gDelayH, gDelayW, remain, gTurn);
        }
    }

    // 5턴마다 적 소환
    if (gTurn % 5 == 0) {
        EnemySpawn();
    }

    StartTurn();
}

// 어뢰 기뢰 등 꼬리 소나도 원래 꼬리 만들었는데 게임 해보니까 소나 범위가 작으면 플레이가 불가능해서 현제는 사용 안함
void Game::ClearTrail()
{
    for (int r = 0; r < MapHeight; r = r + 1) {
        for (int c = 0; c < MapWidth; c = c + 1) gTrail[r][c] = false;
    }
}
void Game::ClearSonarTrail()
{
    for (int r = 0; r < MapHeight; r = r + 1) {
        for (int c = 0; c < MapWidth; c = c + 1) gSonarTrail[r][c] = false;
    }
}

// 아이템
bool Game::IsItemAt(int h, int w)
{
    for (int i = 0; i < gItemCount; i = i + 1) {
        if (!gItems[i].Active) continue;
        if (gItems[i].H == h && gItems[i].W == w) return true;
    }
    return false;
}

void Game::RandomItemSpawn()
{
    if (gItemCount >= 20) return;
    int Wall = MapWidth / 4;
    int LeftMinW = 1;
    int LeftMaxW = Wall - 1;

    for (int tries = 0; tries < 100; tries = tries + 1) {
        int h = 1 + (rand() % (MapHeight - 2));
        int w = LeftMinW + (rand() % (LeftMaxW - LeftMinW + 1));
        if (w < LeftMinW || w > LeftMaxW) continue;
        if (Map[h][w] == '#') continue;
        if (gRubble[h][w]) continue;
        if (h == P.Height && w == P.Width) continue;
        if (IsItemAt(h, w)) continue;

        ItemType Type = static_cast<ItemType>(rand() % 4);
        gItems[gItemCount].H = h;
        gItems[gItemCount].W = w;
        gItems[gItemCount].Type = Type;
        gItems[gItemCount].Life = 10;
        gItems[gItemCount].Active = true;
        gItemCount = gItemCount + 1;
        break;
    }
}

void Game::TryPickupItem()
{
    for (int i = 0; i < gItemCount; i = i + 1) {
        if (!gItems[i].Active) continue;
        if (gItems[i].H != P.Height || gItems[i].W != P.Width) continue;

        switch (gItems[i].Type) {
        case ITEM_TORPEDO:
            gAmmo.Torpedo = gAmmo.Torpedo + 1;
            printf("[획득] 어뢰 +1  (현재 %d)\n", gAmmo.Torpedo);
            break;
        case ITEM_SONAR:
            gAmmo.Sonar = gAmmo.Sonar + 1;
            printf("[획득] 소나 +1  (현재 %d)\n", gAmmo.Sonar);
            break;
        case ITEM_MINE:
            gAmmo.MineStock = gAmmo.MineStock + 1;
            printf("[획득] 기뢰 +1  (현재 %d)\n", gAmmo.MineStock);
            break;
        case ITEM_UPGRADE:
            gHasUglyPatch = true;
            printf("[획득] 어설픈 개조(다음 어뢰 5×5, 지연폭발)\n");
            printf("[안내] 사용법: 다음 어뢰 발사 후 '다음 턴' 종료 시 폭발합니다.\n");
            break;
        }

        gItems[i].Active = false;
        break;
    }
}

// 기뢰
bool Game::IsCellFreeForMine(int h, int w)
{
    if (h < 0 || h >= MapHeight) return false;
    if (w < 0 || w >= MapWidth) return false;
    if (gRubble[h][w]) return false;
    if (h == 0 || w == 0 || h == MapHeight - 1 || w == MapWidth - 1) return false;
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (!gMines[i].Armed) continue;
        if (gMines[i].H == h && gMines[i].W == w) return false;
    }
    for (int i = 0; i < gEnemyCount; i = i + 1) {
        if (!gEnemies[i].Alive) continue;
        if (gEnemies[i].Height == h && gEnemies[i].Width == w) return false;
    }
    return true;
}

void Game::CheckMinesTrigger()
{
    int explodeIdx[32];
    int explodeCnt = 0;
    for (int mi = 0; mi < gMineCount; mi = mi + 1) {
        if (!gMines[mi].Armed) continue;
        int mh = gMines[mi].H;
        int mw = gMines[mi].W;
        bool trigger = false;
        for (int ei = 0; ei < gEnemyCount; ei = ei + 1) {
            if (!gEnemies[ei].Alive) continue;
            int eh = gEnemies[ei].Height;
            int ew = gEnemies[ei].Width;
            int dh = mh - eh; if (dh < 0) dh = -dh;
            int dw = mw - ew; if (dw < 0) dw = -dw;
            if (dh <= 2 && dw <= 2) { trigger = true; break; }
        }
        if (trigger) {
            explodeIdx[explodeCnt] = mi;
            explodeCnt = explodeCnt + 1;
        }
    }

    for (int k = 0; k < explodeCnt; k = k + 1) {
        int idx = explodeIdx[k];
        if (!gMines[idx].Armed) continue;
        int mh = gMines[idx].H;
        int mw = gMines[idx].W;
        int kills = DoExplosionAt(mh, mw, 2);
        printf("[기뢰 폭발] (%d,%d) 격침 %d\n", mh, mw, kills);
        gMines[idx].Armed = false;
        gMines[idx].Life = 0;
    }

    
    int write = 0;
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (gMines[i].Armed && gMines[i].Life > 0) {
            if (write != i) gMines[write] = gMines[i];
            write = write + 1;
        }
    }
    gMineCount = write;
}

void Game::TickMines()
{
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (!gMines[i].Armed) continue;
        gMines[i].Life = gMines[i].Life - 1;
        if (gMines[i].Life <= 0) gMines[i].Armed = false;
    }
    int write = 0;
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (gMines[i].Armed && gMines[i].Life > 0) {
            if (write != i) gMines[write] = gMines[i];
            write = write + 1;
        }
    }
    gMineCount = write;
}

// hud 임당
void Game::PrintHUD()
{
    printf("\n[턴 : %d][잔여 이동 횟수 : %d][P:%d,%d]  어뢰:%d  소나:%d  지뢰:%d  폭탄:%d  적:%d  (업그레이드:%s)\n",
        gTurn, gMoveLeft, P.Height, P.Width, gAmmo.Torpedo, gAmmo.Sonar, gAmmo.MineStock, gAmmo.BombStock, gEnemyCount,
        gHasUglyPatch ? "있음" : "없음");
}

void Game::PrintMenu()
{
    printf("메뉴) 1:이동  2:어뢰발사  3:소나  4:적 스폰(디버그) 5:턴 종료  7:기뢰 발사  9:종료\n");
}

// 그냥 넣어본 개발자 기능 시간 없어서 이거밖에 못했어요 ㅠㅜ
void Game::PlaceInitialEnemy()
{
    // 간단히 적 한기 스폰
    EnemySpawn();
}
