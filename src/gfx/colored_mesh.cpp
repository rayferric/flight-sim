#include "colored_mesh.hpp"

colored_mesh::colored_mesh() {
	// gen array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// gen buf
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// setup attrib layout
	glVertexAttribPointer(
	    0, 3, GL_FLOAT, GL_FALSE, sizeof(colored_mesh::vertex), (GLvoid *)0
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
	    1,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof(colored_mesh::vertex),
	    (GLvoid *)(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
colored_mesh::~colored_mesh() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void colored_mesh::load(const std::vector<colored_mesh::vertex> &verts) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
	    GL_ARRAY_BUFFER,
	    verts.size() * sizeof(colored_mesh::vertex),
	    verts.data(),
	    GL_DYNAMIC_DRAW
	);
	num_verts = verts.size();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void colored_mesh::draw(GLenum draw_mode) {
	glBindVertexArray(vao);
	glDrawArrays(draw_mode, 0, num_verts);
}
