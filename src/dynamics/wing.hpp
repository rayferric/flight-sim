#pragma once

#include "../pch.hpp"

#include "airfoil.hpp"

struct wing_section {
	float span;
	float chord;
	float chordwise_shift = 0.0f;
	// ^ backward shift relative to previous section
	bool has_aileron = false;
	bool has_flap    = false;
	// ^ has_flap cannot be paired with has_aileron
	bool has_slat = false;
};

struct wing_force_vec {
	float force            = 0.0f;
	float origin_spanwise  = 0.0f; // towards the outside of the wing
	float origin_chordwise = 0.0f; // towards the back of the wing
};

struct wing_forces {
	std::vector<wing_force_vec> sectional_lift;
	std::vector<wing_force_vec> sectional_drag;
	wing_force_vec              induced_drag;
};

struct wing_speed_aoa {
	float speed = 0.0f; // m/s
	float aoa   = 0.0f; // degrees
};

class wing {
public:
	airfoil                   airfoil_;
	std::vector<wing_section> sections;
	float                     span_efficiency; // for induced drag

	wing() = default;

	wing(
	    const airfoil                   &airfoil,
	    const std::vector<wing_section> &sections,
	    float                            span_efficiency = 0.85f
	) {
		this->airfoil_        = airfoil;
		this->sections        = sections;
		this->span_efficiency = span_efficiency;

		// sanity check has_aileron and has_flap
		for (size_t i = 0; i < sections.size(); ++i) {
			if (sections[i].has_aileron && sections[i].has_flap) {
				throw std::invalid_argument(
				    "section " + std::to_string(i) +
				    " cannot have both aileron and flap"
				);
			}
		}
	}

	wing_forces calc_forces(
	    float speed,
	    float aoa_deg,
	    float aileron_deg = 0.0f,
	    float flap_deg    = 0.0f,
	    float slat_deg    = 0.0f,
	    float air_density = 1.225f
	) {
		std::vector<wing_speed_aoa> speed_aoa(
		    sections.size(), {speed, aoa_deg}
		);
		return calc_forces(
		    speed_aoa, aileron_deg, flap_deg, slat_deg, air_density
		);
	}

	wing_forces calc_forces(
	    const std::vector<wing_speed_aoa> &speed_aoa,
	    float                              aileron_deg = 0.0f,
	    float                              flap_deg    = 0.0f,
	    float                              slat_deg    = 0.0f,
	    float                              air_density = 1.225f
	) {
		wing_forces forces;
		forces.sectional_lift.reserve(sections.size());
		forces.sectional_drag.reserve(sections.size());

		// sectional forces

		float cumulative_span = 0.0f;
		float chordwise_shift = 0.0f;
		for (size_t i = 0; i < sections.size(); ++i) {
			const wing_section &sec = sections[i];
			auto [cl, cd]           = airfoil_.calc_coeffs(
                speed_aoa[i].aoa,
                (static_cast<int>(sec.has_aileron) * aileron_deg +
                 static_cast<int>(sec.has_flap) * flap_deg),
                static_cast<int>(sec.has_slat) * slat_deg
            );

			float area  = sec.span * sec.chord;
			float speed = speed_aoa[i].speed;
			float lift  = cl * (air_density * speed * speed * 0.5f) * area;
			float drag  = cd * (air_density * speed * speed * 0.5f) * area;

			cumulative_span += sec.span;
			chordwise_shift += sec.chordwise_shift;

			wing_force_vec lift_vec;
			lift_vec.force            = lift;
			lift_vec.origin_spanwise  = cumulative_span - sec.span * 0.5f;
			lift_vec.origin_chordwise = chordwise_shift - sec.chord * 0.25f;
			// ^ lift vector at 25% from leading edge chordwise, chordwise pos=0
			// is middle

			wing_force_vec drag_vec;
			drag_vec.force            = drag;
			drag_vec.origin_spanwise  = cumulative_span - sec.span * 0.5f;
			drag_vec.origin_chordwise = chordwise_shift;
			// ^ drag vector halfway through chord, chordwise pos=0 is middle

			// add to sectional forces
			forces.sectional_lift.push_back(lift_vec);
			forces.sectional_drag.push_back(drag_vec);
		}

		// induced drag

		// aspect ratio
		float mean_chord = 0.0f;
		for (const wing_section &sec : sections) {
			mean_chord += sec.chord * sec.span; // weighted by span
		}
		mean_chord         /= cumulative_span;
		float aspect_ratio  = cumulative_span / mean_chord;

		// weighted mean CL and speed
		float mean_cl    = 0.0f;
		float mean_speed = 0.0f;
		float total_area = 0.0f;
		for (size_t i = 0; i < sections.size(); ++i) {
			float sec_area = sections[i].span * sections[i].chord;
			float speed    = speed_aoa[i].speed;
			float sec_cl   = forces.sectional_lift[i].force /
			               (air_density * speed * speed * 0.5f) / sec_area;
			if (std::isnan(sec_cl)) {
				sec_cl = 0.0f;
			}
			mean_cl    += sec_cl * sec_area;
			mean_speed += speed * sec_area;
			total_area += sec_area;
		}
		mean_cl    /= total_area;
		mean_speed /= total_area;

		// sweep effect
		float cos_sweep = std::cos(glm::radians(airfoil_.sweep_deg));
		float effective_aspect_ratio = aspect_ratio * cos_sweep * cos_sweep;

		// ^ the aspect ratio is for just a single wing, not the whole pair
		// the drag coefficient formula is for the full wing span
		// let's assume that there's a symmetric pair of wings:
		effective_aspect_ratio *= 2.0f;
		// we should also include fuselage width in the span
		// don't have that info here, but let's approximate with 1.5 for fighter
		// jets
		effective_aspect_ratio *= 1.5f;

		// drag coefficient
		float cd = mean_cl * mean_cl /
		           (M_PI * effective_aspect_ratio * span_efficiency);
		float induced_drag =
		    cd * (air_density * mean_speed * mean_speed * 0.5f) * total_area;

		// this drag force is for a full wing span
		// for just this left/right wing it would be two times smaller
		induced_drag /= 2.0f;

		forces.induced_drag.force = induced_drag;
		forces.induced_drag.origin_spanwise =
		    cumulative_span * 0.5f; // at the middle of the wing span
		forces.induced_drag.origin_chordwise =
		    (forces.sectional_drag.front().origin_chordwise +
		     forces.sectional_drag.back().origin_chordwise) *
		    0.5f; // average first and last section origin

		return forces;
	}
};
