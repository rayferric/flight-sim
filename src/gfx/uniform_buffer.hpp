#pragma once

#include "../pch.hpp"

class uniform_buffer {
public:
	uniform_buffer();
	~uniform_buffer();

	void update(const std::vector<uint8_t> &data);
	void bind(uint32_t binding);

	template <typename... Fields> void update(Fields... fields) {
		static_assert(
		    ((std::is_same_v<Fields, float> ||
		      std::is_same_v<Fields, glm::vec2> ||
		      std::is_same_v<Fields, glm::vec3> ||
		      std::is_same_v<Fields, glm::vec4> ||
		      std::is_same_v<Fields, glm::mat4>) &&
		     ...),
		    "Fields must be float or glm types."
		);

		std::vector<uint8_t> data;
		(data.insert(
		     data.end(),
		     reinterpret_cast<const uint8_t *>(&fields),
		     reinterpret_cast<const uint8_t *>(&fields) + sizeof(fields)
		 ),
		 ...);

		update(data);
	}

private:
	GLuint gl_id;
	size_t size = 0;
};
