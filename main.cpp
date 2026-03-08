#include <Windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

Image* crosshair;
ULONG_PTR gdiplusToken;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
        case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
			Graphics graphics(hdc);
			graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
			graphics.DrawImage(crosshair, 0, 0);
			EndPaint(hwnd, &ps);
			break;
        }
		default:
			return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	crosshair = new Image(L"crosshair.png");

	UINT crosshairWidth = crosshair->GetWidth();
	UINT crosshairHeight = crosshair->GetHeight();

	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"CrosshairOverlayClass";
	RegisterClassExW(&wc);

	int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
	int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

	HWND hwnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		L"",
		WS_POPUP | WS_VISIBLE,
		SCREEN_WIDTH / 2 - crosshairHeight / 6,
		SCREEN_HEIGHT / 2 - crosshairHeight / 6,
		crosshairWidth, crosshairHeight,
		nullptr, nullptr, 
		wc.hInstance, 
		nullptr
	);
	if (!hwnd) return 1;
	LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
	SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	delete crosshair;
	GdiplusShutdown(gdiplusToken);

	return 0;
}