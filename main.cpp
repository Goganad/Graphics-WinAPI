#ifndef _UNICODE
#define _UNICODE
#endif

#include <iostream>
#include <windows.h>
#include <gdiplus.h>
#include "FloatingImage.h"

const SIZE MIN_WINDOW_SIZE = SIZE{600, 600};
const COLORREF BACKGROUND_COLOR = RGB(255, 255, 255);

const SIZE IMAGE_SIZE = SIZE{150, 150};
const POINTFLOAT IMAGE_DEFAULT_POSITION = POINTFLOAT{25.0f, 25.0f};

const wchar_t IMAGE_PATH[] = L"amnesiac.png";

static FloatingImage img = FloatingImage();

static FloatingImage *sprite = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void initObjects() {
    Gdiplus::Image image = Gdiplus::Image(IMAGE_PATH);
    if (image.GetLastStatus() != Gdiplus::Ok) {
        MessageBoxW(nullptr, L"Error opening image", L"Attention", MB_OK);
        std::exit(0);
    }

    SIZE spriteNodeSize = IMAGE_SIZE;
    unsigned int imageWidth = image.GetWidth();
    unsigned int imageHeight = image.GetHeight();
    if (imageWidth == 0 || imageHeight == 0) {
        MessageBoxW(nullptr, L"Invalid image", L"Attention", MB_OK);
        std::exit(0);
    }
    float ratio = (float) imageWidth / (float) imageHeight;
    spriteNodeSize.cx = spriteNodeSize.cx * ratio;
    img = FloatingImage(IMAGE_DEFAULT_POSITION, spriteNodeSize, image.Clone());

    sprite = &img;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    initObjects();

    const wchar_t WINDOW_CLASS[] = L"MAIN_WINDOW_CLASS";
    const wchar_t WINDOW_TITLE[] = L"Amnesiac by Radiohead on white background";

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(BACKGROUND_COLOR);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = WINDOW_CLASS;
    wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
            // Optional window styles.
            0x0,
            // Window class
            WINDOW_CLASS,
            // Window text
            WINDOW_TITLE,
            // Window style
            WS_OVERLAPPEDWINDOW,
            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
            // Parent window
            nullptr,
            // Menu
            nullptr,
            // Instance handle
            hInstance,
            // Additional application data
            nullptr
    );

    if (hwnd == nullptr) {
        MessageBoxW(nullptr, L"Error creating window", L"Attention", MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

void fixImagePosition(HWND hwnd, FloatingImage *obj) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    if (obj->position.x + obj->size.cx > clientRect.right) {
        obj->position.x = clientRect.right - obj->size.cx * 2;
    } else if (obj->position.x < clientRect.left) {
        obj->position.x = clientRect.left +  obj->size.cx;
    }
    if (obj->position.y + obj->size.cy > clientRect.bottom) {
        obj->position.y = clientRect.bottom - obj->size.cy * 2;
    } else if (obj->position.y < clientRect.top) {
        obj->position.y = clientRect.top + obj->size.cy;
    }
}

void drawBackground(HDC hdc, PAINTSTRUCT ps) {
    HBRUSH brush = CreateSolidBrush(BACKGROUND_COLOR);
    FillRect(hdc, &ps.rcPaint, brush);
    DeleteObject(brush);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            RECT rcClientRect;
            GetClientRect(hwnd, &rcClientRect);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);

            SelectObject(memDC, GetStockObject(DC_PEN));
            SelectObject(memDC, GetStockObject(DC_BRUSH));

            HBITMAP bmp = CreateCompatibleBitmap(hdc, rcClientRect.right - rcClientRect.left,
                                                 rcClientRect.bottom - rcClientRect.top);
            HBITMAP oldBmp = (HBITMAP) SelectObject(memDC, bmp);

            drawBackground(memDC, ps);
            sprite->draw(memDC);

            BitBlt(hdc, 0, 0, rcClientRect.right - rcClientRect.left, rcClientRect.bottom - rcClientRect.top, memDC, 0,
                   0, SRCCOPY);

            SelectObject(memDC, oldBmp);
            DeleteObject(bmp);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_LEFT: {
                    sprite->position.x -= sprite->size.cx / 2.0f;
                    break;
                }
                case VK_RIGHT: {
                    sprite->position.x += sprite->size.cx / 2.0f;
                    break;
                }
                case VK_UP: {
                    sprite->position.y -= sprite->size.cy / 2.0f;
                    break;
                }
                case VK_DOWN: {
                    sprite->position.y += sprite->size.cy / 2.0f;
                    break;
                }
            }
            fixImagePosition(hwnd, sprite);
            InvalidateRect(hwnd, nullptr, false);
            break;
        }
        case WM_MOUSEWHEEL: {
            int fwKeys = GET_KEYSTATE_WPARAM(wParam);
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (fwKeys == MK_SHIFT) {
                if (zDelta > 0) {
                    sprite->position.x += sprite->size.cx / 4.0f;
                } else {
                    sprite->position.x -= sprite->size.cx / 4.0f;
                }
            } else {
                if (zDelta > 0) {
                    sprite->position.y -= sprite->size.cy / 4.0f;
                } else {
                    sprite->position.y += sprite->size.cy / 4.0f;
                }
            }
            fixImagePosition(hwnd, sprite);
            InvalidateRect(hwnd, nullptr, false);
            break;
        }
        case WM_SIZING: {
            fixImagePosition(hwnd, sprite);
            break;
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
            lpMMI->ptMinTrackSize.x = MIN_WINDOW_SIZE.cx;
            lpMMI->ptMinTrackSize.y = MIN_WINDOW_SIZE.cy;
            break;
        }
        default: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    return 0;
}