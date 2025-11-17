#include <iostream>
#include <windows.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <conio.h>
#include <iostream>

using namespace std;

//размер кл в симвл
const int cell_width = 8;
const int cell_height = 5;

//функц для разных полей
void SetFieldSize2x2();
void SetFieldSize4x4();
void SetFieldSize8x8();
int ShowMainMenu();

//размеры для разных режимов
struct FieldSize {
    int scr_width;
    int scr_height;
    int fld_width;
    int fld_height;
};

FieldSize currentSize = { 80, 25, 4, 4 }; // По умолчанию 4x4

typedef char TScreenMap[50][120]; // Максимальный размер для 8x8

//позиция курсора
void SetCurPos(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class TScreen {
public:
    TScreenMap scr;  //хранит массив символов на экране
    TScreen() { Clear(); }
    void Clear() {
        memset(scr, ' ', sizeof(scr));
        for (int i = 0; i < currentSize.scr_height; i++) {
            scr[i][currentSize.scr_width - 1] = '\0';
        }
    }
    void Show() {
        SetCurPos(0, 0);
        //вывод кж стрк отдельно
        for (int i = 0; i < currentSize.scr_height; i++) {
            cout << scr[i] << endl;
        }
    }
};

//клетки
class TCell {
public:
    int value;
    POINT pos;
    TCell() { Init(0, 0, 0); }
    void Init(int x, int y, int val) {
        value = val;
        pos.x = x;
        pos.y = y;
    }
    void Put(TScreenMap scr);
};

//анимация кл
class TAnimatedCell : public TCell {
    float ax, ay;
    float dx, dy;
    int aCnt;
    int faceVal;
public:
    TAnimatedCell() : TCell() { aCnt = 0; }
    void Anim(POINT to);
    bool IsAnim() { return aCnt > 0; }
    bool IsStat() { return (dx == 0 && dy == 0) ? true : false; }
    void Work() {
        if (aCnt > 0) {
            aCnt--;
            ax += dx;
            ay += dy;
        }
    };
    void PutAnim(TScreenMap scr);
    void PutStat(TScreenMap sct);
};


/*  можнго убрать\\  дебаг ошибки ранд чисел при запуске
void TGame::GenNewRandNum()
{
    cout << "свобод кл: " << GetFreeCellCnt() << endl; //Д.
    if (GetFreeCellCnt() == 0) return;

    int cnt = 1;
    while (cnt > 0)
    {
        int x = rand() % fld_width;
        int y = rand() % fld_height;
        cout << "П.К [" << y << "][" << x << "] = " << cell[y][x].value << endl; //Д.

        if (cell[y][x].value == 0) {
            cell[y][x].value = (rand() % 11 == 0) ? 4 : 2;
            cout << "установ кл: " << cell[y][x].value << endl; //Д.
            cnt--;
        }
    }
}
*/



// Базовый класс игры
class TGame {
protected:
    TScreen screen;
    virtual bool MoveValInArray(TAnimatedCell* valArr[], int cnt) = 0;
    virtual void Move(int dx, int dy) = 0;
    virtual void GenNewRandNum(bool anim = false) = 0;
    virtual int GetFreeCellCnt() = 0;
    virtual bool CheckEndGame() = 0;
public:
    virtual ~TGame() {}
    virtual void Init() = 0;
    virtual void Work() = 0;
    virtual void Show() = 0;
};

//отрис буфер вывод кл
void TAnimatedCell::PutStat(TScreenMap scr)
{
    if (IsAnim())
    {
        TCell cell;
        if (IsStat())
            cell.Init(pos.x, pos.y, faceVal);
        else
            cell.Init(pos.x, pos.y, 0);
        cell.Put(scr);
    }
    else
        Put(scr);
}

//отображ аним
void TAnimatedCell::PutAnim(TScreenMap scr)
{
    if (IsAnim())
    {
        Work();
        if (IsStat()) return;
        TCell cell;
        cell.Init(lround(ax), lround(ay), faceVal);
        cell.Put(scr);
    }
};

//аним механика сдвига
void TAnimatedCell::Anim(POINT to)
{
    if (IsAnim()) return;
    faceVal = value;
    aCnt = 5;
    ax = pos.x;
    ay = pos.y;
    dx = (to.x - ax) / (float)aCnt,
        dy = (to.y - ay) / (float)aCnt;
}

void TCell::Put(TScreenMap scr)
{
    for (int i = 0; i < cell_width; i++) {
        for (int j = 0; j < cell_height; j++) {
            //отрисовка границ клетки
            if (i == 0 || i == cell_width - 1 || j == 0 || j == cell_height - 1) {
                scr[pos.y + j][pos.x + i] = '`';
            }
            else {
                scr[pos.y + j][pos.x + i] = ' ';
            }
        }
    }

    if (value == 0) return;

    char buf[10];
    sprintf_s(buf, sizeof(buf), "%d", value);
    int len = strlen(buf);

    //вычисление позиции для числа
    int posX = pos.x + (cell_width - len) / 2;
    int posY = pos.y + cell_height / 2;

    //запись числа в центр клетки
    for (int i = 0; i < len; i++) {
        scr[posY][posX + i] = buf[i];
    }
}

//временн тест ==убрать потом
bool KeyDownOnce(char c)
{
    if (GetKeyState(c) < 0)
    {
        while (GetKeyState(c) < 0);
        return true;
    }
    return false;
}

//фнц передвиж клеток друг в другг
bool MoveValInArray(TAnimatedCell* valArr[], int cnt)
{
    bool moved = false;
    int lastX = 0;
    for (int i = 1; i < cnt; i++)
        if (valArr[i]->value != 0)
        {
            if (valArr[lastX]->value == 0)
            {
                valArr[i]->Anim(valArr[lastX]->pos);
                valArr[lastX]->Anim(valArr[lastX]->pos);
                moved = true;
                valArr[lastX]->value = valArr[i]->value;
                valArr[i]->value = 0; //сброс
            }
            else //знач равн
                if (valArr[lastX]->value == valArr[i]->value)
                {
                    valArr[i]->Anim(valArr[lastX]->pos);
                    valArr[lastX]->Anim(valArr[lastX]->pos);
                    moved = true;
                    valArr[lastX]->value += valArr[i]->value;
                    valArr[i]->value = 0;
                    lastX++;
                }
                else //знвя не рав
                    if (valArr[lastX]->value != valArr[i]->value)
                    {
                        lastX++;
                        if (lastX != i)
                        {
                            valArr[i]->Anim(valArr[lastX]->pos);
                            valArr[lastX]->Anim(valArr[lastX]->pos);
                            moved = true;
                            valArr[lastX]->value = valArr[i]->value;
                            valArr[i]->value = 0;
                        }
                    }

        }
    return moved;
}

//реализация для 2x2
class TGame2x2 : public TGame {
private:
    TAnimatedCell cell[2][2];
    bool MoveValInArray(TAnimatedCell* valArr[], int cnt);
    void Move(int dx, int dy);
    void GenNewRandNum(bool anim = false);
    int GetFreeCellCnt();
    bool CheckEndGame();
public:
    TGame2x2() { Init(); }
    void Init();
    void Work();
    void Show();
};

bool TGame2x2::CheckEndGame() //проверка конца игры
{
    if (GetFreeCellCnt() > 0)
        return false;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            // проверка сосед кл снизу (по вертикали)
            if (i < 1 && cell[i][j].value == cell[i + 1][j].value)
                return false;
            // проверка сосед кл справа (по горизонтали)  
            if (j < 1 && cell[i][j].value == cell[i][j + 1].value)
                return false;
        }
    }
    return true;
}

void TGame2x2::GenNewRandNum(bool anim) //созд новые случ цф в кл
{
    if (GetFreeCellCnt() == 0) return;

    int cnt = 1;
    while (cnt > 0)
    {
        int x = rand() % 2;
        int y = rand() % 2;

        if (cell[y][x].value == 0)
        {
            if (anim)
                cell[y][x].Anim(cell[y][x].pos);
            cell[y][x].value = (rand() % 11 == 0) ? 4 : 2;
            cnt--;
        }
    }
}

int TGame2x2::GetFreeCellCnt() //возв кол-во пустых кл
{
    int cnt = 0;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            if (cell[i][j].value == 0)
                cnt++;
    return cnt;
}

void TGame2x2::Work()
{
    if (KeyDownOnce('W')) Move(0, -1);
    if (KeyDownOnce('S')) Move(0, 1);
    if (KeyDownOnce('A')) Move(-1, 0);
    if (KeyDownOnce('D')) Move(1, 0);
}

void TGame2x2::Move(int dx, int dy)
{
    bool moved = false;
    if (dx != 0) //горизонталь сдвиг
        for (int j = 0; j < 2; j++)
        {
            TAnimatedCell* valArr[2];
            for (int i = 0; i < 2; i++)
            {
                int x = (dx < 0) ? i : 2 - i - 1;
                valArr[i] = &cell[j][x];
            }
            if (MoveValInArray(valArr, 2)) moved = true;
        }
    if (dy != 0) //вертикаль сдвиг
        for (int i = 0; i < 2; i++)
        {
            TAnimatedCell* valArr[2];
            for (int j = 0; j < 2; j++)
            {
                int y = (dy < 0) ? j : 2 - j - 1;
                valArr[j] = &cell[y][i];
            }
            if (MoveValInArray(valArr, 2)) moved = true;
        }
    if (CheckEndGame())
    {
        //отрисовка экрана прг
        screen.Clear();
        cout << "GAME OVER!\n" << endl;
        cout << "|| PARAVOZICK GAMES ||" << endl;
        //паравозик интертейментен.ооо представляет

        screen.Show();
        _getch();

        Sleep(3000);
        cout << "\n\n\n\n\n\n\n\n\n\n\n\n" << endl;

        Init(); // перезапуск
    }
    else if (moved) //добавляет при каждом ходе ранд число
    {
        GenNewRandNum(true);
    }
}

bool TGame2x2::MoveValInArray(TAnimatedCell* valArr[], int cnt)
{
    return ::MoveValInArray(valArr, cnt);
}

void TGame2x2::Show()
{
    //рис обч кл
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (!cell[i][j].IsAnim()) {
                cell[i][j].PutStat(screen.scr);
            }
        }
    }

    //рис аним кл поверх
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (cell[i][j].IsAnim()) {
                cell[i][j].PutAnim(screen.scr);
            }
        }
    }

    screen.Show();
}

void TGame2x2::Init()
{
    const int dx = 2;
    const int dy = 2;
    srand(GetTickCount());
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            cell[j][i].Init(dx + i * (cell_width - 1), dy + j * (cell_height - 1), 0);
    GenNewRandNum();
    GenNewRandNum();
}

/*o5HwKFdPKqlP6Gj2QRIJOx7SsNACFk2L6rQENBYNxEYeJfbkab2P5L714QpfpaQ1KqnQ2ACYBBk5
batcQQgiL8711gaQWTlsD3E34zULodpWndLSQ9QE8DTSM9MjLjviRReJ0qCXCOTEogqKGgvjCTp4
ZiY + MZgK2yPZSF0jrcx16ZwjDTXJjVSAaz + OPeNIA / FHY5AzgMHY7RaLjV9WpWLcdZRm1LqthM31
9i6E4Yjqk6pghdR1aJmeNflbReDI0K0Zgv4Nh7uRqkHCiWCXGeRlLRIDYqVaiu2HcDt5bAi4ibmm
cSsK / MEngo7wYE6cg36nOY4vT2Id8Vf156Yc + sYg2AVWYr + 09SahEcPuKs4nsSNbS8JpAGxNnwDH
CkGwLlT + NrX9Tz / w0 + u4WzNVLZ4HFaRfEFZu4hPMaz68GHd6eQ4sVPbKch38yoEfeI7E*/

//реализация для 4x4
class TGame4x4 : public TGame {
private:
    TAnimatedCell cell[4][4];
    bool MoveValInArray(TAnimatedCell* valArr[], int cnt);
    void Move(int dx, int dy);
    void GenNewRandNum(bool anim = false);
    int GetFreeCellCnt();
    bool CheckEndGame();
public:
    TGame4x4() { Init(); }
    void Init();
    void Work();
    void Show();
};

bool TGame4x4::CheckEndGame() //проверка конца игры
{
    if (GetFreeCellCnt() > 0)
        return false;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // проверка сосед кл снизу (по вертикали)
            if (i < 3 && cell[i][j].value == cell[i + 1][j].value)
                return false;
            // проверка сосед кл справа (по горизонтали)  
            if (j < 3 && cell[i][j].value == cell[i][j + 1].value)
                return false;
        }
    }
    return true;
}

void TGame4x4::GenNewRandNum(bool anim) //созд новые случ цф в кл
{
    if (GetFreeCellCnt() == 0) return;

    int cnt = 1;
    while (cnt > 0)
    {
        int x = rand() % 4;
        int y = rand() % 4;

        if (cell[y][x].value == 0)
        {
            if (anim)
                cell[y][x].Anim(cell[y][x].pos);
            cell[y][x].value = (rand() % 11 == 0) ? 4 : 2;
            cnt--;
        }
    }
}

int TGame4x4::GetFreeCellCnt() //возв кол-во пустых кл
{
    int cnt = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (cell[i][j].value == 0)
                cnt++;
    return cnt;
}

void TGame4x4::Work()
{
    if (KeyDownOnce('W')) Move(0, -1);
    if (KeyDownOnce('S')) Move(0, 1);
    if (KeyDownOnce('A')) Move(-1, 0);
    if (KeyDownOnce('D')) Move(1, 0);
}

void TGame4x4::Move(int dx, int dy)
{
    bool moved = false;
    if (dx != 0) //горизонталь сдвиг
        for (int j = 0; j < 4; j++)
        {
            TAnimatedCell* valArr[4];
            for (int i = 0; i < 4; i++)
            {
                int x = (dx < 0) ? i : 4 - i - 1;
                valArr[i] = &cell[j][x];
            }
            if (MoveValInArray(valArr, 4)) moved = true;
        }
    if (dy != 0) //вертикаль сдвиг
        for (int i = 0; i < 4; i++)
        {
            TAnimatedCell* valArr[4];
            for (int j = 0; j < 4; j++)
            {
                int y = (dy < 0) ? j : 4 - j - 1;
                valArr[j] = &cell[y][i];
            }
            if (MoveValInArray(valArr, 4)) moved = true;
        }
    if (CheckEndGame())
    {
        //отрисовка экрана прг
        screen.Clear();
        cout << "GAME OVER!\n" << endl;
        cout << "|| PARAVOZICK GAMES ||" << endl;
        //паравозик интертейментен.ооо представляет

        screen.Show();
        _getch();

        Sleep(3000);
        cout << "\n\n\n\n\n\n\n\n\n\n\n\n" << endl;

        Init(); // перезапуск
    }
    else if (moved) //добавляет при каждом ходе ранд число
    {
        GenNewRandNum(true);
    }
}

bool TGame4x4::MoveValInArray(TAnimatedCell* valArr[], int cnt)
{
    return ::MoveValInArray(valArr, cnt);
}

void TGame4x4::Show()
{
    //рис обч кл
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (!cell[i][j].IsAnim()) {
                cell[i][j].PutStat(screen.scr);
            }
        }
    }

    //рис аним кл поверх
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (cell[i][j].IsAnim()) {
                cell[i][j].PutAnim(screen.scr);
            }
        }
    }

    screen.Show();
}

void TGame4x4::Init()
{
    const int dx = 2;
    const int dy = 2;
    srand(GetTickCount());
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cell[j][i].Init(dx + i * (cell_width - 1), dy + j * (cell_height - 1), 0);
    GenNewRandNum();
    GenNewRandNum();
}

// Реализация для 8x8
class TGame8x8 : public TGame {
private:
    TAnimatedCell cell[8][8];
    bool MoveValInArray(TAnimatedCell* valArr[], int cnt);
    void Move(int dx, int dy);
    void GenNewRandNum(bool anim = false);
    int GetFreeCellCnt();
    bool CheckEndGame();
public:
    TGame8x8() { Init(); }
    void Init();
    void Work();
    void Show();
};

bool TGame8x8::CheckEndGame() //проверка конца игры
{
    if (GetFreeCellCnt() > 0)
        return false;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // проверка сосед кл снизу (по вертикали)
            if (i < 7 && cell[i][j].value == cell[i + 1][j].value)
                return false;
            // проверка сосед кл справа (по горизонтали)  
            if (j < 7 && cell[i][j].value == cell[i][j + 1].value)
                return false;
        }
    }
    return true;
}

void TGame8x8::GenNewRandNum(bool anim) //созд новые случ цф в кл
{
    if (GetFreeCellCnt() == 0) return;

    int cnt = 1;
    while (cnt > 0)
    {
        int x = rand() % 8;
        int y = rand() % 8;

        if (cell[y][x].value == 0)
        {
            if (anim)
                cell[y][x].Anim(cell[y][x].pos);
            cell[y][x].value = (rand() % 11 == 0) ? 4 : 2;
            cnt--;
        }
    }
}

int TGame8x8::GetFreeCellCnt() //возв кол-во пустых кл
{
    int cnt = 0;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (cell[i][j].value == 0)
                cnt++;
    return cnt;
}

void TGame8x8::Work()
{
    if (KeyDownOnce('W')) Move(0, -1);
    if (KeyDownOnce('S')) Move(0, 1);
    if (KeyDownOnce('A')) Move(-1, 0);
    if (KeyDownOnce('D')) Move(1, 0);
}

void TGame8x8::Move(int dx, int dy)
{
    bool moved = false;
    if (dx != 0) //горизонталь сдвиг
        for (int j = 0; j < 8; j++)
        {
            TAnimatedCell* valArr[8];
            for (int i = 0; i < 8; i++)
            {
                int x = (dx < 0) ? i : 8 - i - 1;
                valArr[i] = &cell[j][x];
            }
            if (MoveValInArray(valArr, 8)) moved = true;
        }
    if (dy != 0) //вертикаль сдвиг
        for (int i = 0; i < 8; i++)
        {
            TAnimatedCell* valArr[8];
            for (int j = 0; j < 8; j++)
            {
                int y = (dy < 0) ? j : 8 - j - 1;
                valArr[j] = &cell[y][i];
            }
            if (MoveValInArray(valArr, 8)) moved = true;
        }
    if (CheckEndGame())
    {
        //отрисовка экрана прг
        screen.Clear();
        cout << "GAME OVER!\n" << endl;
        cout << "|| PARAVOZICK GAMES ||" << endl;
        //паравозик интертейментен.ооо представляет

        screen.Show();
        _getch();

        Sleep(3000);
        cout << "\n\n\n\n\n\n\n\n\n\n\n\n" << endl;

        Init(); // перезапуск
    }
    else if (moved) //добавляет при каждом ходе ранд число
    {
        GenNewRandNum(true);
    }
}

bool TGame8x8::MoveValInArray(TAnimatedCell* valArr[], int cnt)
{
    return ::MoveValInArray(valArr, cnt);
}

void TGame8x8::Show()
{
    static int frameCount = 0;
    frameCount++;

    //настр отрисовки фпс(через каждые * кадров)
    if (frameCount % 2 == 0) // сейчас рис только каждый 2-й кадр
    {
        //рис обч кл
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (!cell[i][j].IsAnim()) {
                    cell[i][j].PutStat(screen.scr);
                }
            }
        }

        //рис аним кл поверх
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (cell[i][j].IsAnim()) {
                    cell[i][j].PutAnim(screen.scr);
                }
            }
        }

        screen.Show();
    }
}

void TGame8x8::Init()
{
    const int dx = 2;
    const int dy = 2;
    srand(GetTickCount());
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            cell[j][i].Init(dx + i * (cell_width - 1), dy + j * (cell_height - 1), 0);
    GenNewRandNum();
    GenNewRandNum();
}

// Функции для установки размеров поля
void SetFieldSize2x2()
{
    currentSize = { 40, 20, 2, 2 };
    system("mode con cols=40 lines=20");
}

void SetFieldSize4x4()
{
    currentSize = { 80, 25, 4, 4 };
    system("mode con cols=80 lines=25");
}

void SetFieldSize8x8()
{
    currentSize = { 120, 40, 8, 8 };
    system("mode con cols=120 lines=40");
}

// Функция отображения главного меню
int ShowMainMenu()
{
    system("mode con cols=60 lines=20");
    system("cls");
    setlocale(LC_ALL, "RU");


    cout << "===================================================" << endl;

    cout << "              2048 - PARAVOZICK GAMES        " << endl;

    cout << "===================================================" << endl;

    cout << endl;

    cout << "         ВЫБЕРИТЕ РАЗМЕР ПОЛЯ:" << endl;

    cout << endl;

    cout << "         1. 2x2 (ЛЕГКИЙ)" << endl;

    cout << "         2. 4x4 (СТАНДАРТНЫЙ)" << endl;

    cout << "         3. 8x8 (СЛОЖНЫЙ) |медленнее|" << endl;

    cout << endl;

    cout << "         0. ВЫХОД" << endl;

    cout << endl;

    cout << "===================================================" << endl;

    cout << "  Для входа введите номер подменю (указан слево)\n"; 

    cout << "!при возникновении визуальных багов увеличьте окно!\n"; 

    cout << "      !!или зайдите в полноэкранный режим!!\n"; 

    cout << endl;

    cout << "Чтобы вернуться из режимов игры в меню нажмите ESC\n";

    cout << "===================================================" << endl;

    cout << "    Ваш выбор: ";



    int choice;
    cin >> choice;
    return choice;
}

void ShowLogo() {


    cout << "OOkkxxddollccccccccccccccccccccccccccccllloooooooooooooooooooooollllllcccccccc:::::::::;;;;;;;;;;;;;" << endl;


    cout << "kkxddollc:;;;;,,,,,,,,,,,,,,''''',',',,;;::cccclllllloooodoooooddoooooooooooooooooooolllllc;;;;;;;,," << endl

        ;
    cout << ",,''.............................',''',;,''',,,;;;;:::cccllllooooodooooooooooooooolllccloddlcccccll;" << endl;


    cout << ".','''...................''''''''''';lddc..............'''''',,,,:l;,,,;;;;;::::::::::::::::;;;:,,," << endl;


    cout << ".,;,,,''................'lddddxo'...,okk:........... ....    ... ...                         ..c:..." << endl;


    cout << ".';,,,,,'.....   .......,xOOOO0x'...,xOk:... .;llllllc;.     ..  ..                           .l:..." << endl;


    cout << ".....',,......    ... ...''''''.....;k0O:....,coxkkxxdo,.    ..  ..                           ,o,..." << endl;


    cout << "..,;;;;,'.'...        .      .......;ool;',,,,,',;;;;:l:..  ..  ..                          .:o'..." << endl;


    cout << "'.':::,',',;,,''''''.........''''''...............'',,;lddc....  .                           .ll...." << endl;


    cout << ",..;;,,,;:cccccccccccc:::::;;,,,'.......................':lc;... .                           .lc...." << endl;


    cout << "'..,,'..'::;clc::;;;::::::;;,'''''........  .......'......':do'.            .               .,;'...," << endl;


    cout << ":'.......',::;,,',,,,,,,,,,,''''''''.....  .      ..  ....',;cl,..       ...';.            ......''," << endl;


    cout << ":;.  .....'''......................'....  .    .  .   ...;c;;,cdo;..,,;;;;;'.::.           ..''..'.." << endl;


    cout << ",,.    ................................    .   .    ..  ..';::okkxolc;'..... .c:........  .';,......" << endl;


    cout << ",,'.     .   ....................... .... ..... .....',,...'..,cxkxl::;,,,;,..,l,.........,:;,'....'" << endl;


    cout << ";;,'..      .....   .'...     .......      ...',,;;;;;cl,......,ldxxdoclddl;...cc............ .....;" << endl;


    cout << ",''............................. .,;'. ..'',:c:;'..';:;'......,,;coool:cdoc;...,l;................cx" << endl;


    cout << "'',,;;,..''',,,'..',,,,,;;;;;;,. .,:;.  ,ooolcclooddol:;,,;,,;::cloollldxol:;,,;lc''''.'''.......:dO" << endl;


    cout << "llllccc:;;:ll;,,'.,:;,,,;,;;;;;'..,::,. :dcccccc:;;;;:ol,',,;;:ccllolclooooollc:::;;;;,,;,..','',lkO" << endl;


    cout << "oooolllllccclc:;;;:c:;;;;;;;;;;,..,::;. ;oc:;,'...,:ol;'..'',,;::loolcc:;;,,;;,'.....''''..':::;:ooc" << endl;


    cout << "dddodoooooollllcccccc::::::;;;;;...;;;;.;c,'..'',,,;:ol;:::::ccclodollcc:;;;,'.............;::;,;c;," << endl;


    cout << "ddddddddddoooooollllllcccccc:::::,,;;:::clllooddddddxkkxxxxxxxxxxxdlccllllllllc:,''.......'',:;,;:;'" << endl;


    cout << "xddxxddddddddddooooooollllllllc:;;:ccldxkkkOOOOOOkkkkkOkkkkxxxxxdol:::::cllllllcc:;,,'......';,,;;.." << endl;


    cout << "lxxxxxxxxxxxxdddddddddooolc:;,,,,;:c::coxO000000OOOOOOOOOOkkkkxxxddolllooooooooollllc:;,...;c;';;.  " << endl;


    cout << ".cxxxxxxxxxxxxxxxxxdddddoc;;,,,,;,,;;:coxkOKKKK00000000000OOOOOkkkkkxxxddddooooooddoolc,.'col,,;'. ." << endl;


    cout << " .:xkkkkkxxxxxxxxxxxxxxxxxdddooolccccccclddddxkOOOOkkkkkkkkkkxxxxdddddooooc;;;,;:::;;,',;coo:,,....;" << endl;


    cout << "  .,odxxkkkkkkkkkkkxxxxxxxxxxxxxxxxxddddoooooooooddddxxxxxdddoooolllllc::;,'''.........';cc;,'....,'" << endl;


    cout << "    .;;cxkkkkkkkkkkkkkkkkkkxxxxxxxxxxxxxxxxxxdddddxddddddddddddddooooollc::::;'........:od:......'.." << endl;
     

}

int main()
{
    setlocale(LC_ALL, "RU");
    system("cls");
    system("mode con cols=80 lines=20");

    SetCurPos(25, 5);
    cout << "========================================";

    SetCurPos(25, 6);
    cout << "|                                      |";

    SetCurPos(25, 7);
    cout << "|           PARAVOZICK GAMES           |";

    SetCurPos(25, 8);
    cout << "|                                      |";

    SetCurPos(25, 9);
    cout << "========================================";

    SetCurPos(35, 12);
    cout << "     ЗАГРУЗКА...\n\n\n\n\n\n";

    Sleep(3000);


    /*system("cls");
    system("mode con cols=80 lines=20");
    SetCurPos(35, 12);
    cout << "     хммм...";
    Sleep(2000);*/
        ShowLogo();
        Sleep(3000);
    
    //string logoText =
    
    

    // Вызов функции
    //PrintTextWithWrap(logoText, 10, 5, 60);  // x=10, y=5, ширина=60 символов
//Sleep(5000);
   
    while (true)
    {
        int choice = ShowMainMenu();

        TGame* game = nullptr;

        switch (choice)
        {
        case 1:
            SetFieldSize2x2();
            game = new TGame2x2();
            break;
        case 2:
            SetFieldSize4x4();
            game = new TGame4x4();
            break;
        case 3:
            SetFieldSize8x8();
            game = new TGame8x8();
            break;
        case 0:
            return 0;
        default:
            cout << "Неверный выбор! Попробуйте снова." << endl;
            Sleep(2000);
            continue;
        }

        // Запускаем игру с выбранным размером
        while (game && true)
        {
            game->Work();
            game->Show();
            if (GetKeyState(VK_ESCAPE) < 0) break;
            Sleep(0);
        }

        delete game;

        // После завершения игры возвращаемся в меню
        cout << "загрузка в меню..." << endl;
        _getch();
    }

    return 0;
}