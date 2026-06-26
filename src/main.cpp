#include <windows.h>
#include <shellapi.h>

#include <string>

#include "text_cleaner.h"

namespace {

constexpr wchar_t kWindowClassName[] = L"ClipCleanerHiddenWindow";
constexpr wchar_t kAppName[] = L"ClipCleaner";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kHotkeyId = 1;
constexpr UINT kMenuCleanNow = 1001;
constexpr UINT kMenuToggleMarkdown = 1002;
constexpr UINT kMenuExit = 1003;

HINSTANCE g_instance = nullptr;
HWND g_hwnd = nullptr;
NOTIFYICONDATAW g_trayIcon = {};
CleanOptions g_options;

std::wstring GetLastErrorText(DWORD error) {
    wchar_t* buffer = nullptr;
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        nullptr);

    std::wstring message = length > 0 && buffer ? buffer : L"Unknown error";
    if (buffer) {
        LocalFree(buffer);
    }
    return message;
}

void ShowTrayBalloon(const std::wstring& title, const std::wstring& message, DWORD infoFlags = NIIF_INFO) {
    g_trayIcon.uFlags = NIF_INFO;
    wcsncpy_s(g_trayIcon.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(g_trayIcon.szInfo, message.c_str(), _TRUNCATE);
    g_trayIcon.dwInfoFlags = infoFlags;
    Shell_NotifyIconW(NIM_MODIFY, &g_trayIcon);
}

bool ReadClipboardText(std::wstring& text) {
    if (!OpenClipboard(g_hwnd)) {
        return false;
    }

    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (!data) {
        CloseClipboard();
        return false;
    }

    const wchar_t* locked = static_cast<const wchar_t*>(GlobalLock(data));
    if (!locked) {
        CloseClipboard();
        return false;
    }

    text = locked;
    GlobalUnlock(data);
    CloseClipboard();
    return true;
}

bool WriteClipboardText(const std::wstring& text) {
    if (!OpenClipboard(g_hwnd)) {
        return false;
    }

    if (!EmptyClipboard()) {
        CloseClipboard();
        return false;
    }

    const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!memory) {
        CloseClipboard();
        return false;
    }

    void* locked = GlobalLock(memory);
    if (!locked) {
        GlobalFree(memory);
        CloseClipboard();
        return false;
    }

    memcpy(locked, text.c_str(), bytes);
    GlobalUnlock(memory);

    if (!SetClipboardData(CF_UNICODETEXT, memory)) {
        GlobalFree(memory);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

void CleanClipboard() {
    std::wstring text;
    if (!ReadClipboardText(text)) {
        ShowTrayBalloon(kAppName, L"No Unicode text found in the clipboard.", NIIF_WARNING);
        return;
    }

    const std::wstring cleaned = CleanText(text, g_options);
    if (!WriteClipboardText(cleaned)) {
        ShowTrayBalloon(kAppName, L"Failed to write cleaned text to the clipboard.", NIIF_ERROR);
        return;
    }

    ShowTrayBalloon(kAppName, L"Clipboard text cleaned.");
}

void ShowTrayMenu(HWND hwnd) {
    HMENU menu = CreatePopupMenu();
    if (!menu) {
        return;
    }

    AppendMenuW(menu, MF_STRING, kMenuCleanNow, L"Clean clipboard now");
    AppendMenuW(menu,
                MF_STRING | (g_options.stripMarkdownLinks ? MF_CHECKED : MF_UNCHECKED),
                kMenuToggleMarkdown,
                L"Strip Markdown links");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMenuExit, L"Exit");

    POINT cursor = {};
    GetCursorPos(&cursor);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

bool AddTrayIcon(HWND hwnd) {
    g_trayIcon = {};
    g_trayIcon.cbSize = sizeof(g_trayIcon);
    g_trayIcon.hWnd = hwnd;
    g_trayIcon.uID = 1;
    g_trayIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_trayIcon.uCallbackMessage = kTrayMessage;
    g_trayIcon.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    g_trayIcon.uVersion = NOTIFYICON_VERSION_4;
    wcsncpy_s(g_trayIcon.szTip, L"ClipCleaner - Ctrl+Alt+V", _TRUNCATE);

    return Shell_NotifyIconW(NIM_ADD, &g_trayIcon) &&
           Shell_NotifyIconW(NIM_SETVERSION, &g_trayIcon);
}

void RemoveTrayIcon() {
    if (g_trayIcon.cbSize != 0) {
        Shell_NotifyIconW(NIM_DELETE, &g_trayIcon);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            if (!RegisterHotKey(hwnd, kHotkeyId, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'V')) {
                ShowTrayBalloon(kAppName,
                                L"Failed to register Ctrl+Alt+V: " + GetLastErrorText(GetLastError()),
                                NIIF_ERROR);
            }
            AddTrayIcon(hwnd);
            return 0;

        case WM_HOTKEY:
            if (wparam == kHotkeyId) {
                CleanClipboard();
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case kMenuCleanNow:
                    CleanClipboard();
                    return 0;
                case kMenuToggleMarkdown:
                    g_options.stripMarkdownLinks = !g_options.stripMarkdownLinks;
                    return 0;
                case kMenuExit:
                    DestroyWindow(hwnd);
                    return 0;
                default:
                    break;
            }
            break;

        case kTrayMessage:
            if (LOWORD(lparam) == WM_RBUTTONUP || LOWORD(lparam) == WM_CONTEXTMENU) {
                ShowTrayMenu(hwnd);
            } else if (LOWORD(lparam) == WM_LBUTTONDBLCLK) {
                CleanClipboard();
            }
            return 0;

        case WM_DESTROY:
            UnregisterHotKey(hwnd, kHotkeyId);
            RemoveTrayIcon();
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }

    return DefWindowProcW(hwnd, message, wparam, lparam);
}

bool CreateHiddenWindow() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_instance;
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    g_hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        kAppName,
        WS_OVERLAPPED,
        0,
        0,
        0,
        0,
        nullptr,
        nullptr,
        g_instance,
        nullptr);

    return g_hwnd != nullptr;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    g_instance = instance;

    if (!CreateHiddenWindow()) {
        MessageBoxW(nullptr, L"Failed to start ClipCleaner.", kAppName, MB_ICONERROR | MB_OK);
        return 1;
    }

    MSG message = {};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return 0;
}
