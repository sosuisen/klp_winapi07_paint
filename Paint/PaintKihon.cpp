#include <windows.h>
#include <windowsx.h> // GET_X_LPARAN, GET_Y_LPARAMマクロの定義
#include <string>

const int MAX_WIDTH = 400;
const int MAX_HEIGHT = 300;

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    TCHAR szAppName[] = L"PaintApp";
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    // ウィンドウクラスの属性を設定
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szAppName;

    // ウィンドウクラスを登録
    if (!RegisterClass(&wc)) return 0;

    // ウィンドウを作成
    hwnd = CreateWindow(
        szAppName, L"Paint",
        WS_OVERLAPPEDWINDOW,
        50, 50,
        MAX_WIDTH, MAX_HEIGHT,
        NULL, NULL,
        hInstance, NULL);

    if (!hwnd) return 0;

    // ウィンドウを表示
    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    POINT pt;

    // デバッグ情報表示用
    wchar_t format[] = L"%d,%d,%d,%d\n";
    wchar_t* buf = new wchar_t[18];

    RECT rect;

    LARGE_INTEGER start, end;

    switch (uMsg) {
    case WM_PAINT:
        PAINTSTRUCT paint;
        hdc = BeginPaint(hwnd, &paint);

        MoveToEx(hdc, 30, 30, NULL);
        LineTo(hdc, 110, 30);
        LineTo(hdc, 110, 110);

        LineTo(hdc, 30, 110);
        LineTo(hdc, 30, 30);
        LineTo(hdc, 110, 110);
        MoveToEx(hdc, 110, 30, NULL);
        LineTo(hdc, 30, 110); 

        // 更新領域の左上座標と幅、高さを表示
        // x, y, width, height
        /*
        swprintf_s(buf, 18, format,
            paint.rcPaint.left,
            paint.rcPaint.top,
            paint.rcPaint.right - paint.rcPaint.left,
            paint.rcPaint.bottom - paint.rcPaint.top);
        OutputDebugString(buf);
        */

        EndPaint(hwnd, &paint);

        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}