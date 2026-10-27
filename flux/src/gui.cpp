
#define NOMINMAX
#include "gui.h"
#include <windows.h>
#include <map>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace fluxgui {

namespace {

struct Item {
    HWND hwnd = nullptr;
    int parent = 0;
    HBRUSH brush = nullptr;
    COLORREF color = RGB(255, 255, 255);
    bool window = false;
};

std::map<int, Item> items;
std::vector<int> contexts, clicks;

int nextId = 1;
int windowCount = 0;

const wchar_t* klass = L"FluxScript09Window";

std::wstring wide(const std::string& s) {
    if (s.empty()) return L"";

    int n = MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), -1, nullptr, 0
    );

    if (!n)
        throw std::runtime_error("GUIError: invalid UTF-8");

    std::wstring w(n, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), -1, &w[0], n
    );

    w.pop_back();
    return w;
}

int integer(const Value& v) {
    if (v.type != Value::Type::NUMBER ||
        !std::isfinite(v.number) ||
        v.number != std::floor(v.number) ||
        v.number < -100000 ||
        v.number > 100000) {
        throw std::runtime_error(
            "GUIError: expected integer"
        );
    }

    return int(v.number);
}

std::pair<int, int> pair(const Value& v) {
    if (v.type != Value::Type::ARRAY ||
        v.array->size() != 2) {
        throw std::runtime_error(
            "GUIError: expected WxH dimension"
        );
    }

    return {
        integer((*v.array)[0]),
        integer((*v.array)[1])
    };
}

std::string str(const Value& v) {
    if (v.type != Value::Type::STRING)
        throw std::runtime_error(
            "GUIError: expected string"
        );

    return v.text;
}

COLORREF color(const std::string& s) {
    static const std::map<std::string, COLORREF> c = {
        {"black", RGB(0, 0, 0)},
        {"white", RGB(255, 255, 255)},
        {"red", RGB(255, 0, 0)},
        {"green", RGB(0, 180, 0)},
        {"blue", RGB(0, 90, 220)},
        {"gray", RGB(128, 128, 128)},
        {"grey", RGB(128, 128, 128)},
        {"yellow", RGB(255, 255, 0)},
        {"cyan", RGB(0, 255, 255)},
        {"magenta", RGB(255, 0, 255)}
    };

    auto i = c.find(s);

    if (i == c.end())
        throw std::runtime_error(
            "GUIError: unknown color '" + s + "'"
        );

    return i->second;
}

Item& get(int id) {
    auto it = items.find(id);

    if (it == items.end() ||
        !IsWindow(it->second.hwnd)) {
        throw std::runtime_error(
            "GUIError: invalid window/control reference"
        );
    }

    return it->second;
}

int active() {
    if (contexts.empty())
        throw std::runtime_error(
            "GUIError: no active window context"
        );

    int id = contexts.back();

    if (!get(id).window)
        throw std::runtime_error(
            "GUIError: context is not a window"
        );

    return id;
}

LRESULT CALLBACK proc(
    HWND h, UINT msg, WPARAM wp, LPARAM lp
) {
    if (msg == WM_CTLCOLORSTATIC ||
        msg == WM_CTLCOLOREDIT) {

        HWND child = reinterpret_cast<HWND>(lp);

        for (auto& e : items) {
            if (e.second.hwnd == child &&
                e.second.brush) {

                HDC dc = reinterpret_cast<HDC>(wp);

                SetBkColor(dc, e.second.color);

                SetTextColor(
                    dc,
                    (GetRValue(e.second.color) * 299 +
                     GetGValue(e.second.color) * 587 +
                     GetBValue(e.second.color) * 114)
                        < 128000
                        ? RGB(255, 255, 255)
                        : RGB(0, 0, 0)
                );

                return reinterpret_cast<LRESULT>(
                    e.second.brush
                );
            }
        }
    }

    if (msg == WM_DRAWITEM) {
        DRAWITEMSTRUCT* d =
            reinterpret_cast<DRAWITEMSTRUCT*>(lp);

        if (d) {
            for (auto& e : items) {
                if (e.first == int(d->CtlID) &&
                    e.second.brush) {

                    FillRect(
                        d->hDC,
                        &d->rcItem,
                        e.second.brush
                    );

                    DrawEdge(
                        d->hDC,
                        &d->rcItem,
                        (d->itemState & ODS_SELECTED)
                            ? EDGE_SUNKEN
                            : EDGE_RAISED,
                        BF_RECT
                    );

                    wchar_t text[512] = {};

                    GetWindowTextW(
                        e.second.hwnd, text, 511
                    );

                    SetBkMode(
                        d->hDC, TRANSPARENT
                    );

                    SetTextColor(
                        d->hDC,
                        (GetRValue(e.second.color) * 299 +
                         GetGValue(e.second.color) * 587 +
                         GetBValue(e.second.color) * 114)
                            < 128000
                            ? RGB(255, 255, 255)
                            : RGB(0, 0, 0)
                    );

                    DrawTextW(
                        d->hDC,
                        text,
                        -1,
                        &d->rcItem,
                        DT_CENTER |
                        DT_VCENTER |
                        DT_SINGLELINE
                    );

                    return TRUE;
                }
            }
        }
    }

    if (msg == WM_COMMAND &&
        HIWORD(wp) == BN_CLICKED) {

        int id = GetDlgCtrlID(
            reinterpret_cast<HWND>(lp)
        );

        if (id > 0)
            clicks.push_back(id);

        return 0;
    }

    if (msg == WM_ERASEBKGND) {
        for (auto& e : items) {
            if (e.second.hwnd == h &&
                e.second.window) {

                RECT r;
                GetClientRect(h, &r);

                FillRect(
                    reinterpret_cast<HDC>(wp),
                    &r,
                    e.second.brush
                );

                return 1;
            }
        }
    }

    if (msg == WM_DESTROY) {
        for (auto& e : items) {
            if (e.second.hwnd == h &&
                e.second.window) {

                --windowCount;
                break;
            }
        }

        return 0;
    }

    return DefWindowProcW(h, msg, wp, lp);
}

void init() {
    static bool done = false;

    if (done) return;

    WNDCLASSW w = {};

    w.lpfnWndProc = proc;
    w.hInstance = GetModuleHandleW(nullptr);
    w.lpszClassName = klass;

    // Correção do erro C2664:
    w.hCursor = LoadCursorW(
        nullptr,
        MAKEINTRESOURCEW(32512)
    );

    w.hbrBackground =
        GetSysColorBrush(COLOR_WINDOW);

    if (!RegisterClassW(&w) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        throw std::runtime_error(
            "GUIError: RegisterClassW failed"
        );
    }

    done = true;
}

int makeControl(
    const wchar_t* type,
    const Value& size,
    const Value& pos,
    const std::string& text,
    DWORD style
) {
    int parent = active();

    auto sz = pair(size);
    auto xy = pair(pos);

    if (sz.first < 1 || sz.second < 1)
        throw std::runtime_error(
            "GUIError: invalid control size"
        );

    int id = nextId++;

    HWND h = CreateWindowExW(
        0,
        type,
        wide(text).c_str(),
        WS_CHILD | WS_VISIBLE | style,
        xy.first,
        xy.second,
        sz.first,
        sz.second,
        get(parent).hwnd,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(id)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    if (!h)
        throw std::runtime_error(
            "GUIError: CreateWindowExW control failed"
        );

    items[id] = {
        h,
        parent,
        nullptr,
        RGB(255, 255, 255),
        false
    };

    return id;
}

} // namespace anônimo

void enter(int id) {
    get(id);
    contexts.push_back(id);
}

void leave() {
    if (!contexts.empty())
        contexts.pop_back();
}

int current() {
    return contexts.empty()
        ? 0
        : contexts.back();
}

void onClick(int id) {
    get(id);
}

std::vector<int> poll() {
    MSG msg;

    while (PeekMessageW(
        &msg, nullptr, 0, 0, PM_REMOVE
    )) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    auto out = clicks;
    clicks.clear();

    return out;
}

bool alive() {
    return windowCount > 0;
}

void shutdown() {
    for (auto& p : items) {
        if (p.second.window &&
            IsWindow(p.second.hwnd)) {
            DestroyWindow(p.second.hwnd);
        }
    }

    for (auto& p : items) {
        if (p.second.brush)
            DeleteObject(p.second.brush);
    }

    items.clear();
    contexts.clear();
    clicks.clear();
    windowCount = 0;
}

Value invoke(
    const std::string& n,
    const std::vector<Value>& a
) {
    auto argc = [&](size_t count) {
        if (a.size() != count)
            throw std::runtime_error(
                "TypeError: " + n +
                " expects " +
                std::to_string(count) +
                " arguments"
            );
    };

    if (n == "windowAdd") {
        argc(2);
        init();

        auto sz = pair(a[1]);

        if (sz.first < 1 ||
            sz.second < 1 ||
            sz.first > 16000 ||
            sz.second > 16000) {
            throw std::runtime_error(
                "GUIError: invalid window dimensions"
            );
        }

        int id = nextId++;

        HWND h = CreateWindowExW(
            0,
            klass,
            wide(str(a[0])).c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            sz.first,
            sz.second,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (!h)
            throw std::runtime_error(
                "GUIError: CreateWindowExW failed"
            );

        Item item;
        item.hwnd = h;
        item.window = true;
        item.brush =
            CreateSolidBrush(item.color);

        items[id] = item;
        ++windowCount;

        ShowWindow(h, SW_SHOW);
        UpdateWindow(h);

        return Value::num(id);
    }

    if (n == "windowBackground") {
        argc(1);

        Item& item = get(active());

        item.color = color(str(a[0]));

        if (item.brush)
            DeleteObject(item.brush);

        item.brush =
            CreateSolidBrush(item.color);

        InvalidateRect(
            item.hwnd, nullptr, TRUE
        );

        return Value();
    }

    if (n == "windowResizable") {
        argc(1);

        Item& item = get(active());

        if (a[0].type != Value::Type::BOOL)
            throw std::runtime_error(
                "GUIError: expected boolean"
            );

        LONG_PTR style = GetWindowLongPtrW(
            item.hwnd, GWL_STYLE
        );

        if (a[0].boolean) {
            style |= WS_THICKFRAME |
                     WS_MAXIMIZEBOX;
        } else {
            style &= ~(WS_THICKFRAME |
                       WS_MAXIMIZEBOX);
        }

        SetWindowLongPtrW(
            item.hwnd, GWL_STYLE, style
        );

        SetWindowPos(
            item.hwnd,
            nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOZORDER |
            SWP_FRAMECHANGED
        );

        return Value();
    }

    if (n == "windowLabel" ||
        n == "windowButton" ||
        n == "windowTextbox") {

        argc(4);

        color(str(a[1]));

        const bool label =
            n == "windowLabel";

        const bool button =
            n == "windowButton";

        int id = makeControl(
            label ? L"STATIC"
                  : button ? L"BUTTON"
                           : L"EDIT",
            a[0],
            a[2],
            str(a[3]),
            label ? SS_LEFT
                  : button ? BS_OWNERDRAW
                           : WS_BORDER |
                             ES_AUTOHSCROLL
        );

        items[id].color =
            color(str(a[1]));

        items[id].brush =
            CreateSolidBrush(
                items[id].color
            );

        InvalidateRect(
            items[id].hwnd,
            nullptr,
            TRUE
        );

        return Value::num(id);
    }

    throw std::runtime_error(
        "GUIError: unknown native function '" +
        n + "'"
    );
}
} // namespace fluxgui
