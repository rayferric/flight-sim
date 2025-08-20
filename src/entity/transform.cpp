#include "transform.hpp"

glm::mat4 transform::calc_mat4_flu_space() const {
	glm::mat4 combined_tf = glm::rotate(
	    glm::mat4(1), glm::radians(rpy_deg.x), glm::vec3(1.0f, 0.0f, 0.0f)
	);
	combined_tf =
	    glm::rotate(
	        glm::mat4(1), glm::radians(rpy_deg.y), glm::vec3(0.0f, 1.0f, 0.0f)
	    ) *
	    combined_tf;
	combined_tf =
	    glm::rotate(
	        glm::mat4(1), glm::radians(rpy_deg.z), glm::vec3(0.0f, 0.0f, 1.0f)
	    ) *
	    combined_tf;
	combined_tf = glm::translate(glm::mat4(1), pos_flu) * combined_tf;
	return combined_tf;
}

void transform::set_pose(const glm::vec3 &pos_flu, const glm::vec3 &rpy_deg) {
	this->pos_flu = pos_flu;
	this->rpy_deg = rpy_deg;
}

glm::vec3 transform::get_pos_flu() const {
	return pos_flu;
}

glm::vec3 transform::get_rpy_deg() const {
	return rpy_deg;
}
