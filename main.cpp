//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include "stdio.h"


// ������ ������ ����  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//����� � ������� ������ 
    bool active;
} sprite;

sprite racket;//������� ������
sprite ball;//�����
const int brickCountX = 20;
const int brickCountY = 5;
sprite brick[brickCountX][brickCountY];//�������


struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

//c����� ����

void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    racket.width = 300;
    racket.height = 50;
    racket.speed = 50;//�������� ����������� �������
    racket.x = window.width / 2.;//������� ���������� ����
    racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������f

    ball.dy = (rand() % 65 + 35) / 100.;//��������� ������ ������ ������
    ball.dx = (1 - ball.dy);//��������� ������ ������ ������
    ball.speed = 100;
    ball.rad = 20;
    ball.x = racket.x;//x ���������� ������ - �� ������� �������
    ball.y = racket.y - ball.rad;//����� ����� ������ �������

    game.score = 0;
    game.balls = 9;

    for (int y = 0; y < brickCountY; y++)
    {
        for (int x = 0; x < brickCountX; x++)
        {
            brick[x][y].hBitmap = racket.hBitmap;
            brick[x][y].width = 120;
            brick[x][y].x = x * brick[x][y].width;
            brick[x][y].height = 80;
            brick[x][y].y = y * brick[x][y].height;
            brick[x][y].active = true;
            if (y < 3)
                brick[x][y].active = false;
        }
    }

}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScore() //ShowScore - �����.
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
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

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������
    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// �����

    //ShowBitmap(window.context, ball.x - ball.rad+ball.dx*ball.speed, ball.y - ball.rad+ball.dy*ball.speed, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// �����

    for (int y = 0; y < brickCountY; y++)
    {
        for (int x = 0; x < brickCountX; x++)
        {
            if (brick[x][y].active) //��������� ��������?
            {
                ShowBitmap(window.context, brick[x][y].x, brick[x][y].y, brick[x][y].width, brick[x][y].height, brick[x][y].hBitmap);
            }
        }
    }
}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
    racket.x = min(racket.x, window.width - racket.width / 2.);//���������� ��� ������� ����
}

void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//����� ������� ����� ������� - ����������� �������
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//����� �����, � �� �� � ������ ��������� ������
        {
            game.score++;//�� ������ ������� ���� ���� ����
            ball.speed += 5. / game.score;//�� ����������� ��������� - ���������� �������� ������
            ball.dy *= -1;//������
            racket.width -= 10. / game.score;//������������� ��������� ������ ������� - ��� ���������
            ProcessSound("bounce.wav");//������ ���� �������
        }
        else
        {//����� �� �����

            tail = true;//����� ������ ������ ���� �������

            if (ball.y - ball.rad > window.height)//���� ����� ���� �� ������� ����
            {
                game.balls--;//��������� ���������� "������"

                ProcessSound("fail.wav");//������ ����

                if (game.balls < 0) { //�������� ������� ��������� "������"

                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//������� ��������� � ���������
                    InitGame();//������������������ ����
                }

                ball.dy = -(rand() % 65 + 35) / 100.;//������ ����� ��������� ������ ��� ������
                ball.dx = (1 - ball.dy);
                ball.x = racket.x;//�������������� ���������� ������ - ������ ��� �� �������
                ball.y = racket.y - ball.rad;
                game.action = false;//���������������� ����, ���� ����� �� ������ ������
                tail = false;
            }
        }
    }
}

void clearScreen()
{
    RECT rect;
    GetClientRect(window.hWnd, &rect);
    auto blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(window.context, &rect, blackBrush);
    DeleteObject(blackBrush);
}
struct point2d
{
    float x;
    float y;
    float z;
};

point2d Line[] = {-1,1,1, 1,1,1, 1,-1,1, -1,-1, 1, -1,1,-1, 1,1,-1, 1,-1,-1, -1,-1,-1};

struct index
{
    int i[2];
};

index List[] = { 0,1,1,2,2,3,3,0,0,4,4,5,5,6,6,7,7,3,3,2,2,6,6,5,5,1,1,0,0,4,4,7 };

void DrawLine(point2d p1, point2d p2)
{
    MoveToEx(window.context, p1.x*50 + window.width / 2, p1.y*50 + window.height / 2 , NULL);
    LineTo(window.context, p2.x*50 + window.width / 2 , p2.y*50 + window.height / 2 );
}

void turnZ(point2d &p, float alpha)
{
    float x = p.x * cos(alpha) - p.y * sin(alpha);
    float y = p.x * sin(alpha) + p.y * cos(alpha);

    p.x = x;
    p.y = y;
}

void turnY(point2d& p, float alpha)
{
    float x = p.x * cos(alpha) - p.z * sin(alpha);
    float z = p.x * sin(alpha) + p.z * cos(alpha);

    p.x = x;
    p.z = z;
}

void turnX(point2d& p, float alpha)
{
    float y = p.y * cos(alpha) - p.z * sin(alpha);
    float z = p.y * sin(alpha) + p.z * cos(alpha);

    p.y = y;
    p.z = z;
}
void Marker()
{
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

    SelectObject(window.context, pen);
    clearScreen();
    
    float alpha = timeGetTime() * .001;

    for (int i = 0;i < 16; i++)
    {
        int ind1 = List[i].i[0];
        int ind2 = List[i].i[1];

        point2d p1 = Line[ind1];
        point2d p2 = Line[ind2];
        turnX(p1, alpha);
        turnX(p2, alpha);

        DrawLine(p1, p2);
    }
    DeleteObject(pen);
}


bool collision;

void ProcessRoom()
{
    collision = false;

    //������������ �����, ������� � ���. ������� - ���� ������� ����� ���� ���������, � ������, ��� ������� �� ����� ������ ������������� ����� ������� �������� ������
    CheckWalls();
    CheckRoof();
    CheckFloor();
    // ��������� ������������ � ���������

    float curX = ball.x;
    float curY = ball.y;
    float curDX = ball.dx;
    float curDY = ball.dy;
    float tempY;
    float tempX;
    float br = ball.rad;
    float bdx = curDX * ball.speed;
    float bdy = curDY * ball.speed;
    int lenght = sqrt(bdx * bdx + bdy * bdy) / 2;
    bool n = false;
    for (int i = 0; i < lenght; i++)
    {


        float s = (float)i / (float)lenght;
        int n = 0;

        float strt = atan2(curDY, curDX) * (180. / 3.141519) - 90;
        //float angle = atan2(curDY, curDX) * (180. / 3.141519);
        //float strt = angle;
        for (float angle = strt;angle <= strt + 180;angle += 10)
        {

            for (int y = 0; y < brickCountY; y++)
            {
                for (int x = 0; x < brickCountX; x++)
                {

                    if (brick[x][y].active) // ���� ������ �������
                    {

                        tempX = curX + curDX * ball.speed * s;
                        tempY = curY + curDY * ball.speed * s;

                        float bx = tempX + br * cos(angle * 3.141519 / 180.);
                        float by = tempY + br * sin(angle * 3.141519 / 180.);
                        SetPixel(window.context, bx, by, RGB(255, 255, 255));

                        // ��������� ������������ � ��������
                        if (bx >= brick[x][y].x && bx < brick[x][y].x + brick[x][y].width &&
                            by >= brick[x][y].y && by < brick[x][y].y + brick[x][y].height)
                        {

                            collision = true;
                            // ����������, � ����� ������ ������� ��������� ������������
                            float overlapLeft = tempX - brick[x][y].x; // ���������� �� ����� ����� �������
                            float overlapRight = brick[x][y].x + brick[x][y].width - tempX; // ���������� �� ������ ����� �������
                            float overlapTop = tempY - brick[x][y].y; // ���������� �� ������� ����� �������
                            float overlapBottom = brick[x][y].y + brick[x][y].height - tempY; // ���������� �� ������ ����� �������
                            float edge;
                            float minV = min(overlapTop, overlapBottom);
                            float minH = min(overlapLeft, overlapRight);

                            if (minH < minV)
                            {
                                edge = -1.f;
                                curX += 2 * (tempX - curX);
                            }
                            if (minH > minV)
                            {
                                edge = 1.f;
                                curY += 2 * (tempY - curY);
                            }


                            curDX *= edge;
                            curDY *= -edge;

                            angle = strt + 180;
                            strt = atan2(curDY, curDX) * (180. / 3.141519) - 90;
                            brick[x][y].active = false; // ������������ ������
                            game.score++; // ����������� ����
                            ProcessSound("bounce.wav"); // ����������� ����
                            n = false;

                        }
                    }
                }
            }
        }
    }


    if (collision)
    {

        ball.x = tempX;
        ball.y = tempY;
        ball.dx = curDX;
        ball.dy = curDY;
    }
}



void ProcessBall()
{
    if (game.action)
    {
        if (collision) return;
        //���� ���� � �������� ������ - ���������� �����
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //����� - ����� "��������" � �������
        ball.x = racket.x;
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(window.hWnd, &p);
        //ball.x = p.x;
        //ball.y = p.y;
        //ShowRacketAndBall();//������ ���, ������� � �����
        //ShowScore();//������ ���� � �����
        //ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
        //ProcessBall();//���������� �����
        Marker();

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

//        ProcessInput();//����� ����������
  //      LimitRacket();//���������, ����� ������� �� ������� �� �����

    }

}