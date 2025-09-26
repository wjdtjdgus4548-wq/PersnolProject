#include "SubMarinGame.h"
#include <iostream>
#include <ctime>

// ������
Game::Game()
{
    // �⺻ �ʱ�ȭ
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

// ���η���
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
            printf("���� ����.\n");
            break;
        }
        else if (sel == 1) // �̵�
        {
            if (gMoveLeft <= 0)
            {
                printf("���� �̵�Ƚ���� 0�Դϴ�. ���� �Ѱ��ּ���.\n");
            }
            else
            {
                int r = PlayerMove();
                if (r == -1)
                {
                    printf("���� ����.\n");
                    break;
                }
                if (r == 1)
                {
                    gMoveLeft = gMoveLeft - 1;
                    TryPickupItem();
                    if (gMoveLeft == 0)
                    {
                        printf("�̵� Ƚ�� 0! ���� �Ѱ��ּ���\n");
                    }
                }
            }

            // �÷��̾� �ൿ�� ���� �� ���� (����)
            EnemyMovePhase(1, true);
        }
        else if (sel == 2) // ���
        {
            if (gAmmo.Torpedo <= 0) {
                printf("[��� ����]\n");
            }
            else {
                int rangeX = 0;
                std::printf("��ڰ� ���� �Ÿ��� �Է��� �ּ��� (2~30): ");
                std::cin >> rangeX;
                if (!std::cin.good()) {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::printf("�߸��� �Է��Դϴ�. ��� ���.\n");
                }
                else {
                    rangeX = Clamp(rangeX, 2, 30);
                    int kills = ShotTorpedo(rangeX);
                    if (kills > 0) std::printf("[��ħ %d]\n", kills);
                    else std::printf("[��ħ 0]\n");
                    EndTurn();
                }
            }
        }
        else if (sel == 3) // �ҳ�
        {
            if (gAmmo.Sonar <= 0) {
                printf("[�ҳ� ����]\n");
            }
            else {
                ShootSonarRight();
                EndTurn();
            }
        }
        else if (sel == 4) // ����� �� ��ȯ
        {
            EnemySpawn();
        }
        else if (sel == 5) // �� ����
        {
            EndTurn();
        }
        else if (sel == 7) // ��� �߻�
        {
            if (gAmmo.MineStock <= 0) {
                printf("[��� ����]\n");
            }
            else {
                int rangeX = 0;
                std::printf("��ڰ� ���ư� �Ÿ��� �Է��� �ּ��� (2~30): ");
                std::cin >> rangeX;
                if (!std::cin.good()) {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::printf("�߸��� �Է��Դϴ�. ��� ���.\n");
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
            printf("�� �� ���� �Է��Դϴ�.\n");
        }

        // ��/�� �˻�
        int Alive = 0;
        for (int i = 0; i < gEnemyCount; i = i + 1) {
            if (gEnemies[i].Alive) Alive = Alive + 1;
        }
        if (Alive == 0 && gEnemyCount > 0) {
            printf("[��� �� ��ħ - �¸�!]\n");
            break;
        }
        if (gAmmo.Torpedo == 0 && Alive > 0) {
            printf("[��ڼ��� ���� %d ���ҽ��ϴ� - �й�]\n", Alive);
            break;
        }
        if (Alive >= 5) {
            printf("[���� 5���� �̻� ��ȯ����ϴ� - �й�]\n");
            break;
        }
    }

    printf("���͸� ������ ������ ����˴ϴ�");
    std::cin.clear();
    std::cin.ignore(1024, '\n');
    std::cin.get();
}

// �� ������
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

    // ���� / Ʈ���� / �ҳ� �׸���
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

    // ������ �׸���
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

    // ���� ǥ��
    for (int i = 0; i < gMineCount; i = i + 1) {
        if (!gMines[i].Armed) continue;
        int mh = gMines[i].H;
        int mw = gMines[i].W;
        if (mh < 0 || mh >= MapHeight || mw < 0 || mw >= MapWidth) continue;
        if (Map[mh][mw] == '#') continue;
        if (Map[mh][mw] == '*') continue;
        Map[mh][mw] = 'm';
    }

    // �ҳ� �������� (�� ǥ��)
    ApplySonarOverlay();

    // �÷��̾� ��ġ
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

// �÷��̾�
int Game::PlayerMove()
{
    char Input;
    std::printf("WASD�� �Է��� �ּ���(Q)�� ���� : ");
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
        printf("���� ���̾� ��������;;\n");
        return 0;
    }
    P.Height = NextH;
    P.Width = NextW;
    return 1;
}

// ���ϰ� �� �̵� �ϴ°�
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
        printf("[����] �� %d���� ��ħ! ��� +%d (���� %d)\n", Kills, Kills, gAmmo.Torpedo);
    }

    return Kills;
}

int Game::ShotTorpedo(int RangeX)
{
    if (gAmmo.Torpedo <= 0) {
        printf("[��� ����]\n");
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
            // �߰����� �����
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

    // ���׷��̵� ó��: �������� ����
    if (gHasUglyPatch) {
        gHasUglyPatch = false;
        gDelayBoomArmed = true;
        gDelayH = h;
        gDelayW = w;
        gDelayExplodeTurn = gTurn + 2; // ���� ��
        printf("[�������߿���] (%d,%d) -> ���� ���� ��: %d\n", gDelayH, gDelayW, gDelayExplodeTurn);
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
        printf("[��ھ���]");
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
        printf("[��� ����] (%d,%d) ���� ���� 5��\n", h, w);
    }
    else {
        printf("[�����ѵ� �ʰ� - ��ġ����]\n");
    }
    return 1;
}

int Game::ShootSonarRight()
{
    if (gAmmo.Sonar <= 0) {
        printf("[�ҳ� ����]\n");
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

    printf("�ҳ� ����!\n");
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
    if (Found) printf("���� �߰߉���ϴ�!\n");
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

    // �� �̵� �� ��� Ʈ���� �˻�
    CheckMinesTrigger();
}

// ��
void Game::StartTurn()
{
    gMoveLeft = 5;
    ClearTrail();
    ClearSonarTrail();
}

void Game::EndTurn()
{
    // �� ���幫��
    EnemyMovePhase(2, false);

    TickSonar();
    TickMines();

    // ������ ���� ����
    for (int i = 0; i < gItemCount; i = i + 1) {
        if (!gItems[i].Active) continue;
        gItems[i].Life = gItems[i].Life - 1;
        if (gItems[i].Life <= 0) gItems[i].Active = false;
    }

    // �������� ���� ó�� (�Ϲ�ȣ ���)
    gTurn = gTurn + 1;
    if (gDelayBoomArmed && gDelayExplodeTurn > 0) {
        if (gTurn >= gDelayExplodeTurn) {
            int Kills = DoExplosionAt(gDelayH, gDelayW, 2);
            printf("[��������] (%d,%d) 5x5 ����! ��ħ %d (�� %d)\n", gDelayH, gDelayW, Kills, gTurn);
            gDelayBoomArmed = false;
            gDelayExplodeTurn = -1;
            gDelayH = gDelayW = -1;
        }
        else {
            int remain = gDelayExplodeTurn - gTurn;
            if (remain < 0) remain = 0;
            printf("[�������ߴ��] (%d,%d) ���� ��: %d (���� �� %d)\n", gDelayH, gDelayW, remain, gTurn);
        }
    }

    // 5�ϸ��� �� ��ȯ
    if (gTurn % 5 == 0) {
        EnemySpawn();
    }

    StartTurn();
}

// ��� ��� �� ���� �ҳ��� ���� ���� ������µ� ���� �غ��ϱ� �ҳ� ������ ������ �÷��̰� �Ұ����ؼ� ������ ��� ����
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

// ������
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
            printf("[ȹ��] ��� +1  (���� %d)\n", gAmmo.Torpedo);
            break;
        case ITEM_SONAR:
            gAmmo.Sonar = gAmmo.Sonar + 1;
            printf("[ȹ��] �ҳ� +1  (���� %d)\n", gAmmo.Sonar);
            break;
        case ITEM_MINE:
            gAmmo.MineStock = gAmmo.MineStock + 1;
            printf("[ȹ��] ��� +1  (���� %d)\n", gAmmo.MineStock);
            break;
        case ITEM_UPGRADE:
            gHasUglyPatch = true;
            printf("[ȹ��] ��� ����(���� ��� 5��5, ��������)\n");
            printf("[�ȳ�] ����: ���� ��� �߻� �� '���� ��' ���� �� �����մϴ�.\n");
            break;
        }

        gItems[i].Active = false;
        break;
    }
}

// ���
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
        printf("[��� ����] (%d,%d) ��ħ %d\n", mh, mw, kills);
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

// hud �Ӵ�
void Game::PrintHUD()
{
    printf("\n[�� : %d][�ܿ� �̵� Ƚ�� : %d][P:%d,%d]  ���:%d  �ҳ�:%d  ����:%d  ��ź:%d  ��:%d  (���׷��̵�:%s)\n",
        gTurn, gMoveLeft, P.Height, P.Width, gAmmo.Torpedo, gAmmo.Sonar, gAmmo.MineStock, gAmmo.BombStock, gEnemyCount,
        gHasUglyPatch ? "����" : "����");
}

void Game::PrintMenu()
{
    printf("�޴�) 1:�̵�  2:��ڹ߻�  3:�ҳ�  4:�� ����(�����) 5:�� ����  7:��� �߻�  9:����\n");
}

// �׳� �־ ������ ��� �ð� ��� �̰Źۿ� ���߾�� �Ф�
void Game::PlaceInitialEnemy()
{
    // ������ �� �ѱ� ����
    EnemySpawn();
}
