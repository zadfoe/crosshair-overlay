#include <Windows.h>
#include <gdiplus.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <cstring>

using namespace Gdiplus;

HINSTANCE hInstance;
Image* crosshairImg;
HWND hCrosshairWnd;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK CrosshairWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			//PostQuitMessage(0);
			break;
        case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

			Graphics graphics(hdc);
			graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
			graphics.DrawImage(crosshairImg, 0, 0);

			EndPaint(hWnd, &ps);

			break;
        }
		default:
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void CALLBACK WinSwitchEventProc(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD); // DECLARATION

HWND NewCrosshairWnd(HWND wndToOverlay) {
	UINT crosshairWidth = crosshairImg->GetWidth();
	UINT crosshairHeight = crosshairImg->GetHeight();

	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.lpfnWndProc = CrosshairWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"CrosshairOverlayClass";
	RegisterClassExW(&wc);

	int centerX, centerY;

	if (wndToOverlay) {
		RECT rect;
		GetClientRect(wndToOverlay, &rect);
		MapWindowPoints(wndToOverlay, nullptr, (LPPOINT)&rect, 2);
		centerX = (rect.left + rect.right) / 2;
		centerY = (rect.top + rect.bottom) / 2;
	}
	else {
		centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
		centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
	}

	HWND hWnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		L"",
		WS_POPUP | WS_VISIBLE,
		centerX - crosshairHeight / 6,
		centerY - crosshairHeight / 6,
		crosshairWidth, crosshairHeight,
		nullptr, nullptr,
		wc.hInstance,
		nullptr
	);
	if (!hWnd) return 0;
	LONG exStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
	SetWindowLongW(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	return hWnd;
}

void CALLBACK WinSwitchEventProc(
	HWINEVENTHOOK hWinEventHook,
	DWORD event,
	HWND hwnd,
	LONG idObject,
	LONG idChild,
	DWORD dwEventThread,
	DWORD dwmsEventTime)
{
	char newWndClass[256];
	char crosshairWndClass[256];

	GetClassNameA(hwnd, newWndClass, sizeof(newWndClass));
	GetClassNameA(hCrosshairWnd, crosshairWndClass, sizeof(crosshairWndClass));

	//std::cout << "Foreground window is now: " << newWndClass << '\n';
	
	// if the newly selected window is not the crosshair itself
	if (strcmp(newWndClass, crosshairWndClass) != 0) {
		PostMessageW(hCrosshairWnd, WM_CLOSE, 0, 0);
		hCrosshairWnd = NewCrosshairWnd(hwnd);
	}
}

INT APIENTRY WinMain(
	_In_ HINSTANCE h, 
	_In_opt_ HINSTANCE, 
	_In_ LPSTR lpCmdLine, 
	_In_ int nShowCmd)
{
	hInstance = h;

	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	crosshairImg = new Image(L"crosshair.png");

	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);

	std::cout 
		<< "The crosshair does not adjust when you press the fullscreen button on your active window. Drag the window when fullscreening instead.\n"
		<< "Close this console's window, not the crosshair's, to end the program.\n";

	SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinSwitchEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, NULL, WinSwitchEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	SetWinEventHook(EVENT_SYSTEM_MINIMIZEEND, EVENT_SYSTEM_MINIMIZEEND, NULL, WinSwitchEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

	hCrosshairWnd = NewCrosshairWnd(0);
	if (!hCrosshairWnd) return 1;

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	delete crosshairImg;
	GdiplusShutdown(gdiplusToken);

	return 0;
}