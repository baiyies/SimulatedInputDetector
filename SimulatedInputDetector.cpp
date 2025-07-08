#include <Windows.h>

HINSTANCE g_hInstance = NULL;

HWND g_hMainWnd = NULL;

HHOOK g_hMouseHook = NULL;

const wchar_t CLASS_NAME[] = L"SimulatedClickDetectorWindowClass";

#define UWM_SIMULATED_CLICK (WM_APP + 1)

//https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msllhookstruct
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_LBUTTONDOWN ||
            wParam == WM_LBUTTONUP ||
            wParam == WM_RBUTTONDOWN ||
            wParam == WM_RBUTTONUP)
        {
            MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

            HWND hWndUnderMouse = WindowFromPoint(pMouseStruct->pt);

            if (hWndUnderMouse == g_hMainWnd || IsChild(g_hMainWnd, hWndUnderMouse))
            {
                if (pMouseStruct->flags & LLMHF_INJECTED)
                {
                    PostMessage(g_hMainWnd, UWM_SIMULATED_CLICK, 0, 0);
                    return 1;
                }
            }
        }
    }

    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

void InstallHook()
{
    g_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_hInstance, 0);
}

void UninstallHook()
{
    if (g_hMouseHook)
    {
        UnhookWindowsHookEx(g_hMouseHook);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        DrawText(hdc, L"Please perform physical and simulated clicks on this window to observe the effect.", -1, &clientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
    }
    return 0;
    case UWM_SIMULATED_CLICK:
    {
        SetWindowText(hwnd, L"Simulated click detected!");
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        SetWindowText(hwnd, L"Physical left-click detected.");
    }
    return 0;
    case WM_RBUTTONDOWN:
    {
        SetWindowText(hwnd, L"Physical right-click detected.");
    }
    return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    SetProcessDPIAware();

    g_hInstance = hInstance;

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    g_hMainWnd = CreateWindowEx(
        0, 
        CLASS_NAME,
        L"Simulated Click Detector - Awaiting input...",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL 
    );

    if (g_hMainWnd == NULL)
    {
        return 0;
    }

    InstallHook();

    ShowWindow(g_hMainWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UninstallHook();

    return 0;
}