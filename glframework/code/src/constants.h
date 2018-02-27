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

	extern const float INITIAL_SPEED_PARTICLES;

	extern const glm::vec3 GRAVITY;

	extern const int FACES_CUBE;
}
#endif