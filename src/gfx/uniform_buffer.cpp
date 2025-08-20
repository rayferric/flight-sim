#include "uniform_buffer.hpp"

uniform_buffer::uniform_buffer() {
	glGenBuffers(1, &gl_id);
}

uniform_buffer::~uniform_buffer() {
	glDeleteBuffers(1, &gl_id);
}

void uniform_buffer::update(const std::vector<uint8_t> &data) {
	glBindBuffer(GL_UNIFORM_BUFFER, gl_id);
	if (data.size() != size) {
		size = data.size();
		glBufferData(GL_UNIFORM_BUFFER, size, data.data(), GL_DYNAMIC_DRAW);
	} else {
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data.data());
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void uniform_buffer::bind(uint32_t binding) {
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, gl_id);
}
