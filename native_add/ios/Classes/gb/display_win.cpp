#include "std_include.hpp"
#include "display_win.hpp"
#include "utils/utils.hpp"

display_win::display_win()
{
	this->window_thread_ = std::thread([this]
	{
		this->window_runner();
	});

	while (!this->working())
	{
		std::this_thread::sleep_for(1ms);
	}
}

void display_win::window_runner()
{
	WNDCLASSEX wc;
	zero_object(wc);
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = display_win::window_proc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(102));
	wc.hIconSm = wc.hIcon;
	wc.hbrBackground = HBRUSH(COLOR_WINDOW);
	wc.lpszClassName = L"GBAWindow";
	RegisterClassEx(&wc);

	const int scale = 3;

	const int width = GB_WIDTH * scale;
	const int height = GB_HEIGHT * scale;

	this->window_ = CreateWindowExA(NULL, "GBAWindow", "GB-EMU", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
	                                CW_USEDEFAULT, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	SetWindowLongPtrA(this->window_, GWLP_USERDATA, LONG_PTR(this));

	MSG msg;
	while (this->working())
	{
		if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			std::this_thread::sleep_for(1ms);
		}
	}
}

void display_win::draw_frame(const color* buffer)
{
	RECT rect;
	GetClientRect(this->window_, &rect);

	const HDC hdc = GetDC(this->window_);
	const HDC src = CreateCompatibleDC(hdc);
	const HBITMAP map = CreateBitmap(GB_WIDTH, GB_HEIGHT, 1, 8 * 4, buffer);

	SelectObject(src, map);
	StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, src, 0, 0, GB_WIDTH, GB_HEIGHT, SRCCOPY);

	DeleteDC(src);
	DeleteObject(map);
	ReleaseDC(this->window_, hdc);
}

bool display_win::is_on()
{
	return this->working();
}

bool display_win::working() const
{
	return (IsWindow(this->window_) != FALSE);
}

LRESULT display_win::window_proc(const UINT message, const WPARAM w_param, const LPARAM l_param) const
{
	switch (message)
	{
	case WM_SIZE:
		{
			//this->render_texture();
			break;
		}

	case WM_KILL_WINDOW:
		{
			DestroyWindow(this->window_);
			return 0;
		}

	default: break;
	}

	return DefWindowProc(this->window_, message, w_param, l_param);
}

LRESULT CALLBACK display_win::window_proc(const HWND h_wnd, const UINT message, const WPARAM w_param,
                                          const LPARAM l_param)
{
	auto* gpu_ = reinterpret_cast<display_win*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA));
	if (gpu_) return gpu_->window_proc(message, w_param, l_param);
	return DefWindowProc(h_wnd, message, w_param, l_param);
}

display_win::~display_win()
{
	this->close_window();
	if (this->window_thread_.joinable()) this->window_thread_.join();
}

void display_win::close_window()
{
	if (this->working())
	{
		SendMessageA(this->window_, WM_KILL_WINDOW, NULL, NULL);
		this->window_ = nullptr;
	}
}

void display_win::set_title(std::string title)
{
	if (this->working()) SetWindowTextA(this->window_, utils::va("GB-EMU - %s", title.data()));
}

bool display_win::is_window_active() const
{
	return this->window_ && GetForegroundWindow() == this->window_;
}
