#pragma once

#include "../pch.hpp"

class window {
public:
	struct loop_info {
		std::function<void()>                   on_draw   = nullptr;
		std::function<void()>                   on_update = nullptr;
		std::function<void(uint32_t, uint32_t)> on_resize = nullptr;
	};

	GLFWwindow *glfw_window;

	window();
	~window();

	void open(uint32_t w, uint32_t h, const std::string &title);
	void run_loop(const loop_info &info);

	bool is_glfw_key_down(int key) const;

private:
	window::loop_info cur_loop_info;

	static void fbsz_cb(GLFWwindow *window, int width, int height);
};
