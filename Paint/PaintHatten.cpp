#include <windows.h>
#include <windowsx.h> // GET_X_LPARAN, GET_Y_LPARAMマクロの定義
#include <string>
#include <chrono>

static const int MAX_WIDTH = 1200;
static const int MAX_HEIGHT = 800;

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
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

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool isFirstDown = true;
    static POINT moveToPoint;
    static POINT lineToPoint;
    static HBITMAP  hBitmap;    // ビットマップ
    static HDC      hMemDC;     // オフスクリーン

    HDC hdc;

    // benchmark
    std::chrono::system_clock::time_point start, end;
    long microsec = 0;

    // デバッグ情報表示用
    wchar_t format[] = L"%d,%d,%d,%d\n";
    wchar_t* buf = new wchar_t[18]; // (8文字+終端ヌル1文字)x2バイト。いわゆるASCIIコードの範囲はUTF16でも1文字2バイトに収まる

    switch (uMsg) {
    case WM_CREATE:
        // オフスクリーンをメモリデバイスコンテキストを用いて作成
        hdc = GetDC(hwnd);
        hMemDC = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, MAX_WIDTH, MAX_HEIGHT);
        SelectObject(hMemDC, hBitmap);
        // オフスクリーンを白で塗りつぶす（デフォルトは黒）
        PatBlt(hMemDC, 0, 0, MAX_WIDTH, MAX_HEIGHT, WHITENESS);

        ReleaseDC(hwnd, hdc);
        return 0;
    case WM_CLOSE:
        // オフスクリーンの破棄
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        DestroyWindow(hwnd);
        return 0;
    case WM_MOUSEMOVE:
        if (GetKeyState(VK_LBUTTON) & 0x8000) {
            if (isFirstDown) {
                // 現在のマウス座標をmoveToEx用の位置へコピー
                moveToPoint.x = GET_X_LPARAM(lParam);
                moveToPoint.y = GET_Y_LPARAM(lParam);
                MoveToEx(hMemDC, moveToPoint.x, moveToPoint.y, NULL);
                isFirstDown = false;
            }
            else {
                // 前回のlineTo用の位置を、moveToEx用の位置へコピー
                moveToPoint.x = lineToPoint.x;
                moveToPoint.y = lineToPoint.y;
            }

            // 現在のマウス座標を、lineTo用の位置へコピー
            lineToPoint.x = GET_X_LPARAM(lParam);
            lineToPoint.y = GET_Y_LPARAM(lParam);

            LineTo(hMemDC, lineToPoint.x, lineToPoint.y);

            /*
             * InvalidateRect の第2引数には、
             * moveToPoint と lineToPoint を元に
             * 必要最小限の更新領域（RECT）を
             * 計算して与えることが可能（発展課題）
             * 下では NULL を与えてウィンドウ全体を更新領域に追加
             */
            // InvalidateRect(hwnd, NULL, false);


            RECT rect{};
            rect.left = moveToPoint.x < lineToPoint.x ? moveToPoint.x : lineToPoint.x;
            rect.right = moveToPoint.x > lineToPoint.x ? moveToPoint.x : lineToPoint.x;
            rect.top = moveToPoint.y < lineToPoint.y ? moveToPoint.y : lineToPoint.y;
            rect.bottom = moveToPoint.y > lineToPoint.y ? moveToPoint.y : lineToPoint.y;

            // rect.left と rect.right が同じ
            // あるいは rect.top と rect.bottom が同じ値の場合、
            // WM_PAINTは呼ばれない
            // よって right と bottom に1足しておけば安全。
            rect.right++;
            rect.bottom++;
            InvalidateRect(hwnd, &rect, false);

        }
        else {
            isFirstDown = true;
        }

        return 0;
    case WM_PAINT:
        PAINTSTRUCT paint;
        hdc = BeginPaint(hwnd, &paint);

        // ベンチマーク開始（発展課題用）
        start = std::chrono::system_clock::now();

        // ビットブロック転送
        // ソースはオフスクリーンのデバイスコンテキスト、
        // コピー先はウィンドウのデバイスコンテキスト。
        // ソースとコピー先の領域は、
        // 更新領域と同じなのでを paint構造体から計算できる。
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

        // ベンチマーク終了
        end = std::chrono::system_clock::now();
        microsec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        OutputDebugString((std::to_wstring(microsec) + L"\n").c_str());

        EndPaint(hwnd, &paint);

        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}