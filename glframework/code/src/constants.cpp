#include "constants.h"

namespace Constants {
	// EMITTER
	extern const int   MIN_RATE_PARTICLES_EMITTER(100);
	extern const int   MAX_RATE_PARTICLES_EMITTER(500);
	extern const float MIN_TIME_PARTICLE_LIFE(15.0f);
	extern const float MAX_TIME_PARTICLE_LIFE(60.0f);

	// FOUNTAIN
	extern const float MIN_ANGLE_FOUNTAIN(30.0f);
	extern const float MAX_ANGLE_FOUNTAIN(75.0f);
	extern const int   NUM_PARTICLES_FOUNTAIN(10);
	extern const float MAX_RADIUS_FOUNTAIN(7.0f);

	// CASCADE
	extern const int NUM_PARTICLES_CASCADE(20);

	// PARTICLES
	extern const glm::vec3 DEFAULT_DIR_PARTICLES(0.0f, 1.0f, 0.0f);
	extern const glm::vec3 DEFAULT_SPEED_PARTICLES(1.0f, 1.0f, 1.0f);

	// FORCES
	extern const glm::vec3 GRAVITY(0.0f, -9.81f, 0.0f);

	extern const int FACES_CUBE(6);
}