#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include "WindowPersist.h"

bool WindowPersist::IsValid() const
{
	return data[2] > 0 && data[3] > 0;
}

void WindowPersist::ApplyState(HWND hnd) const
{
	WINDOWPLACEMENT placement = {};
	placement.length = sizeof(WINDOWPLACEMENT);
	placement.rcNormalPosition.left = data[0];
	placement.rcNormalPosition.top = data[1];
	placement.rcNormalPosition.right = data[0] + data[2];
	placement.rcNormalPosition.bottom = data[1] + data[3];
	placement.showCmd = maximized ? SW_MAXIMIZE : 0;

	SetWindowPlacement(hnd, &placement);

	DWORD dwStyle = GetWindowLong(hnd, GWL_STYLE);
    if (fullscreen)
    {
        DWORD dwStyle = GetWindowLong(hnd, GWL_STYLE);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(MonitorFromWindow(hnd,
            MONITOR_DEFAULTTONEAREST), &mi)) {
            SetWindowLong(hnd, GWL_STYLE,
                dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
        SetWindowLong(hnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
    }
    else
        SetWindowLong(hnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
}

void WindowPersist::SaveState(HWND hnd)
{
    DWORD dwStyle = GetWindowLong(hnd, GWL_STYLE);
    fullscreen = (dwStyle & WS_OVERLAPPEDWINDOW) == 0;

    if (!fullscreen)
    {
        WINDOWPLACEMENT placement = {};
        placement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hnd, &placement);
        data[0] = placement.rcNormalPosition.left;
        data[1] = placement.rcNormalPosition.top;
        data[2] = placement.rcNormalPosition.right - data[0];
        data[3] = placement.rcNormalPosition.bottom - data[1];
        maximized = (placement.flags & WPF_RESTORETOMAXIMIZED) != 0;
    }
}

std::string WindowPersist::getAsString() const
{
    int val0 = maximized, val1 = fullscreen;

    char str[1024];
    sprintf_s(str, sizeof(str), "%d,%d,%d,%d|%d,%d", data[0], data[1], data[2], data[3], val0, val1);

    return str;
}

void WindowPersist::setFromString(const char* value)
{
    assert(value);

    WindowPersist r = *this;

    int val0 = r.maximized, val1 = r.fullscreen;

    if (sscanf_s(value, "%d,%d,%d,%d|%d,%d",
        &r.data[0], &r.data[1], &r.data[2], &r.data[3], &val0, &val1) == 6)
    {
        r.maximized = val0;
        r.fullscreen = val1;
        if(r.IsValid())
            *this = r;
    }
}


