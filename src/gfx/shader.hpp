#pragma once

#include "../pch.hpp"

class shader {
public:
	shader();
	~shader();

	void compile(const std::string &vert_src, const std::string &frag_src);
	void compile_from_file(
	    const std::filesystem::path &vert_path,
	    const std::filesystem::path &frag_path
	);
	void bind();

private:
	GLuint prog_id;
};
