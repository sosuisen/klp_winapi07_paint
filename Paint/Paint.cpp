#include <windows.h>
#include <windowsx.h> // GET_X_LPARAN, GET_Y_LPARAMマクロの定義
#include <string>

const int MAX_WIDTH = 1200;
const int MAX_HEIGHT = 800;

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool isFirstPoint = true;
POINT prevPoint;
POINT currentPoint;
HBITMAP  hBitmap;    // ビットマップ
HDC      hMemDC;     // オフスクリーン


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
    wchar_t* buf = new wchar_t[18]; // (8文字+終端ヌル1文字)x2バイト。いわゆるASCIIコードの範囲はUTF16でも1文字2バイトに収まる

    RECT rect;

    LARGE_INTEGER start, end;


    switch (uMsg) {
    case WM_CREATE:
        // ダブルバッファリング
        // メモリデバイスコンテキストを作成
        hdc = GetDC(hwnd);
        hMemDC = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, MAX_WIDTH, MAX_HEIGHT);
        SelectObject(hMemDC, hBitmap);
        // 白で塗りつぶす
        PatBlt(hMemDC, 0, 0, MAX_WIDTH, MAX_HEIGHT, WHITENESS);

        ReleaseDC(hwnd, hdc);
        return 0;
    case WM_CLOSE:
        // メモリデバイスコンテキストの破棄
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        DestroyWindow(hwnd);
        return 0;
    case WM_MOUSEMOVE:
        //マウスが押下されたとき
         /*
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        if (pointIndex < 999) {
            points[pointIndex++] = pt;
        }
        */
        //マウス左が押されているか
        if (GetKeyState(VK_LBUTTON) & 0x80) {
            if (isFirstPoint) {
                prevPoint.x = GET_X_LPARAM(lParam);
                prevPoint.y = GET_Y_LPARAM(lParam);

                // ダブルバッファリングの場合はここでMoveToEx
                MoveToEx(hMemDC, prevPoint.x, prevPoint.y, NULL);
            }
            else {
                prevPoint.x = currentPoint.x;
                prevPoint.y = currentPoint.y;
            }
            currentPoint.x = GET_X_LPARAM(lParam);
            currentPoint.y = GET_Y_LPARAM(lParam);

            // ダブルバッファリングの場合はここでLineTo
            // LineToを呼ぶと、描画先の点へ現在位置が移動するので
            // MoveToEx は呼ばなくてよい
            LineTo(hMemDC, currentPoint.x, currentPoint.y);

            isFirstPoint = false;
            // 最後の引数を false にすると、
            // 更新領域内が消去されず、
            // これまで描いた線が残ります。
            // ただし、ダブルバッファリングされてない場合、
            // ウィンドウのリサイズなどで別途更新領域が発生すると
            // 消去されます。
            // InvalidateRect(hwnd, NULL, false);
        
            // 最後の引数を true にすると、
            // ダブルバッファリングされてない場合、
            // これまで描いた線は全て消去され、
            // 直前の線分のみが描画されます
            // 今回のケースで true にする意味はとくにない。
            // InvalidateRect(hwnd, NULL, true);

            rect.left = prevPoint.x < currentPoint.x ? prevPoint.x : currentPoint.x;
            rect.right = prevPoint.x > currentPoint.x ? prevPoint.x : currentPoint.x;
            rect.top = prevPoint.y < currentPoint.y ? prevPoint.y : currentPoint.y;
            rect.bottom = prevPoint.y > currentPoint.y ? prevPoint.y : currentPoint.y;

            // rect.left と rect.right が同じ
            // あるいは rect.top と rect.bottom が同じ値の場合、
            // WM_PAINTは呼ばれない
            // よって right と bottom に1足しておけば安全。
            rect.right++;
            rect.bottom++;

            // SXGA(1280x1024)くらいになると全画面更新と比べて差が出る。
            InvalidateRect(hwnd, &rect, true);
        }
        else {
            isFirstPoint = true;
        }

        return 0;
    case WM_PAINT:
        QueryPerformanceCounter(&start);

        PAINTSTRUCT paint;
        hdc = BeginPaint(hwnd, &paint);

        /*
        MoveToEx(hdc, 10, 10, NULL);
        LineTo(hdc, 110, 10);
        LineTo(hdc, 110, 110);
        */

        /*
        MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
        LineTo(hdc, currentPoint.x, currentPoint.y);
        */

        BitBlt(
            hdc, 
            paint.rcPaint.left,
            paint.rcPaint.top,
            paint.rcPaint.right - paint.rcPaint.left,
            paint.rcPaint.bottom - paint.rcPaint.top,
            hMemDC,
            paint.rcPaint.left,
            paint.rcPaint.top,
            SRCCOPY);

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

        QueryPerformanceCounter(&end);
        OutputDebugString((std::to_wstring(end.QuadPart - start.QuadPart) + L"\n").c_str());
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}