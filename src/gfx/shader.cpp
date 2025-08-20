#include "shader.hpp"

static std::string load_text(const std::filesystem::path &path) {
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const auto    sz = std::filesystem::file_size(path);

	std::string str(sz, '\0');
	f.read(str.data(), sz);

	return str;
}

// --- impl ---

shader::shader() {
	prog_id = glCreateProgram();
}
shader::~shader() {
	glDeleteProgram(prog_id);
}

void shader::compile(const std::string &vert_src, const std::string &frag_src) {
	GLuint v_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint f_id = glCreateShader(GL_FRAGMENT_SHADER);

	const char *v_str = vert_src.c_str();
	const char *f_str = frag_src.c_str();
	const GLint v_len = vert_src.size();
	const GLint f_len = frag_src.size();

	glad_glShaderSource(v_id, 1, &v_str, &v_len);
	glShaderSource(f_id, 1, &f_str, &f_len);

	glCompileShader(v_id);
	glCompileShader(f_id);

	glAttachShader(prog_id, v_id);
	glAttachShader(prog_id, f_id);
	glLinkProgram(prog_id);

	GLint success;
	glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar info[512];
		glGetProgramInfoLog(prog_id, 512, NULL, info);
		throw std::runtime_error(
		    "glLinkProgram failed:\n" + std::string(info) + "\n"
		);
	}

	glDeleteShader(f_id);
	glDeleteShader(v_id);
}

void shader::compile_from_file(
    const std::filesystem::path &vert_path,
    const std::filesystem::path &frag_path
) {
	std::string vert_src = load_text(vert_path);
	std::string frag_src = load_text(frag_path);
	return compile(vert_src, frag_src);
}

void shader::bind() {
	glUseProgram(prog_id);
}
