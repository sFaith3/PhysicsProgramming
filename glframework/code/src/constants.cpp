#include "constants.h"

namespace Constants {
	// EMITTER
	extern const int MINIMUM_RATE_PARTICLES_EMITTER(100);
	extern const int MAXIMUM_RATE_PARTICLES_EMITTER(500);
	extern const float MINIMUM_TIME_PARTICLE_LIFE(15.0f);
	extern const float MAXIMUM_TIME_PARTICLE_LIFE(100.0f);

	// FOUNTAIN
	extern const float MINIMUM_ANGLE_FOUNTAIN(75.0f);
	extern const float MAXIMUM_ANGLE_FOUNTAIN(115.0f);

	// CASCADE
	extern const int MAXIMUM_PARTICLES_CASCADE(20);

	// PARTICLES
	extern const glm::vec3 INITIAL_SPEED_PARTICLES(2.0f, 2.0f, 2.0f);

	// FORCES
	extern const glm::vec3 GRAVITY(0.0f, -9.81f, 0.0f);

	extern const int FACES_CUBE(6);
}