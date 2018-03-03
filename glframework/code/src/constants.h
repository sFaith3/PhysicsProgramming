#pragma once
#include <glm/vec3.hpp>

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {
	// EMITTER
	extern const int   MIN_RATE_PARTICLES_EMITTER;
	extern const int   MAX_RATE_PARTICLES_EMITTER;
	extern const float MIN_TIME_PARTICLE_LIFE;
	extern const float MAX_TIME_PARTICLE_LIFE;

	// FOUNTAIN
	extern const float MIN_ANGLE_FOUNTAIN;
	extern const float MAX_ANGLE_FOUNTAIN;
	extern const int   NUM_PARTICLES_FOUNTAIN;
	extern const float MIN_RADIUS_FOUNTAIN;
	extern const float MAX_RADIUS_FOUNTAIN;

	// CASCADE
	extern const int NUM_PARTICLES_CASCADE; // For the first wave at the pos from A to B

	// PARTICLES
	extern const int	   MAX_BUFFER_PARTICLES;
	extern const glm::vec3 DEFAULT_DIR_PARTICLES;
	extern const glm::vec3 DEFAULT_SPEED_PARTICLES;

	// COLLIDER CAPSULE
	extern const glm::vec3 DEFAULT_POS_A_CAPSULE;
	extern const glm::vec3 DEFAULT_POS_B_CAPSULE;
	extern const float	   MIN_RADIUS_CAPSULE;
	extern const float	   MAX_RADIUS_CAPSULE;
	extern const float	   DEFAULT_RADIUS_CAPSULE;

	// FORCES
	extern const glm::vec3 GRAVITY;

	extern const int FACES_CUBE;

	extern const float MAX_ANGLE_CIRCUM;
}
#endif