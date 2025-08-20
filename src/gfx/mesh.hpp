#pragma once

#include "../pch.hpp"

class mesh {
public:
	struct vertex {
		float pos[3];
		float uv[2];
		float norm[3];
		float tangent[3];
	};

	mesh();
	~mesh();

	void load(const std::vector<vertex> &data);
	void load_from_file(const std::filesystem::path &path);
	void draw(GLenum draw_mode = GL_TRIANGLES);

private:
	GLuint vao;
	GLuint vbo;
	size_t num_verts;
};
