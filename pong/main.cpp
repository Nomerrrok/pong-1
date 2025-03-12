//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include "stdio.h"
#include <vector>
#include <tuple>
#include <algorithm>


// секция данных игры  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
    bool active;
} sprite;

sprite racket;//ракетка игрока
sprite ball;//шарик
const int brickCountX = 12;
const int brickCountY = 2;
sprite brick [brickCountX][brickCountY] ;//кирпичи


struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    racket.width = 300;
    racket.height = 50;
    racket.speed = 50;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракеткиf

    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = (1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 100;
    ball.rad = 20;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;

    for (int y = 0; y < brickCountY; y++)
    {
        for (int x = 0; x < brickCountX; x++)
        {
            brick[x][y].hBitmap = racket.hBitmap;
            brick[x][y].width = 100;
            brick[x][y].x = x * brick[x][y].width;
            brick[x][y].height = 40;
            brick[x][y].y = y * brick[x][y].height*10 + window.height/3;
            brick[x][y].active = true;

        }
    }

}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore() //ShowScore - метод.
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");

    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик

    //ShowBitmap(window.context, ball.x - ball.rad+ball.dx*ball.speed, ball.y - ball.rad+ball.dy*ball.speed, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик

    for (int y = 0; y < brickCountY; y++)
    {
        for (int x = 0; x < brickCountX; x++)
        {
            if (brick[x][y].active) //Отрисовка кирпичей?
            {
                ShowBitmap(window.context, brick[x][y].x, brick[x][y].y, brick[x][y].width, brick[x][y].height, brick[x][y].hBitmap);
            }
        }
    }
}
bool ballDirectionChanged = false;
void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}

void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
        ballDirectionChanged = true;
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
        ballDirectionChanged = true;
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
        {
            game.score++;//за каждое отбитие даем одно очко
            ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
            ball.dy *= -1;//отскок
            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
            ProcessSound("bounce.wav");//играем звук отскока
            ballDirectionChanged = true;


        }
        else
        {//шарик не отбит

            tail = true;//дадим шарику упасть ниже ракетки

            if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
            {
                game.balls--;//уменьшаем количество "жизней"

                ProcessSound("fail.wav");//играем звук

                if (game.balls < 0) { //проверка условия окончания "жизней"

                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
                    InitGame();//переинициализируем игру
                }

                ball.dy = -(rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
                ball.dx = (1 - ball.dy);
                ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
                ball.y = racket.y - ball.rad;
                game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
                tail = false;
            }
        }
    }
}

struct CollisionInfo
{
    int X = -1;
    int Y = -1;
    float collisionX = -1.0f;
    float collisionY = -1.0f;
    int I = 100;
    bool collision = false;
};
CollisionInfo collisionInfo;
HPEN hPen;

void Line(int x1, int y1, int x2, int y2)
{
    if (!hPen)
    {
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); //задаём сплошную кисть, закрашенную цветом RGB

    }
    SelectObject(window.context, hPen);
    MoveToEx(window.context, x1, y1, NULL); //сделать текущими координаты x1, y1
    LineTo(window.context, x2, y2);
}
void checkBricks()
{
    if (!ballDirectionChanged) return;


    float bdx = ball.dx * ball.speed;
    float bdy = ball.dy * ball.speed;
    int length = sqrt(bdx * bdx + bdy * bdy);
    float minTime = FLT_MAX;
    float minDist = FLT_MAX;
    for (int y = 0; y < brickCountY; y++)
    {
        for (int x = 0; x < brickCountX; x++)
        {
            if (brick[x][y].active)
            {
                for (int i = 0; i < length; i++)
                {
                    float s = (float(i) / float(length))*15;
                    float tempX = ball.x + bdx * s;
                    float tempY = ball.y + bdy * s;
                    SetPixel(window.context, tempX, tempY, RGB(255, 255, 255));
                    float startAngle = atan2(ball.dy, ball.dx) * (180.0 / 3.141519) - 90;
                    if (tempX + ball.rad >= brick[x][y].x && tempX - ball.rad <= brick[x][y].x + brick[x][y].width &&
                        tempY + ball.rad >= brick[x][y].y && tempY - ball.rad <= brick[x][y].y + brick[x][y].height )
                    {
                        collisionInfo.X = x;
                        collisionInfo.Y = y;
                        collisionInfo.I = i;
                        for (float angle = startAngle; angle < startAngle + 180; angle += 10)
                        {

                            float bx = tempX + ball.rad * cos(angle * 3.141519 / 180.0);
                            float by = tempY + ball.rad * sin(angle * 3.141519 / 180.0);
                            SetPixel(window.context, bx, by, RGB(255, 255, 255));
                            if (bx+ball.rad >= brick[x][y].x && bx - ball.rad <= brick[x][y].x + brick[x][y].width &&
                                by+ball.rad >= brick[x][y].y && by - ball.rad <= brick[x][y].y + brick[x][y].height)
                            {
                                collisionInfo.collisionX = bx;
                                collisionInfo.collisionY = by;

                                ballDirectionChanged == false;

                            }
                        }
                    }
                }
            }
        }
    }
}


void handleCollision(int brickX, int brickY, float collisionX, float collisionY, int I)
{


    float bdx = ball.dx * ball.speed;
    float bdy = ball.dy * ball.speed;
    int length = sqrt(bdx * ball.speed + ball.speed * bdy);

    if (sqrt((collisionX - ball.x) * (collisionX - ball.x) + (collisionY - ball.y) * (collisionY - ball.y)) <= length)
    {

        brick[brickX][brickY].active = false;
        game.score++; // Увеличиваем счет
        ProcessSound("bounce.wav");
        float overlapLeft = collisionX - brick[brickX][brickY].x;
        float overlapRight = brick[brickX][brickY].x + brick[brickX][brickY].width - collisionX;
        float overlapTop = collisionY - brick[brickX][brickY].y;
        float overlapBottom = brick[brickX][brickY].y + brick[brickX][brickY].height - collisionY;

        float minV = min(overlapTop, overlapBottom);
        float minH = min(overlapLeft, overlapRight);

        float edge;

        if (minH < minV)
        {
            edge = -1.0f;
            if (overlapLeft < overlapRight)
            {
                ball.y = collisionX;
                ball.x = collisionX + ball.rad;
            }
            else
            {
                ball.y = collisionY;
                ball.x = collisionX - ball.rad;
            }
        }
        else
        {
            edge = 1.0f;
            if (overlapTop < overlapBottom)
            {
                ball.x = collisionX;
                ball.y = collisionY - ball.rad;
            }
            else
            {
                ball.x = collisionX;
                ball.y = collisionY - ball.rad;
            }
        }

        ball.dx *= edge;
        ball.dy *= -edge;
        collisionInfo.X = -1;
        collisionInfo.Y = -1;
        collisionInfo.collisionX = -1.0f;
        collisionInfo.collisionY = -1.0f;
        collisionInfo.I = 100;
        ballDirectionChanged = true;
    }
}



POINT p;

void ProcessRoom()
{
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
    CheckWalls();
    CheckRoof();
    CheckFloor();
    checkBricks();
    ballDirectionChanged == false;
    handleCollision(collisionInfo.X, collisionInfo.Y, collisionInfo.collisionX, collisionInfo.collisionY, collisionInfo.I);

    ScreenToClient(window.hWnd, &p);
    GetCursorPos(&p);
    //bool ballDirectionChanged = false;

    
   // ball.x = p.x;
  // ball.y = p.y;
    
}

void ProcessBall()
{
    if (game.action)
    {
        //если игра в активном режиме - перемещаем шарик
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //иначе - шарик "приклеен" к ракетке
        ball.x = racket.x;
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

   // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
        ProcessBall();//перемещаем шарик

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        while (!GetAsyncKeyState(VK_RETURN))
        {
           Sleep(56);
        }
 
        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        
    }

}
