#pragma once

#include "../pch.hpp"

class colored_mesh {
public:
	struct vertex {
		float pos[3];
		float color[3];
	};

	colored_mesh();
	~colored_mesh();

	void load(const std::vector<vertex> &data);
	void draw(GLenum draw_mode = GL_LINES);

private:
	GLuint vao;
	GLuint vbo;
	size_t num_verts;
};
