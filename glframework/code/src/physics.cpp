#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <SDL2\SDL.h>
#include "constants.h"
#include <vector>

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

namespace Sphere {
	extern void setupSphere(glm::vec3 pos, float radius);
	extern void updateSphere(glm::vec3 pos, float radius);

}

namespace Box {
	extern float cubeVerts[];
}

enum class EnumTypeMovement {
	FOUNTAIN, CASCADE
};

struct Plane {
	glm::vec3 normal;
	float D;
};

extern const int MAX_BUFFER_PARTICLES(5000);

//bool show_test_window = false;

bool playingSimulation;

// EMITTER
int rateParticleEmitter;		//It must be >= 100
float particleLifeTimeEmitter;  //It must be >= 1.0
int currTypeEmitter;
glm::vec3 posEmitter;
glm::vec3 dirEmitter;
float angleEmitter;
glm::vec3 finalPosEmitter;
glm::vec3 initialSpeedEmitter;
int currNumParticlesCascade;

// ELASTICITY & FRICTION
float elasticCoef;
float frictionCoef;

// SPHERE COLLIDER
bool renderSphere;
extern glm::vec3 posSphere;
extern float radiusSphere;

// CAPSULE COLLIDER
bool useCapsuleCollider;
glm::vec3 posACapsule;
glm::vec3 posBCapsule;
float radiusCapsule;

// FORCES
bool useGravity;
glm::vec3 acceleration;

// PARTICLES
int numParticlesEnabled;
extern int startDrawingFromParticle;
extern int endIndexParticlesToDraw;

float particlesLifeTime[MAX_BUFFER_PARTICLES];
glm::vec3 *ptrPosParticles;	  // Posiciones de las partículas
glm::vec3 *ptrSpeedParticles; // Velocidades de las partículas

std::vector<Plane> planes;

// Forward Declarations
void PhysicsInit();
void PhysicsCleanup();


void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

	ImGui::Checkbox("Play Simulation", &playingSimulation);
	if (ImGui::Button("Reset Simulation")) {
		PhysicsCleanup();
		PhysicsInit();
	}

	/// EMITER
	if (ImGui::TreeNode("Emitter")) {
		ImGui::DragInt("Rate", &rateParticleEmitter, 1.0f, Constants::MINIMUM_RATE_PARTICLES_EMITTER, Constants::MAXIMUM_RATE_PARTICLES_EMITTER);
		ImGui::DragFloat("Particle life", &particleLifeTimeEmitter, 1.0f, Constants::MINIMUM_TIME_PARTICLE_LIFE, Constants::MAXIMUM_TIME_PARTICLE_LIFE);

		ImGui::RadioButton("Fountain", &currTypeEmitter, 0); ImGui::SameLine();
		ImGui::RadioButton("Cascade", &currTypeEmitter, 1);

		/// FOUNTAIN & CASCADE
		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN:
			ImGui::DragFloat3("Fountain Position", &posEmitter.x);
			ImGui::DragFloat3("Fountain Direction", &dirEmitter.x, 1.0f, -1.0f, 1.0f);
			ImGui::DragFloat("Fountain Angle", &angleEmitter, 1.0f, Constants::MINIMUM_ANGLE_FOUNTAIN, Constants::MAXIMUM_ANGLE_FOUNTAIN);
			break;

		case EnumTypeMovement::CASCADE:
			ImGui::DragFloat3("Cascade Position A", &posEmitter.x);
			ImGui::DragFloat3("Cascade Position B", &finalPosEmitter.x);
			ImGui::DragFloat3("Cascade Velocity", &initialSpeedEmitter.x);

			//currNumParticlesCascade = 0;
			break;

		default:
			break;
		}

		ImGui::TreePop();
	}

	/// ELASTICITY & FRICTION
	if (ImGui::TreeNode("Elasticity & Friction")) {
		ImGui::DragFloat("Elastic Coefficient", &elasticCoef, 1.0f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction Coefficient", &frictionCoef, 1.0f, 0.0f, 1.0f);

		ImGui::TreePop();
	}

	/// COLLIDERS
	if (ImGui::TreeNode("Colliders")) {
		// SPHERE
		ImGui::Checkbox("Use Sphere Collider", &renderSphere);
		ImGui::DragFloat3("Sphere Position", &posSphere.x);
		ImGui::DragFloat("Sphere Radius", &radiusSphere);

		// CAPSULE
		ImGui::Checkbox("Use Capsule Collider", &useCapsuleCollider);
		ImGui::DragFloat3("Capsule Position A", &posACapsule.x);
		ImGui::DragFloat3("Capsule Position B", &posBCapsule.x);
		ImGui::DragFloat("Capsule Radius", &radiusCapsule);

		ImGui::TreePop();
	}

	/// FORCES
	if (ImGui::TreeNode("Forces")) {
		ImGui::Checkbox("Use Gravity", &useGravity);
		ImGui::DragFloat3("Acceleration", &acceleration.x);

		ImGui::TreePop();
	}

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	/*if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}*/
}

void InitEmitter() {
	rateParticleEmitter = Constants::MINIMUM_RATE_PARTICLES_EMITTER;
	particleLifeTimeEmitter = Constants::MINIMUM_TIME_PARTICLE_LIFE;
	currTypeEmitter = (int)EnumTypeMovement::FOUNTAIN;
	posEmitter = { 0.0f, 2.5f, 0.0f };
	dirEmitter = { 0.0f, 0.3f, 0.3f };
	angleEmitter = Constants::MINIMUM_ANGLE_FOUNTAIN;
	finalPosEmitter = { 2.0f, 2.5f, 0.0f };
	currNumParticlesCascade = 0;
	initialSpeedEmitter = Constants::INITIAL_SPEED_PARTICLES;
}

void InitColliders() {
	// Sphere
	renderSphere = false;
	posSphere = { 0.0f, 0.0f, 0.0f };
	radiusSphere = 1.0f;

	// Capsule
	useCapsuleCollider = false;
	posACapsule = { 0.0f, 0.0f, 0.0f };
	posBCapsule = { 0.0f, 0.0f, 0.0f };
	radiusCapsule = 1.0f;
}

/*
 * D = -(Ax + By + Cz)
 */
void InitPlanes() {
	Plane newPlane;

	// Plane 1
	newPlane.normal = glm::vec3(0.f, -1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 10.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 2
	newPlane.normal = glm::vec3(0.f, 1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);

	// Plane 3
	newPlane.normal = glm::vec3(-1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 4
	newPlane.normal = glm::vec3(1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);

	// Plane 5
	newPlane.normal = glm::vec3(0.f, 0.f, -1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 6
	newPlane.normal = glm::vec3(0.f, 0.f, 1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);
}

void PhysicsInit() {
	playingSimulation = false;

	InitEmitter();

	// ELASTICITY & FRICTION
	elasticCoef = 1.0f;
	frictionCoef = 0.0f;

	InitColliders();

	// FORCES
	useGravity = true;
	acceleration = Constants::GRAVITY;

	// DATA FOR PARTICLES
	endIndexParticlesToDraw = 0;
	startDrawingFromParticle = 0;
	numParticlesEnabled = 0;

	ptrPosParticles = new glm::vec3[MAX_BUFFER_PARTICLES]{ glm::vec3(0.0f, 0.0f, 0.0f) };
	ptrSpeedParticles = new glm::vec3[MAX_BUFFER_PARTICLES]{ acceleration };

	InitPlanes();
}

void NewParticle(int currTypeEmitter) {
	if (numParticlesEnabled < MAX_BUFFER_PARTICLES - 1) {
		particlesLifeTime[endIndexParticlesToDraw] = particleLifeTimeEmitter;

		glm::vec3 initialDir;
		glm::vec3 newPos;

		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN: // TODO
			initialDir.x = dirEmitter.x * cos(angleEmitter);
			initialDir.y = dirEmitter.y * sin(angleEmitter);
			initialDir.z = dirEmitter.z;// * tan(angleEmitter);

			ptrPosParticles[endIndexParticlesToDraw] = posEmitter;
			ptrSpeedParticles[endIndexParticlesToDraw] = initialSpeedEmitter * initialDir;

			break;

		case EnumTypeMovement::CASCADE:
			newPos.x = (finalPosEmitter.x - posEmitter.x) / Constants::MAXIMUM_PARTICLES_CASCADE;
			newPos.y = (finalPosEmitter.y - posEmitter.y) / Constants::MAXIMUM_PARTICLES_CASCADE;
			newPos.z = (finalPosEmitter.z - posEmitter.z) / Constants::MAXIMUM_PARTICLES_CASCADE;

			currNumParticlesCascade = (currNumParticlesCascade + 1) % Constants::MAXIMUM_PARTICLES_CASCADE;
			newPos *= (currNumParticlesCascade + 1);
			newPos += posEmitter;

			ptrPosParticles[endIndexParticlesToDraw] = newPos;
			ptrSpeedParticles[endIndexParticlesToDraw] = initialSpeedEmitter;

			break;

		default:
			break;
		}
		
		endIndexParticlesToDraw = (endIndexParticlesToDraw + 1) % MAX_BUFFER_PARTICLES;
		numParticlesEnabled++;
	}
}

void UpdateEmitter(float dt) {
	// Spawn at specified time
	int numParticlesToEnable = static_cast<int>(dt * rateParticleEmitter);
	for (int i = 0; i < numParticlesToEnable && numParticlesEnabled < MAX_BUFFER_PARTICLES - 1; i++) {
		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN:
			NewParticle(currTypeEmitter);
			break;

		case EnumTypeMovement::CASCADE:
			for (int i = 0; i < Constants::MAXIMUM_PARTICLES_CASCADE && numParticlesEnabled < MAX_BUFFER_PARTICLES - 1; i++) {
				NewParticle(currTypeEmitter);
			}
			break;

		default:
			break;
		}
	}
}

void DeleteLastParticle() {
	startDrawingFromParticle = (startDrawingFromParticle + 1) % MAX_BUFFER_PARTICLES;
	numParticlesEnabled--;
}

bool IsParticleAlive(int currParticle, float dt) {
	if (particlesLifeTime[currParticle] < 0.0f) {
		DeleteLastParticle();
		return false;
	}
	else {
		particlesLifeTime[currParticle] -= dt;
		return true;
	}
}

void UpdateCollisions(int currParticle) {
	// WALLS


	// GROUND PLANES


	// SPHERE


	// CAPSULE

}

void CheckCollision(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	glm::vec3 n, vn, vt;
	Plane currPlane;
	float d;

	for (int i = 0; i < Constants::FACES_CUBE; i++) {
		currPlane = planes[i];
		n = currPlane.normal;
		d = (glm::dot(n, p) + currPlane.D) * (glm::dot(n, p0) + currPlane.D);

		/// Collision Particle - Plane
		if (d < 0.0f) {
			p0 = p;
			v0 = v;

			// With Elasticity
			p = p0 - (1 + elasticCoef) * (glm::dot(n, p0) + currPlane.D) * n;
			v = v0 - (1 + elasticCoef) * glm::dot(n, v0) * n;

			// With Friction
			vn = glm::dot(n, v) * n;
			vt = v - vn;
			v = v - (frictionCoef * vt);

			break;
		}
	}

	//Collisions with Sphere;
	//Calculamos si la distancia entre la particula y el centro de la esfera - el Radio <=0
	//Formula: d |C-P| = sqrt((pow((Cx - Px),2) , pow((Cy - Py),2) , pow((Cz - Pz),2))
	if (renderSphere)
	{
		d = glm::sqrt(glm::pow(posSphere.x - p.x, 2) + glm::pow(posSphere.y - p.y, 2) + glm::pow(posSphere.z - p.z, 2)) - radiusSphere;
		if (d <= 0)
		{
			glm::vec3 p0p = p - p0;
			float alfa1 = 0, alfa2 = 0, resultAlfa = 0;
			//Usaremos la equación de la recta y la equacion de la esfera.
			//Sacaremos una equiacion Ax^2 + bx + c = 0 para descubrir la alfa
			float a = glm::pow(p0.x - posSphere.x, 2) + glm::pow(p0.y - posSphere.y, 2) + glm::pow(p0.z - posSphere.z, 2) - glm::pow(radiusSphere, 2) + 2 * ((p0.x - posSphere.x) + (p0.y - posSphere.y) + (p0.z - posSphere.z));
			float b = 2 * ((p.x - p0.x) + (p.y - p0.y) + (p.z - p0.z));
			float c = glm::pow(p.x - p0.x, 2) + glm::pow(p.y - p0.y, 2) + glm::pow(p.z - p0.z, 2);

			alfa1 = -b;
			alfa1 += /*(glm::sqrt((*/glm::pow(b, 2) - 4 * a*c;
			alfa1 /= 2 * a;
			alfa2 = -b;
			alfa2; -/*(glm::sqrt(*/glm::pow(b, 2) - 4 * a*c;
			alfa2 /= 2 * a;
			if (glm::abs(alfa1) > glm::abs(alfa2))
			{
				resultAlfa = alfa2;
			}
			else {
				resultAlfa = alfa1;
			}
			//hacemos la recta r= p0 + &p0p; buscaremos el valor "&".
			//Una vez tengamos alfa conoceremos el punto de colision.
			glm::vec3 PuntoColision = { p0.x + p0p.x *resultAlfa, p0.y + p0p.y *resultAlfa, p0.z + p0p.z *resultAlfa };

			glm::vec3 PuntoColision_CentroEsfera = { (PuntoColision.x - posSphere.x) / 2, (PuntoColision.y - posSphere.y) / 2, (PuntoColision.z - posSphere.z) / 2 };


			d = -glm::dot(n, PuntoColision);

			p0 = p;
			v0 = v;

			// With Elasticity
			p = p0 - (1 + elasticCoef) * (glm::dot(n, p0) + d) * n;
			v = v0 - (1 + elasticCoef) * glm::dot(n, v0) * n;

			// With Friction
			vn = glm::dot(n, v) * n;
			vt = v - vn;
			v = v - (frictionCoef * vt);



		}
	}

}

void UpdateParticleMovement(int currParticle, float dt) {
	glm::vec3 p0 = ptrPosParticles[currParticle];
	glm::vec3 v0 = ptrSpeedParticles[currParticle];

	// Euler Method
	glm::vec3 p = p0 + dt * v0;
	glm::vec3 v = v0 + dt * Constants::GRAVITY;

	CheckCollision(p0, p, v0, v);

	ptrPosParticles[currParticle] = p;
	ptrSpeedParticles[currParticle] = v;
}

/*
 * Ring Buffer
 */
void UpdateParticles(float dt) {
	if (endIndexParticlesToDraw - startDrawingFromParticle < 0) {
		for (int i = startDrawingFromParticle; i < MAX_BUFFER_PARTICLES; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}
		for (int i = 0; i < endIndexParticlesToDraw; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}

		LilSpheres::updateParticles(startDrawingFromParticle, MAX_BUFFER_PARTICLES - startDrawingFromParticle, (float*)&ptrPosParticles[startDrawingFromParticle]);
		LilSpheres::updateParticles(0, endIndexParticlesToDraw, (float*)&ptrPosParticles[0]);
	}
	else {
		for (int i = startDrawingFromParticle; i < endIndexParticlesToDraw; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}
		LilSpheres::updateParticles(startDrawingFromParticle, numParticlesEnabled, (float*)&ptrPosParticles[startDrawingFromParticle]);
	}
}

void PhysicsUpdate(float dt) {
	if (playingSimulation) {
		if (useGravity) {
			acceleration = Constants::GRAVITY;
		}
		Sphere::updateSphere(posSphere, radiusSphere);
		UpdateEmitter(dt);
		UpdateParticles(dt);
	}
}

void PhysicsCleanup() {
	delete[] ptrPosParticles;
	delete[] ptrSpeedParticles;
}