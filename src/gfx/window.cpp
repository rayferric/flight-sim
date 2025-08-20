#include "window.hpp"

window::window() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

window::~window() {
	glfwTerminate();
}

void window::open(uint32_t w, uint32_t h, const std::string &title) {
	// create window
	glfw_window = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
	if (glfw_window == NULL) {
		glfwTerminate();
		throw std::runtime_error("glfwCreateWindow failed");
	}
	glfwMakeContextCurrent(glfw_window);

	// load gl
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		throw std::runtime_error("gladLoadGL failed");
	}

	// add this user ptr
	glfwSetWindowUserPointer(glfw_window, this);
}

void window::run_loop(const window::loop_info &info) {
	cur_loop_info = info;

	// hook window resize -> viewport resize
	uint32_t w, h;
	glfwGetFramebufferSize(glfw_window, (int *)&w, (int *)&h);
	window::fbsz_cb(glfw_window, w, h);
	glfwSetFramebufferSizeCallback(glfw_window, window::fbsz_cb);

	// main loop
	while (!glfwWindowShouldClose(glfw_window)) {
		glfwSwapBuffers(glfw_window);

		if (cur_loop_info.on_draw) {
			cur_loop_info.on_draw();
		}

		glfwPollEvents();

		if (cur_loop_info.on_update) {
			cur_loop_info.on_update();
		}
	}
}

bool window::is_glfw_key_down(int key) const {
	return glfwGetKey(glfw_window, key) == GLFW_PRESS;
}

// static
void window::fbsz_cb(GLFWwindow *glfw_window, int width, int height) {
	window *this_ptr =
	    static_cast<window *>(glfwGetWindowUserPointer(glfw_window));
	if (this_ptr->cur_loop_info.on_resize) {
		this_ptr->cur_loop_info.on_resize(width, height);
	}
}
