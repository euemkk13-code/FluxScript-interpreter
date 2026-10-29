
#define NOMINMAX
#include "gui.h"

#include <algorithm>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#include <gdiplus.h>

#undef min
#undef max

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <memory>
#include <chrono>
#include <utility>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")

namespace fluxgui {
namespace {

using Clock = std::chrono::steady_clock;

struct Item {
    HWND hwnd = nullptr;
    int parent = 0;

    bool window = false;
    bool hovering = false;
    bool bold = false;

    std::string kind;
    std::string font = "Arial";

    int fontSize = 16;

    COLORREF color = RGB(255, 255, 255);
    COLORREF background = RGB(32, 32, 32);

    HBRUSH brush = nullptr;
    HFONT fontHandle = nullptr;

    std::unique_ptr<Gdiplus::Image> image;
};

struct Timer {
    int id;
    Clock::time_point deadline;
};

std::map<int, Item> items;
std::vector<int> contexts;
std::vector<Event> events;
std::vector<Timer> timers;

int nextId = 1;
int windowCount = 0;

ULONG_PTR gdipToken = 0;
bool initialized = false;

const wchar_t* windowClass = L"FluxScriptWindow";

std::wstring wide(const std::string& s) {
    if (s.empty())
        return L"";

    int n = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        s.c_str(),
        -1,
        nullptr,
        0
    );

    if (!n)
        throw std::runtime_error(
            "GUIError: invalid UTF-8"
        );

    std::wstring result(n, L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        s.c_str(),
        -1,
        &result[0],
        n
    );

    result.pop_back();
    return result;
}

std::string utf8(const std::wstring& s) {
    if (s.empty())
        return "";

    int n = WideCharToMultiByte(
        CP_UTF8,
        0,
        s.data(),
        int(s.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    std::string result(n, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        s.data(),
        int(s.size()),
        &result[0],
        n,
        nullptr,
        nullptr
    );

    return result;
}

std::string textOf(HWND hwnd) {
    int n = GetWindowTextLengthW(hwnd);

    std::wstring buffer(n + 1, L'\0');

    GetWindowTextW(
        hwnd,
        &buffer[0],
        n + 1
    );

    buffer.resize(n);

    return utf8(buffer);
}

double number(const Value& v) {
    if (v.type != Value::Type::NUMBER ||
        !std::isfinite(v.number))
        throw std::runtime_error(
            "GUIError: expected number"
        );

    return v.number;
}

int integer(const Value& v) {
    double n = number(v);

    if (n != std::floor(n) ||
        n < -100000 ||
        n > 100000)
        throw std::runtime_error(
            "GUIError: expected integer"
        );

    return int(n);
}

std::string string(const Value& v) {
    if (v.type != Value::Type::STRING)
        throw std::runtime_error(
            "GUIError: expected string"
        );

    return v.text;
}

bool boolean(const Value& v) {
    if (v.type != Value::Type::BOOL)
        throw std::runtime_error(
            "GUIError: expected boolean"
        );

    return v.boolean;
}

std::pair<int, int> dimensions(const Value& v) {
    if (v.type != Value::Type::ARRAY ||
        !v.array ||
        v.array->size() != 2)
        throw std::runtime_error(
            "GUIError: expected WxH"
        );

    return {
        integer((*v.array)[0]),
        integer((*v.array)[1])
    };
}

Value pairValue(int a, int b) {
    return Value::list({
        Value::num(a),
        Value::num(b)
    });
}

COLORREF parseColor(const std::string& s) {
    static const std::map<std::string, COLORREF> colors = {
        {"black", RGB(0, 0, 0)},
        {"white", RGB(255, 255, 255)},
        {"red", RGB(255, 0, 0)},
        {"green", RGB(0, 180, 0)},
        {"blue", RGB(0, 90, 220)},
        {"gray", RGB(128, 128, 128)},
        {"grey", RGB(128, 128, 128)},
        {"yellow", RGB(255, 255, 0)},
        {"cyan", RGB(0, 255, 255)},
        {"magenta", RGB(255, 0, 255)},
        {"orange", RGB(255, 165, 0)},
        {"purple", RGB(128, 0, 128)},
        {"pink", RGB(255, 105, 180)}
    };

    auto it = colors.find(s);

    if (it != colors.end())
        return it->second;

    if (s.size() == 7 && s[0] == '#') {
        try {
            size_t used = 0;

            unsigned long rgb =
                std::stoul(s.substr(1), &used, 16);

            if (used == 6) {
                return RGB(
                    (rgb >> 16) & 255,
                    (rgb >> 8) & 255,
                    rgb & 255
                );
            }
        } catch (...) {}
    }

    throw std::runtime_error(
        "GUIError: unknown color '" + s + "'"
    );
}

std::string colorText(COLORREF c) {
    const char* hex = "0123456789ABCDEF";

    std::string s = "#000000";

    int values[3] = {
        GetRValue(c),
        GetGValue(c),
        GetBValue(c)
    };

    for (int i = 0; i < 3; ++i) {
        s[1 + i * 2] = hex[values[i] >> 4];
        s[2 + i * 2] = hex[values[i] & 15];
    }

    return s;
}

Item& get(int id) {
    auto it = items.find(id);

    if (it == items.end() ||
        !IsWindow(it->second.hwnd))
        throw std::runtime_error(
            "GUIError: invalid control"
        );

    return it->second;
}

int active() {
    if (contexts.empty())
        throw std::runtime_error(
            "GUIError: no active window"
        );

    int id = contexts.back();

    if (!get(id).window)
        throw std::runtime_error(
            "GUIError: expected window"
        );

    return id;
}

int findId(HWND hwnd) {
    for (const auto& p : items) {
        if (p.second.hwnd == hwnd)
            return p.first;
    }

    return 0;
}

void emit(
    int id,
    const std::string& kind,
    Value value = Value()
) {
    events.push_back({
        id,
        kind,
        std::move(value)
    });
}

COLORREF parentBackground(const Item& item) {
    if (item.window)
        return item.background;

    auto it = items.find(item.parent);

    if (it != items.end())
        return it->second.background;

    return RGB(255, 255, 255);
}

HBRUSH parentBrush(const Item& item) {
    if (item.window)
        return item.brush;

    auto it = items.find(item.parent);

    if (it != items.end())
        return it->second.brush;

    return GetSysColorBrush(COLOR_WINDOW);
}

void refreshChildren(int parent) {
    for (auto& p : items) {
        if (p.second.parent == parent &&
            IsWindow(p.second.hwnd)) {

            InvalidateRect(
                p.second.hwnd,
                nullptr,
                TRUE
            );
        }
    }
}

void setColor(Item& item, COLORREF color) {
    item.color = color;

    InvalidateRect(
        item.hwnd,
        nullptr,
        TRUE
    );
}

void setBackground(Item& item, COLORREF color) {
    item.background = color;

    if (item.brush) {
        DeleteObject(item.brush);
        item.brush = nullptr;
    }

    item.brush = CreateSolidBrush(color);

    InvalidateRect(
        item.hwnd,
        nullptr,
        TRUE
    );

    if (item.window)
        refreshChildren(findId(item.hwnd));
}

void updateFont(Item& item) {
    if (item.fontHandle) {
        SendMessageW(
            item.hwnd,
            WM_SETFONT,
            0,
            FALSE
        );

        DeleteObject(item.fontHandle);
        item.fontHandle = nullptr;
    }

    HDC dc = GetDC(item.hwnd);

    int dpi = dc
        ? GetDeviceCaps(dc, LOGPIXELSY)
        : 96;

    if (dc)
        ReleaseDC(item.hwnd, dc);

    item.fontHandle = CreateFontW(
        -MulDiv(item.fontSize, dpi, 72),
        0, 0, 0,
        item.bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        wide(item.font).c_str()
    );

    SendMessageW(
        item.hwnd,
        WM_SETFONT,
        reinterpret_cast<WPARAM>(
            item.fontHandle
        ),
        TRUE
    );
}

void drawButton(DRAWITEMSTRUCT* d, Item& item) {
    HDC dc = d->hDC;

    RECT rect = d->rcItem;

    COLORREF bg = parentBackground(item);

    HBRUSH brush = CreateSolidBrush(bg);

    FillRect(dc, &rect, brush);
    DeleteObject(brush);

    COLORREF border = item.color;

    HPEN pen = CreatePen(
        PS_SOLID,
        1,
        border
    );

    HGDIOBJ oldPen =
        SelectObject(dc, pen);

    HGDIOBJ oldBrush =
        SelectObject(
            dc,
            GetStockObject(NULL_BRUSH)
        );

    Rectangle(
        dc,
        rect.left,
        rect.top,
        rect.right,
        rect.bottom
    );

    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(pen);

    if (d->itemState & ODS_SELECTED) {
        OffsetRect(&rect, 1, 1);
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, item.color);

    HGDIOBJ oldFont = nullptr;

    if (item.fontHandle) {
        oldFont = SelectObject(
            dc,
            item.fontHandle
        );
    }

    std::wstring caption =
        wide(textOf(item.hwnd));

    DrawTextW(
        dc,
        caption.c_str(),
        -1,
        &rect,
        DT_CENTER |
        DT_VCENTER |
        DT_SINGLELINE
    );

    if (oldFont)
        SelectObject(dc, oldFont);

    if (d->itemState & ODS_FOCUS) {
        RECT focus = d->rcItem;
        InflateRect(&focus, -4, -4);
        DrawFocusRect(dc, &focus);
    }
}

LRESULT CALLBACK controlProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp,
    UINT_PTR,
    DWORD_PTR
) {
    int id = findId(hwnd);

    if (id && msg == WM_MOUSEMOVE) {
        Item& item = items.at(id);

        if (!item.hovering) {
            item.hovering = true;

            TRACKMOUSEEVENT t = {};
            t.cbSize = sizeof(t);
            t.dwFlags = TME_LEAVE;
            t.hwndTrack = hwnd;

            TrackMouseEvent(&t);

            emit(
                id,
                "mouseEnter",
                Value::truth(true)
            );
        }

        emit(
            id,
            "detectMouseMoving",
            Value::truth(true)
        );
    }

    if (id && msg == WM_MOUSELEAVE) {
        items.at(id).hovering = false;

        emit(
            id,
            "mouseLeave",
            Value::truth(false)
        );
    }

    if (id && msg == WM_RBUTTONUP)
        emit(id, "onrclick");

    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(
            hwnd,
            controlProc,
            1
        );
    }

    return DefSubclassProc(
        hwnd,
        msg,
        wp,
        lp
    );
}

LRESULT CALLBACK windowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
) {
    if (msg == WM_COMMAND) {
        HWND child =
            reinterpret_cast<HWND>(lp);

        int id = findId(child);

        if (id) {
            if (HIWORD(wp) == BN_CLICKED) {
                emit(id, "onclick");

                if (items.at(id).kind == "checkbox") {
                    bool checked =
                        SendMessageW(
                            child,
                            BM_GETCHECK,
                            0,
                            0
                        ) == BST_CHECKED;

                    emit(
                        id,
                        "onchange",
                        Value::truth(checked)
                    );
                }
            }

            if (HIWORD(wp) == EN_CHANGE) {
                emit(
                    id,
                    "onchange",
                    Value::str(textOf(child))
                );
            }
        }
    }

    if (msg == WM_HSCROLL ||
        msg == WM_VSCROLL) {

        HWND child =
            reinterpret_cast<HWND>(lp);

        int id = findId(child);

        if (id) {
            LRESULT pos = SendMessageW(
                child,
                TBM_GETPOS,
                0,
                0
            );

            emit(
                id,
                "onchange",
                Value::num(double(pos))
            );
        }
    }

    if (msg == WM_CTLCOLORSTATIC ||
        msg == WM_CTLCOLOREDIT ||
        msg == WM_CTLCOLORBTN) {

        HWND child =
            reinterpret_cast<HWND>(lp);

        int id = findId(child);

        if (id) {
            Item& item = items.at(id);

            HDC dc =
                reinterpret_cast<HDC>(wp);

            SetTextColor(
                dc,
                item.color
            );

            SetBkMode(
                dc,
                TRANSPARENT
            );

            SetBkColor(
                dc,
                parentBackground(item)
            );

            return reinterpret_cast<LRESULT>(
                parentBrush(item)
            );
        }
    }

    if (msg == WM_DRAWITEM) {
        auto* d =
            reinterpret_cast<DRAWITEMSTRUCT*>(lp);

        if (d) {
            int id = findId(d->hwndItem);

            if (id &&
                items.at(id).kind == "button") {

                drawButton(
                    d,
                    items.at(id)
                );

                return TRUE;
            }
        }
    }

    if (msg == WM_ERASEBKGND) {
        int id = findId(hwnd);

        if (id) {
            Item& item = items.at(id);

            if (item.window) {
                RECT rect;
                GetClientRect(hwnd, &rect);

                FillRect(
                    reinterpret_cast<HDC>(wp),
                    &rect,
                    item.brush
                );

                return 1;
            }

            if (item.kind == "image") {
                RECT rect;
                GetClientRect(hwnd, &rect);

                FillRect(
                    reinterpret_cast<HDC>(wp),
                    &rect,
                    parentBrush(item)
                );

                return 1;
            }
        }
    }

    if (msg == WM_PAINT) {
        int id = findId(hwnd);

        if (id &&
            items.at(id).kind == "image") {

            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);

            Item& item = items.at(id);

            RECT rect;
            GetClientRect(hwnd, &rect);

            FillRect(
                dc,
                &rect,
                parentBrush(item)
            );

            if (item.image) {
                Gdiplus::Graphics graphics(dc);

                graphics.DrawImage(
                    item.image.get(),
                    0,
                    0,
                    rect.right,
                    rect.bottom
                );
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    if (msg == WM_DESTROY) {
        int id = findId(hwnd);

        if (id && items.at(id).window) {
            --windowCount;
            emit(id, "close");
        }

        return 0;
    }

    return DefWindowProcW(
        hwnd,
        msg,
        wp,
        lp
    );
}

void init() {
    if (initialized)
        return;

    INITCOMMONCONTROLSEX controls = {};
    controls.dwSize = sizeof(controls);

    controls.dwICC =
        ICC_BAR_CLASSES |
        ICC_PROGRESS_CLASS;

    if (!InitCommonControlsEx(&controls))
        throw std::runtime_error(
            "GUIError: common controls initialization failed"
        );

    Gdiplus::GdiplusStartupInput input;

    if (Gdiplus::GdiplusStartup(
        &gdipToken,
        &input,
        nullptr
    ) != Gdiplus::Ok)
        throw std::runtime_error(
            "GUIError: GDI+ initialization failed"
        );

    WNDCLASSW wc = {};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = windowClass;

    wc.hCursor = LoadCursorW(
        nullptr,
        MAKEINTRESOURCEW(32512)
    );

    wc.hbrBackground =
        GetSysColorBrush(COLOR_WINDOW);

    if (!RegisterClassW(&wc) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throw std::runtime_error(
            "GUIError: RegisterClassW failed"
        );

    initialized = true;
}

int makeControl(
    const wchar_t* klass,
    const std::string& kind,
    const Value& size,
    const Value& position,
    const std::string& text,
    DWORD style
) {
    init();

    int parent = active();

    auto sz = dimensions(size);
    auto xy = dimensions(position);

    if (sz.first < 1 || sz.second < 1)
        throw std::runtime_error(
            "GUIError: invalid control size"
        );

    int id = nextId++;

    HWND hwnd = CreateWindowExW(
        0,
        klass,
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

    if (!hwnd)
        throw std::runtime_error(
            "GUIError: cannot create control"
        );

    Item& item = items[id];

    item.hwnd = hwnd;
    item.parent = parent;
    item.kind = kind;

    item.background =
        parentBackground(item);

    updateFont(item);

    SetWindowSubclass(
        hwnd,
        controlProc,
        1,
        0
    );

    return id;
}

void setImage(
    Item& item,
    const std::string& path
) {
    std::wstring filename = wide(path);

    std::unique_ptr<Gdiplus::Image> image(
        new Gdiplus::Image(
            filename.c_str()
        )
    );

    if (image->GetLastStatus() != Gdiplus::Ok)
        throw std::runtime_error(
            "GUIError: cannot load image '" +
            path + "'"
        );

    item.image = std::move(image);

    InvalidateRect(
        item.hwnd,
        nullptr,
        TRUE
    );
}

} // anonymous namespace

void enter(int id) {
    if (!get(id).window)
        throw std::runtime_error(
            "GUIError: expected window"
        );

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

std::vector<Event> pollEvents() {
    MSG msg;

    while (PeekMessageW(
        &msg,
        nullptr,
        0,
        0,
        PM_REMOVE
    )) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    auto now = Clock::now();

    for (auto it = timers.begin();
         it != timers.end();) {

        if (now >= it->deadline) {
            emit(it->id, "timer");
            it = timers.erase(it);
        } else {
            ++it;
        }
    }

    std::vector<Event> result;
    result.swap(events);

    return result;
}

std::vector<int> poll() {
    std::vector<int> clicks;

    for (const Event& event : pollEvents()) {
        if (event.kind == "onclick")
            clicks.push_back(event.id);
    }

    return clicks;
}

int scheduleTimer(double seconds) {
    if (!std::isfinite(seconds) ||
        seconds < 0)
        throw std::runtime_error(
            "GUIError: invalid timer duration"
        );

    int id = nextId++;

    timers.push_back({
        id,
        Clock::now() +
        std::chrono::duration_cast<
            Clock::duration
        >(
            std::chrono::duration<double>(
                seconds
            )
        )
    });

    return id;
}

void cancelTimer(int id) {
    timers.erase(
        std::remove_if(
            timers.begin(),
            timers.end(),
            [id](const Timer& t) {
                return t.id == id;
            }
        ),
        timers.end()
    );
}

bool alive() {
    return windowCount > 0;
}

Value property(
    int id,
    const std::string& name
) {
    Item& item = get(id);

    RECT rect = {};
    GetWindowRect(item.hwnd, &rect);

    if (item.parent) {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        POINT point = {
            rect.left,
            rect.top
        };

        ScreenToClient(
            get(item.parent).hwnd,
            &point
        );

        rect.left = point.x;
        rect.top = point.y;
        rect.right = point.x + width;
        rect.bottom = point.y + height;
    }

    if (name == "text")
        return Value::str(
            textOf(item.hwnd)
        );

    if (name == "color")
        return Value::str(
            colorText(item.color)
        );

    if (name == "position")
        return pairValue(
            rect.left,
            rect.top
        );

    if (name == "size")
        return pairValue(
            rect.right - rect.left,
            rect.bottom - rect.top
        );

    if (name == "visible")
        return Value::truth(
            IsWindowVisible(item.hwnd) != FALSE
        );

    if (name == "enabled")
        return Value::truth(
            IsWindowEnabled(item.hwnd) != FALSE
        );

    if (name == "font")
        return Value::str(item.font);

    if (name == "bold")
        return Value::truth(item.bold);

    if (name == "detectmouse")
        return Value::truth(
            item.hovering
        );

    if (name == "checked" &&
        item.kind == "checkbox") {

        return Value::truth(
            SendMessageW(
                item.hwnd,
                BM_GETCHECK,
                0,
                0
            ) == BST_CHECKED
        );
    }

    if (name == "value" &&
        (item.kind == "slider" ||
         item.kind == "progress")) {

        UINT message =
            item.kind == "slider"
                ? TBM_GETPOS
                : PBM_GETPOS;

        return Value::num(
            double(
                SendMessageW(
                    item.hwnd,
                    message,
                    0,
                    0
                )
            )
        );
    }

    throw std::runtime_error(
        "GUIError: unknown property '" +
        name + "'"
    );
}

void setProperty(
    int id,
    const std::string& name,
    const Value& value
) {
    Item& item = get(id);

    if (name == "text") {
        SetWindowTextW(
            item.hwnd,
            wide(string(value)).c_str()
        );
        return;
    }

    if (name == "color") {
        setColor(
            item,
            parseColor(string(value))
        );
        return;
    }

    if (name == "position") {
        auto p = dimensions(value);

        SetWindowPos(
            item.hwnd,
            nullptr,
            p.first,
            p.second,
            0,
            0,
            SWP_NOSIZE |
            SWP_NOZORDER
        );
        return;
    }

    if (name == "size") {
        auto p = dimensions(value);

        if (p.first < 1 || p.second < 1)
            throw std::runtime_error(
                "GUIError: invalid size"
            );

        SetWindowPos(
            item.hwnd,
            nullptr,
            0,
            0,
            p.first,
            p.second,
            SWP_NOMOVE |
            SWP_NOZORDER
        );
        return;
    }

    if (name == "visible") {
        ShowWindow(
            item.hwnd,
            boolean(value)
                ? SW_SHOW
                : SW_HIDE
        );
        return;
    }

    if (name == "enabled") {
        EnableWindow(
            item.hwnd,
            boolean(value)
        );
        return;
    }

    if (name == "font") {
        item.font = string(value);
        updateFont(item);
        return;
    }

    if (name == "bold") {
        item.bold = boolean(value);
        updateFont(item);
        return;
    }

    if (name == "checked" &&
        item.kind == "checkbox") {

        SendMessageW(
            item.hwnd,
            BM_SETCHECK,
            boolean(value)
                ? BST_CHECKED
                : BST_UNCHECKED,
            0
        );
        return;
    }

    if (name == "value" &&
        (item.kind == "slider" ||
         item.kind == "progress")) {

        SendMessageW(
            item.hwnd,
            item.kind == "slider"
                ? TBM_SETPOS
                : PBM_SETPOS,
            TRUE,
            integer(value)
        );
        return;
    }

    throw std::runtime_error(
        "GUIError: property cannot be assigned '" +
        name + "'"
    );
}

Value invoke(
    const std::string& name,
    const std::vector<Value>& args
) {
    auto argc = [&](size_t count) {
        if (args.size() != count)
            throw std::runtime_error(
                "TypeError: " + name +
                " expects " +
                std::to_string(count) +
                " arguments"
            );
    };

    if (name == "alert" ||
        name == "confirm") {

        argc(1);

        int result = MessageBoxW(
            nullptr,
            wide(string(args[0])).c_str(),
            L"FluxScript",
            name == "confirm"
                ? MB_YESNO | MB_ICONQUESTION
                : MB_OK | MB_ICONINFORMATION
        );

        return name == "confirm"
            ? Value::truth(result == IDYES)
            : Value();
    }

    if (name == "windowAdd") {
        argc(2);
        init();

        auto size = dimensions(args[1]);

        if (size.first < 1 ||
            size.second < 1)
            throw std::runtime_error(
                "GUIError: invalid window size"
            );

        int id = nextId++;

        HWND hwnd = CreateWindowExW(
            0,
            windowClass,
            wide(string(args[0])).c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            size.first,
            size.second,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (!hwnd)
            throw std::runtime_error(
                "GUIError: cannot create window"
            );

        Item& item = items[id];

        item.hwnd = hwnd;
        item.window = true;
        item.kind = "window";

        setBackground(
            item,
            RGB(255, 255, 255)
        );

        ++windowCount;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        return Value::num(id);
    }

    if (name == "windowBackground") {
        argc(1);

        setBackground(
            get(active()),
            parseColor(string(args[0]))
        );

        return Value();
    }

    if (name == "windowResizable") {
        argc(1);

        HWND hwnd = get(active()).hwnd;

        LONG_PTR style =
            GetWindowLongPtrW(
                hwnd,
                GWL_STYLE
            );

        if (boolean(args[0])) {
            style |=
                WS_THICKFRAME |
                WS_MAXIMIZEBOX;
        } else {
            style &= ~(
                WS_THICKFRAME |
                WS_MAXIMIZEBOX
            );
        }

        SetWindowLongPtrW(
            hwnd,
            GWL_STYLE,
            style
        );

        SetWindowPos(
            hwnd,
            nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOZORDER |
            SWP_FRAMECHANGED
        );

        return Value();
    }

    if (name == "windowLabel") {
        argc(4);

        int id = makeControl(
            L"STATIC",
            "label",
            args[0],
            args[2],
            string(args[3]),
            SS_LEFT
        );

        setColor(
            items[id],
            parseColor(string(args[1]))
        );

        return Value::num(id);
    }

    if (name == "windowButton") {
        argc(4);

        int id = makeControl(
            L"BUTTON",
            "button",
            args[0],
            args[2],
            string(args[3]),
            BS_OWNERDRAW
        );

        setColor(
            items[id],
            parseColor(string(args[1]))
        );

        return Value::num(id);
    }

    if (name == "windowTextbox") {
        argc(4);

        int id = makeControl(
            L"EDIT",
            "textbox",
            args[0],
            args[2],
            string(args[3]),
            WS_BORDER |
            ES_AUTOHSCROLL
        );

        setColor(
            items[id],
            parseColor(string(args[1]))
        );

        return Value::num(id);
    }

    if (name == "windowCheckbox") {
        argc(3);

        int id = makeControl(
            L"BUTTON",
            "checkbox",
            args[0],
            args[1],
            "",
            BS_AUTOCHECKBOX
        );

        setColor(
            items[id],
            parseColor(string(args[2]))
        );

        return Value::num(id);
    }

    if (name == "windowSlider") {
        argc(5);

        std::string left =
            string(args[0]);

        std::string right =
            string(args[1]);

        int id = makeControl(
            TRACKBAR_CLASSW,
            "slider",
            args[2],
            args[3],
            "",
            TBS_AUTOTICKS
        );

        SendMessageW(
            items[id].hwnd,
            TBM_SETRANGE,
            TRUE,
            MAKELONG(0, 100)
        );

        COLORREF color =
            parseColor(string(args[4]));

        setColor(items[id], color);

        auto pos = dimensions(args[3]);
        auto size = dimensions(args[2]);

        int labelWidth =
            std::max(50, size.first / 3);

        Value labelSize =
            pairValue(labelWidth, 22);

        Value leftPos = pairValue(
            pos.first,
            pos.second + size.second
        );

        Value rightPos = pairValue(
            pos.first +
                size.first -
                labelWidth,
            pos.second + size.second
        );

        int leftId = makeControl(
            L"STATIC",
            "label",
            labelSize,
            leftPos,
            left,
            SS_LEFT
        );

        int rightId = makeControl(
            L"STATIC",
            "label",
            labelSize,
            rightPos,
            right,
            SS_RIGHT
        );

        setColor(items[leftId], color);
        setColor(items[rightId], color);

        return Value::num(id);
    }

    if (name == "windowProgressBar") {
        argc(6);

        int minValue =
            integer(args[4]);

        int maxValue =
            integer(args[5]);

        if (minValue >= maxValue)
            throw std::runtime_error(
                "GUIError: invalid progress range"
            );

        int id = makeControl(
            PROGRESS_CLASSW,
            "progress",
            args[0],
            args[1],
            "",
            PBS_SMOOTH
        );

        HWND hwnd = items[id].hwnd;

        SendMessageW(
            hwnd,
            PBM_SETRANGE32,
            minValue,
            maxValue
        );

        SendMessageW(
            hwnd,
            PBM_SETPOS,
            minValue,
            0
        );

        SendMessageW(
            hwnd,
            PBM_SETBARCOLOR,
            0,
            parseColor(string(args[2]))
        );

        SendMessageW(
            hwnd,
            PBM_SETBKCOLOR,
            0,
            parseColor(string(args[3]))
        );

        return Value::num(id);
    }

    if (name == "windowImage") {
        argc(1);

        std::string path =
            string(args[0]);

        int id = makeControl(
            windowClass,
            "image",
            pairValue(200, 150),
            pairValue(0, 0),
            "",
            0
        );

        setImage(
            items[id],
            path
        );

        return Value::num(id);
    }

    throw std::runtime_error(
        "GUIError: unknown native function '" +
        name + "'"
    );
}

void shutdown() {
    for (auto& p : items) {
        if (p.second.window &&
            IsWindow(p.second.hwnd)) {

            DestroyWindow(p.second.hwnd);
        }
    }

    for (auto& p : items) {
        if (p.second.fontHandle) {
            DeleteObject(
                p.second.fontHandle
            );
        }

        if (p.second.brush) {
            DeleteObject(
                p.second.brush
            );
        }
    }

    items.clear();
    contexts.clear();
    events.clear();
    timers.clear();

    windowCount = 0;

    if (gdipToken) {
        Gdiplus::GdiplusShutdown(
            gdipToken
        );
        gdipToken = 0;
    }
    initialized = false;
}
} // namespace fluxgui
