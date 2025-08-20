#pragma once

#include "../pch.hpp"

class curve {
public:
	float x_min = 0.0f, x_max = 1.0f;
	float y_min = 0.0f, y_max = 1.0f;

	void load_from_file(const std::filesystem::path &path) {
		// structure:
		// <num control pts> <num samples>
		// first control pt: 1.0 1.3
		// ...
		// first sample: 0.01
		// ...

		// read file
		std::ifstream file(path);
		if (!file.is_open()) {
			throw std::runtime_error(
			    "Failed to open curve file: " + path.string()
			);
		}
		size_t num_control_pts, num_samples;
		file >> num_control_pts >> num_samples;
		// skip control points
		for (size_t i = 0; i < num_control_pts; ++i) {
			float x, y;
			file >> x >> y;
		}
		// load samples
		y_data.resize(num_samples);
		for (size_t i = 0; i < num_samples; ++i) {
			file >> y_data[i];
		}
		file.close();
	}

	float sample(float x) {
		if (y_data.empty()) {
			throw std::runtime_error("Curve data is empty.");
		}
		x = glm::clamp(x, x_min, x_max);
		x = (x - x_min) / (x_max - x_min); // normalize to [0, 1]

		// Perform sampling (linear interpolation, etc.)
		size_t idx_low  = static_cast<size_t>(x * (y_data.size() - 1));
		size_t idx_high = glm::min(idx_low + 1, y_data.size() - 1);
		float  t        = (x * (y_data.size() - 1)) - idx_low;
		float  val01    = glm::mix(y_data[idx_low], y_data[idx_high], t);

		return val01 * (y_max - y_min) + y_min; // denormalize
	}

	void set_x_range(float x_min, float x_max) {
		this->x_min = x_min;
		this->x_max = x_max;
	}

	void set_y_range(float y_min, float y_max) {
		this->y_min = y_min;
		this->y_max = y_max;
	}

private:
	std::vector<float> y_data;
};
