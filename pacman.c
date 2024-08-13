#include <windows.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 40 
#define HEIGHT 22
#define PACMAN 'C' 
#define WALL '#' 
#define FOOD '.' 
#define EMPTY ' ' 
#define DEMON 'X' 

// Global Variables
int res = 0;
int score = 0;
int pacman_x, pacman_y;
char board[HEIGHT][WIDTH];
int food = 0;
int curr = 0;
POINT demons[10];  // Storing demon positions
int demon_count = 10;

// Function Declarations
void initialize();
void draw(HDC hdc);
void move(int move_x, int move_y);
void moveDemons();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Initializing the game board
void initialize() {
    // Initializing board with walls and empty spaces
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || j == WIDTH - 1 || j == 0 || i == HEIGHT - 1) {
                board[i][j] = WALL;
            } else {
                board[i][j] = EMPTY;
            }
        }
    }

    // Adding random walls
    int count = 50;
    while (count != 0) {
        int i = (rand() % HEIGHT);
        int j = (rand() % WIDTH);

        if (board[i][j] != WALL && board[i][j] != PACMAN) {
            board[i][j] = WALL;
            count--;
        }
    }

    // Adding some rows of walls
    int val = 5;
    while (val--) {
        int row = (rand() % HEIGHT);
        for (int j = 3; j < WIDTH - 3; j++) {
            if (board[row][j] != WALL && board[row][j] != PACMAN) {
                board[row][j] = WALL;
            }
        }
    }

    // Adding demons
    count = demon_count;
    while (count != 0) {
        int i = (rand() % HEIGHT);
        int j = (rand() % WIDTH);

        if (board[i][j] != WALL && board[i][j] != PACMAN) {
            board[i][j] = DEMON;
            demons[count-1].x = j;  // Store demon's x position
            demons[count-1].y = i;  // Store demon's y position

            count--;
        }
    }

    // Pacman starting position
    pacman_x = WIDTH / 2;
    pacman_y = HEIGHT / 2;
    board[pacman_y][pacman_x] = PACMAN;

    // Placing food
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i % 2 == 0 && j % 2 == 0 && board[i][j] != WALL && board[i][j] != DEMON && board[i][j] != PACMAN) {
                board[i][j] = FOOD;
                food++;
            }
        }
    }
}

// Drawing the game board using Windows GDI
void draw(HDC hdc) {
    int cell_width = 20, cell_height = 20;
    HBRUSH hBrush;
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = SelectObject(hdc, hPen);
    RECT rect;

    // background
    SetBkColor(hdc, RGB(0, 0, 0));
    HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &(RECT){0, 0, WIDTH * cell_width, HEIGHT * cell_height}, hBackgroundBrush);
    DeleteObject(hBackgroundBrush);

    // walls
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 10, 10);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = SelectObject(hMemDC, hBitmap);
    HBRUSH hTextureBrush = CreatePatternBrush(hBitmap);

    // Drawing a simple brick pattern
    RECT brickRect = {0, 0, 10, 10};
    FillRect(hMemDC, &brickRect, CreateSolidBrush(RGB(101, 67, 33))); // Brick color
    for (int i = 0; i < 10; i += 2) {
        MoveToEx(hMemDC, i, 0, NULL);
        LineTo(hMemDC, i, 10);
    }
    for (int i = 0; i < 10; i += 2) {
        MoveToEx(hMemDC, 0, i, NULL);
        LineTo(hMemDC, 10, i);
    }
    SelectObject(hMemDC, oldBitmap);
    DeleteDC(hMemDC);

    // Drawing the game board
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            rect.left = j * cell_width;
            rect.top = i * cell_height;
            rect.right = rect.left + cell_width;
            rect.bottom = rect.top + cell_height;

            if (board[i][j] == WALL) {
                // Drawing walls as textured bricks
                hBrush = CreatePatternBrush(hBitmap);
                SelectObject(hdc, hBrush);
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
            } else if (board[i][j] == FOOD) {
                // Drawing food as cyan and smaller circles
                hBrush = CreateSolidBrush(RGB(0,0,255));
                SelectObject(hdc, hBrush);
                Ellipse(hdc, rect.left + 5, rect.top + 5, rect.right - 5, rect.bottom - 5);
                DeleteObject(hBrush);
            } else if (board[i][j] == PACMAN) {
                // Drawing Pacman as a larger yellow circle
                hBrush = CreateSolidBrush(RGB(255, 255, 0));
                SelectObject(hdc, hBrush);
                Ellipse(hdc, rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2);
                DeleteObject(hBrush);
            } else if (board[i][j] == DEMON) {
                // Drawing demons as red triangles
                POINT points[3] = {
                    {rect.left + cell_width / 2, rect.top + 2},
                    {rect.left + 2, rect.bottom - 2},
                    {rect.right - 2, rect.bottom - 2}
                };
                hBrush = CreateSolidBrush(RGB(255, 0, 0));
                SelectObject(hdc, hBrush);
                Polygon(hdc, points, 3);
                DeleteObject(hBrush);
            }
        }
    }

    // Displaying the score in white
    SetTextColor(hdc, RGB(255, 255, 255));
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    TextOut(hdc, 0, HEIGHT * cell_height + 10, scoreText, strlen(scoreText));

    // Drawing legend
    const int legend_start_x = 10;
    int legend_y = HEIGHT * cell_height + 40;
    
    // Legend for Food
    hBrush = CreateSolidBrush(RGB(0,0,255));
    SelectObject(hdc, hBrush);
    Ellipse(hdc, legend_start_x, legend_y, legend_start_x + 10, legend_y + 10);
    DeleteObject(hBrush);
    TextOut(hdc, legend_start_x + 20, legend_y, "Food", 4);

    // Legend for Pacman
    hBrush = CreateSolidBrush(RGB(255, 255, 0));
    SelectObject(hdc, hBrush);
    Ellipse(hdc, legend_start_x, legend_y + 20, legend_start_x + 15, legend_y + 35);
    DeleteObject(hBrush);
    TextOut(hdc, legend_start_x + 20, legend_y + 25, "Pacman", 6);

    // Legend for Demon
    hBrush = CreateSolidBrush(RGB(255, 0, 0));
    SelectObject(hdc, hBrush);
    POINT demon_points[3] = {
        {legend_start_x + 7, legend_y + 50},
        {legend_start_x - 3, legend_y + 70},
        {legend_start_x + 17, legend_y + 70}
    };
    Polygon(hdc, demon_points, 3);
    DeleteObject(hBrush);
    TextOut(hdc, legend_start_x + 20, legend_y + 55, "Demon", 5);

    // Cleaning up
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
    DeleteObject(hBitmap);
}





// Moving Pacman based on input
void move(int move_x, int move_y) {
    int x = pacman_x + move_x;
    int y = pacman_y + move_y;

    if (board[y][x] != WALL) {
        if (board[y][x] == FOOD) {
            score++;
            food--;
            curr++;
            if (food == 0) {
                res = 2;
                return;
            }
        } else if (board[y][x] == DEMON) {
            res = 1;
        }

        board[pacman_y][pacman_x] = EMPTY;
        pacman_x = x;
        pacman_y = y;
        board[pacman_y][pacman_x] = PACMAN;
    }
}

// Moving demons randomly
void moveDemons() {
    for (int i = 0; i < demon_count; i++) {
        int move_x = (rand() % 3) - 1;  // -1, 0, or 1
        int move_y = (rand() % 3) - 1;

        int x = demons[i].x + move_x;
        int y = demons[i].y + move_y;

        if (board[y][x] != WALL && board[y][x] != DEMON) {
            if (board[y][x] == PACMAN) {
                res = 1;
            }

            board[demons[i].y][demons[i].x] = EMPTY;
            demons[i].x = x;
            demons[i].y = y;
            board[y][x] = DEMON;
        }
    }
}

// Window procedure to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        draw(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN: {
        switch (wParam) {
        case 'W':
            move(0, -1);
            break;
        case 'S':
            move(0, 1);
            break;
        case 'A':
            move(-1, 0);
            break;
        case 'D':
            move(1, 0);
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        }
        moveDemons();
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "PacmanWindowClass";
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Pacman Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    initialize();

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (res == 1) {
            MessageBox(hwnd, "Game Over! Dead by Demon.", "Pacman", MB_OK);
            break;
        } else if (res == 2) {
            MessageBox(hwnd, "You Win!", "Pacman", MB_OK);
            break;
        }
    }

    return 0;
}
