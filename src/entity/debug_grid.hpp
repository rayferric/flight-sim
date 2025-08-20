#pragma once

#include "../pch.hpp"

#include "../gfx/colored_mesh.hpp"
#include "../gfx/shader.hpp"
#include "../gfx/uniform_buffer.hpp"

class debug_grid {
public:
	void init(
	    const std::string &shader_vert_path,
	    const std::string &shader_frag_path,
	    float              render_distance = 100.0f,
	    float              gizmo_spacing   = 20.0f,
	    float              axis_length     = 1.0f
	) {
		// mesh.load_from_file(section_mesh_path);
		grid_shader.compile_from_file(shader_vert_path, shader_frag_path);
		this->render_distance = render_distance;

		// generate a bunch of gizmos
		std::vector<colored_mesh::vertex> verts;
		float section_width = 2.0f * render_distance;
		for (float x = -section_width; x <= section_width; x += gizmo_spacing) {
			for (float y  = -section_width; y <= section_width;
			     y       += gizmo_spacing) {
				for (float z  = -section_width; z <= section_width;
				     z       += gizmo_spacing) {
					// add a 3-axis gizmo
					for (int i = 0; i < 3; ++i) {
						colored_mesh::vertex v1, v2;
						v1.pos[0] = v2.pos[0] = x;
						v1.pos[1] = v2.pos[1] = y;
						v1.pos[2] = v2.pos[2]  = z;
						v2.pos[i]             += axis_length;

						// colors
						v1.color[0] = v1.color[1] = v1.color[2] = 0.0;
						v2.color[0] = v2.color[1] = v2.color[2] = 0.0;
						v1.color[i] = v2.color[i] = 1.0;

						verts.push_back(v1);
						verts.push_back(v2);
					}
				}
			}
		}
		mesh.load(verts);

		grid_settings_ubo.update(render_distance);
	}

	void draw() {
		// set line thickness
		glLineWidth(3.0f);
		grid_shader.bind();
		model_ubo.bind(1);
		grid_settings_ubo.bind(2);
		mesh.draw();
	}

	void update_tiling_from_view_pos(const glm::vec3 &view_pos_flu) {
		// find the current position for the segment by quantizing the view
		// position down to half-section_width precision
		glm::vec3 quantized_pos =
		    glm::round(view_pos_flu / render_distance) * render_distance;
		glm::mat4 model_mat = glm::translate(glm::mat4(1), quantized_pos);
		model_ubo.update(model_mat);
	}

protected:
	uniform_buffer model_ubo, grid_settings_ubo;
	colored_mesh   mesh;
	shader         grid_shader;
	float          render_distance;
};
