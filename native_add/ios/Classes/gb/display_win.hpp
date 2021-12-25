#pragma once
#include "display.hpp"
#include <thread>
#include <Windows.h>

#define WM_KILL_WINDOW (WM_USER+0)


class display_win : public display
{
public:
	display_win();
	~display_win() override;

	bool is_on() override;

	void close_window();

	void draw_frame(const color* buffer) override;
	void set_title(std::string title) override;

	bool is_window_active() const;

private:
	HWND window_{};
	bool working() const;

	std::thread window_thread_;
	
	static LRESULT CALLBACK window_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
	LRESULT window_proc(UINT message, WPARAM w_param, LPARAM l_param) const;
	
	void window_runner();
};
