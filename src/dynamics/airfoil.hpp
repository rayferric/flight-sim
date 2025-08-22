#pragma once

#include "../pch.hpp"

#include "curve.hpp"

class airfoil {
public:
	// lift
	curve cl_vs_aoa_curve;
	float max_sampled_stall_angle = 0.0f;
	float min_sampled_stall_angle = 0.0f;
	float max_sampled_cl          = 0.0f;
	float min_sampled_cl          = 0.0f;
	float sweep_deg;
	float flap_eff_per_deg;
	float slat_eff_per_deg;
	// drag
	float base_cd;
	float cd_aoa2_scale;
	float flap_cd_eff_per_deg;
	float slat_cd_eff_per_deg;

	struct coeffs {
		float cl; // lift coefficient
		float cd; // drag coefficient
	};

	airfoil() = default;

	airfoil(
	    const curve &cl_vs_aoa_curve,
	    float        curve_max_cl        = 2.4f,
	    float        curve_max_aoa_deg   = 30.0f,
	    float        sweep_deg           = 40.0f,
	    float        flap_eff_per_deg    = 0.015f,
	    float        slat_eff_per_deg    = 0.015f,
	    float        base_cd             = 0.02f,
	    float        cd_aoa2_scale       = 0.0002f,
	    float        flap_cd_eff_per_deg = 0.001f,
	    float        slat_cd_eff_per_deg = 0.001f
	) {
		// lift
		this->cl_vs_aoa_curve = cl_vs_aoa_curve;
		this->cl_vs_aoa_curve.set_x_range(
		    -curve_max_aoa_deg, curve_max_aoa_deg
		);
		this->cl_vs_aoa_curve.set_y_range(-curve_max_cl, curve_max_cl);
		for (float x = -curve_max_aoa_deg; x <= curve_max_aoa_deg; x += 0.1f) {
			float y = this->cl_vs_aoa_curve.sample(x);
			if (y > this->max_sampled_cl) {
				this->max_sampled_cl          = y;
				this->max_sampled_stall_angle = x;
			}
			if (y < this->min_sampled_cl) {
				this->min_sampled_cl          = y;
				this->min_sampled_stall_angle = x;
			}
		}
		this->sweep_deg        = sweep_deg;
		this->flap_eff_per_deg = flap_eff_per_deg;
		this->slat_eff_per_deg = slat_eff_per_deg;
		// drag
		this->base_cd             = base_cd;
		this->cd_aoa2_scale       = cd_aoa2_scale;
		this->flap_cd_eff_per_deg = flap_cd_eff_per_deg;
		this->slat_cd_eff_per_deg = slat_cd_eff_per_deg;
	}

	coeffs
	calc_coeffs(float aoa_deg, float flap_deg = 0.0f, float slat_deg = 0.0f) {
		// sanity checks
		aoa_deg =
		    glm::clamp(aoa_deg, cl_vs_aoa_curve.x_min, cl_vs_aoa_curve.x_max);
		if (flap_deg < -45.0f || flap_deg > 45.0f) {
			throw std::invalid_argument(
			    "flap_deg must be in [-45, 45] degrees"
			);
		}
		if (slat_deg < 0.0f || slat_deg > 45.0f) {
			throw std::invalid_argument("slat_deg must be in [0, 45] degrees");
		}

		// lift

		// slat effect
		float d_cl_max    = slat_eff_per_deg * slat_deg;
		float cl_max      = cl_vs_aoa_curve.y_max;
		float curve_scale = (cl_max + d_cl_max) / cl_max; // [1, 1.something]

		float cl = cl_vs_aoa_curve.sample(aoa_deg / curve_scale) * curve_scale;

		// flap effect
		float flap_eff_factor;
		if (aoa_deg > 0.0f) {
			flap_eff_factor = 1.0f - glm::smoothstep(
			                             max_sampled_stall_angle,
			                             this->cl_vs_aoa_curve.x_max,
			                             aoa_deg
			                         );
		} else {
			flap_eff_factor = glm::smoothstep(
			    this->cl_vs_aoa_curve.x_min, min_sampled_stall_angle, aoa_deg
			);
		}
		cl += flap_eff_factor * flap_eff_per_deg * flap_deg;

		// sweep effect
		if (this->sweep_deg > 0.0f) {
			cl *= glm::cos(glm::radians(this->sweep_deg));
		}

		// drag

		float cd  = base_cd + cd_aoa2_scale * aoa_deg * aoa_deg;
		cd       += flap_cd_eff_per_deg * std::abs(flap_deg);
		cd       += slat_cd_eff_per_deg * std::abs(slat_deg);
		cd        = glm::clamp(cd, base_cd, 1.5f);

		return {cl, cd};
	}
};
