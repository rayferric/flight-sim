#pragma once

#include "../pch.hpp"

#include "../gfx/colored_mesh.hpp"
#include "../gfx/shader.hpp"

class skybox {
public:
	void init(
	    const std::filesystem::path &shader_vert_path,
	    const std::filesystem::path &shader_frag_path
	) {
		skybox_shader.compile_from_file(shader_vert_path, shader_frag_path);
		screen_plane.load({
		    // bottom-left triangle
		    {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    // upper-right triangle
		    {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		    {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
		});
	}

	void draw() {
		skybox_shader.bind();
		screen_plane.draw(GL_TRIANGLES);
	}

private:
	shader       skybox_shader;
	colored_mesh screen_plane;
};
