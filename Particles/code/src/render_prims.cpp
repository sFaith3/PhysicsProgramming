#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "constants.h"

// Boolean variables allow to show/hide the primitives
bool renderSphere = false;
bool renderCapsule = false;
bool renderParticles = true;
bool renderCloth = false;
bool renderCube = false;

int startDrawingFromParticle = 0;
int endIndexParticlesToDraw = 0;
extern int numParticlesEnabled;

namespace Sphere {
	glm::vec3 posSphere;
	float radiusSphere;
	extern void setupSphere(glm::vec3 pos, float radius);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius);
	extern void drawSphere();
}

namespace Capsule {
	glm::vec3 posACapsule;
	glm::vec3 posBCapsule;
	float radiusCapsule;
	extern void setupCapsule(glm::vec3 posA, glm::vec3 posB, float radius);
	extern void cleanupCapsule();
	extern void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius);
	extern void drawCapsule();
}

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

namespace Cube {
	extern void setupCube();
	extern void cleanupCube();
	extern void updateCube(const glm::mat4& transform);
	extern void drawCube();
}

void setupPrims() {
	Sphere::setupSphere(Sphere::posSphere, Sphere::radiusSphere);
	Capsule::setupCapsule(Capsule::posACapsule, Capsule::posBCapsule, Capsule::radiusCapsule);
	LilSpheres::setupParticles(LilSpheres::maxParticles);
	ClothMesh::setupClothMesh();
	Cube::setupCube();
}
void cleanupPrims() {
	Sphere::cleanupSphere();
	Capsule::cleanupCapsule();
	LilSpheres::cleanupParticles();
	ClothMesh::cleanupClothMesh();
	Cube::cleanupCube();
}

void renderPrims() {
	if (renderSphere)
		Sphere::drawSphere();
	if (renderCapsule)
		Capsule::drawCapsule();

	if (renderParticles) {
		if (endIndexParticlesToDraw - startDrawingFromParticle < 0) {
			LilSpheres::drawParticles(startDrawingFromParticle, Constants::MAX_BUFFER_PARTICLES - startDrawingFromParticle);
			LilSpheres::drawParticles(0, endIndexParticlesToDraw);
		}
		else {
			LilSpheres::drawParticles(startDrawingFromParticle, numParticlesEnabled);
		}
	}
	
	if (renderCloth)
		ClothMesh::drawClothMesh();

	if (renderCube)
		Cube::drawCube();
}
