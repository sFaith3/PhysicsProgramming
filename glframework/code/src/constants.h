#pragma once
#include <glm/vec3.hpp>

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {
	// EMITTER
	extern const int MINIMUM_RATE_PARTICLES_EMITTER;
	extern const int MAXIMUM_RATE_PARTICLES_EMITTER;
	extern const float MINIMUM_TIME_PARTICLE_LIFE;
	extern const float MAXIMUM_TIME_PARTICLE_LIFE;

	// FOUNTAIN
	extern const float MINIMUM_ANGLE_FOUNTAIN;
	extern const float MAXIMUM_ANGLE_FOUNTAIN;
	extern const int MAXIMUM_PARTICLES_FOUNTAIN;
	extern const float MAXIMUM_RADIUS_FOUNTAIN;
	// CASCADE
	extern const int MAXIMUM_PARTICLES_CASCADE; // For the first wave at the pos from A to B

	// PARTICLES
	extern const int MAX_BUFFER_PARTICLES;
	extern const glm::vec3 INITIAL_SPEED_PARTICLES;

	// FORCES
	extern const glm::vec3 GRAVITY;

	extern const int FACES_CUBE;
}
#endif