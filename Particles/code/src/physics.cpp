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
	extern glm::vec3 posSphere;
	extern float radiusSphere;
	extern void updateSphere(glm::vec3 pos, float radius);
}

namespace Capsule {
	extern glm::vec3 posACapsule;
	extern glm::vec3 posBCapsule;
	extern float radiusCapsule;
	extern void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius);
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

extern const int MAX_BUFFER_PARTICLES(SHRT_MAX / 3);

//bool show_test_window = false;

bool playingSimulation;

// EMITTER
int emitterRateParticle;		//It must be >= 100
float emitterParticleLifeTime;  //It must be >= 1.0
int emitterCurrType;
glm::vec3 emitterPos;
glm::vec3 emitterDir;
glm::vec3 emitterInitialSpeed;

float fountAngle;
float fountRadius;
const float deltaCircumAngle = Constants::MAX_ANGLE_CIRCUM / Constants::NUM_PARTICLES_FOUNTAIN + 1;
float currCircumAngle;
int fountCurrNumParticles;

glm::vec3 emitterFinalPos;
int cascadeCurrNumParticles;

// ELASTICITY & FRICTION
float coefElastic;
float coefFriction;

// SPHERE COLLIDER
extern bool renderSphere;

// CAPSULE COLLIDER
extern bool renderCapsule;

// FORCES
bool useGravity;
glm::vec3 acceleration;

// PARTICLES
int numParticlesEnabled;
extern int startDrawingFromParticle;
extern int endIndexParticlesToDraw;

float particlesLifeTime[MAX_BUFFER_PARTICLES];
glm::vec3 *ptrParticlesPos = new glm::vec3[MAX_BUFFER_PARTICLES]{ glm::vec3(0.0f, 0.0f, 0.0f) }; // Posiciones de las partículas
glm::vec3 *ptrParticlesSpeed = new glm::vec3[MAX_BUFFER_PARTICLES]{ acceleration };				 // Velocidades de las partículas

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
		PhysicsInit();
	}

	/// EMITER
	if (ImGui::TreeNode("Emitter")) {
		ImGui::DragInt("Rate", &emitterRateParticle, 1.0f, Constants::MIN_RATE_PARTICLES_EMITTER, Constants::MAX_RATE_PARTICLES_EMITTER);
		ImGui::DragFloat("Particle life", &emitterParticleLifeTime, 1.0f, Constants::MIN_TIME_PARTICLE_LIFE, Constants::MAX_TIME_PARTICLE_LIFE);

		/// FOUNTAIN & CASCADE
		ImGui::RadioButton("Fountain", &emitterCurrType, 0); ImGui::SameLine();
		ImGui::RadioButton("Cascade", &emitterCurrType, 1);

		switch (static_cast<EnumTypeMovement>(emitterCurrType)) {
		case EnumTypeMovement::FOUNTAIN:
			ImGui::DragFloat3("Fountain Position", &emitterPos.x);
			ImGui::DragFloat3("Fountain Direction", &emitterDir.x, 1.0f, -1.0f, 1.0f);
			ImGui::DragFloat("Fountain Angle", &fountAngle, 1.0f, Constants::MIN_ANGLE_FOUNTAIN, Constants::MAX_ANGLE_FOUNTAIN);
			break;

		case EnumTypeMovement::CASCADE:
			ImGui::DragFloat3("Cascade Position A", &emitterPos.x);
			ImGui::DragFloat3("Cascade Position B", &emitterFinalPos.x);
			ImGui::DragFloat3("Cascade Velocity", &emitterInitialSpeed.x);
			break;

		default:
			break;
		}

		ImGui::TreePop();
	}

	/// ELASTICITY & FRICTION
	if (ImGui::TreeNode("Elasticity & Friction")) {
		ImGui::DragFloat("Elastic Coefficient", &coefElastic, 1.0f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction Coefficient", &coefFriction, 1.0f, 0.0f, 1.0f);

		ImGui::TreePop();
	}

	/// COLLIDERS
	if (ImGui::TreeNode("Colliders")) {
		// SPHERE
		ImGui::Checkbox("Use Sphere Collider", &renderSphere);
		ImGui::DragFloat3("Sphere Position", &Sphere::posSphere.x);
		ImGui::DragFloat("Sphere Radius", &Sphere::radiusSphere);

		// CAPSULE
		ImGui::Checkbox("Use Capsule Collider", &renderCapsule);
		ImGui::DragFloat3("Capsule Position A", &Capsule::posACapsule.x);
		ImGui::DragFloat3("Capsule Position B", &Capsule::posBCapsule.x);
		ImGui::DragFloat("Capsule Radius", &Capsule::radiusCapsule, 1.0f, Constants::MIN_RADIUS_CAPSULE, Constants::MAX_RADIUS_CAPSULE);

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
	emitterRateParticle = Constants::MIN_RATE_PARTICLES_EMITTER;
	emitterParticleLifeTime = Constants::MIN_TIME_PARTICLE_LIFE;
	emitterCurrType = (int)EnumTypeMovement::FOUNTAIN;
	emitterPos = { 0.0f, 1.5f, 0.0f };
	emitterDir = Constants::DEFAULT_DIR_PARTICLES;
	emitterInitialSpeed = Constants::DEFAULT_SPEED_PARTICLES;

	fountAngle = Constants::MIN_ANGLE_FOUNTAIN;
	fountRadius = Constants::MIN_RADIUS_FOUNTAIN + (rand() % static_cast<int>(Constants::MAX_RADIUS_FOUNTAIN - Constants::MIN_RADIUS_FOUNTAIN + 1));
	currCircumAngle = 0.0f;
	fountCurrNumParticles = 0;

	emitterFinalPos = { 1.0f, 2.5f, 1.0f };
	cascadeCurrNumParticles = 0;
}

void InitColliders() {
	// Sphere
	renderSphere = false;
	Sphere::posSphere = { 0.0f, 0.0f, 0.0f };
	Sphere::radiusSphere = 1.0f;

	// Capsule
	renderCapsule = false;
	Capsule::posACapsule = Constants::DEFAULT_POS_A_CAPSULE;
	Capsule::posBCapsule = Constants::DEFAULT_POS_B_CAPSULE;
	Capsule::radiusCapsule = Constants::DEFAULT_RADIUS_CAPSULE;
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
	coefElastic = 1.0f;
	coefFriction = 0.0f;

	InitColliders();

	// FORCES
	useGravity = true;
	acceleration = Constants::GRAVITY;

	// DATA FOR PARTICLES
	endIndexParticlesToDraw = 0;
	startDrawingFromParticle = 0;
	numParticlesEnabled = 0;

	InitPlanes();
}

/*
 * Do it after to initialize his variables
 */
void EnableParticle() {
	particlesLifeTime[endIndexParticlesToDraw] = emitterParticleLifeTime;
	endIndexParticlesToDraw = (endIndexParticlesToDraw + 1) % MAX_BUFFER_PARTICLES;
	numParticlesEnabled++;
}

void NewFountainParticles() {
	glm::vec3 v0;
	if (numParticlesEnabled < MAX_BUFFER_PARTICLES) {
		v0.x = cos(currCircumAngle) * fountRadius;
		v0.y = sin(fountAngle) * Constants::DEFAULT_SPEED_PARTICLES.y;
		v0.z = sin(currCircumAngle) * fountRadius;
		currCircumAngle += deltaCircumAngle;

		fountCurrNumParticles = (fountCurrNumParticles + 1) % Constants::NUM_PARTICLES_FOUNTAIN;
		if (fountCurrNumParticles == 0) {
			currCircumAngle = 0.0f;
		}

		ptrParticlesPos[endIndexParticlesToDraw] = emitterPos;
		ptrParticlesSpeed[endIndexParticlesToDraw] = v0 + emitterDir;

		EnableParticle();
	}
}

void NewCascadeParticles() {
	glm::vec3 x0;
	if (numParticlesEnabled < MAX_BUFFER_PARTICLES) {
		x0 = (emitterFinalPos - emitterPos) / glm::vec3(Constants::NUM_PARTICLES_CASCADE);
		x0 *= (cascadeCurrNumParticles + 1);
		x0 += emitterPos;
		cascadeCurrNumParticles = (cascadeCurrNumParticles + 1) % Constants::NUM_PARTICLES_CASCADE;

		ptrParticlesPos[endIndexParticlesToDraw] = x0;
		ptrParticlesSpeed[endIndexParticlesToDraw] = emitterInitialSpeed;

		EnableParticle();
	}
}

void NewParticles(int emitterCurrType) {
	if (numParticlesEnabled < MAX_BUFFER_PARTICLES - 1) {
		switch (static_cast<EnumTypeMovement>(emitterCurrType)) {
		case EnumTypeMovement::FOUNTAIN:
			NewFountainParticles();
			break;

		case EnumTypeMovement::CASCADE:
			NewCascadeParticles();
			break;

		default:
			break;
		}
	}
}

/*
 * Spawn at specified time
 */
void UpdateEmitter(float dt) {
	int numParticlesToEnable = static_cast<int>(dt * emitterRateParticle);
	for (int i = 0; i < numParticlesToEnable && numParticlesEnabled < MAX_BUFFER_PARTICLES; i++) {
		NewParticles(emitterCurrType);
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

void CollisionParticlePlane(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v, glm::vec3 n, float d) {
	p0 = p;
	v0 = v;

	// With Elasticity
	p = p0 - (1 + coefElastic) * (glm::dot(n, p0) + d) * n;
	v = v0 - (1 + coefElastic) * glm::dot(n, v0) * n;

	// With Friction
	glm::vec3 vn = glm::dot(n, v) * n;
	glm::vec3 vt = v - vn;
	v = v - (coefFriction * vt);
}

void CollisionParticleWithCapsule(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	// P'  = B + ((P - B) * (B - A)) * (B - A)
	// P'' = B + Clamp((P - B) * (B - A), 0, 1) * (B - A)
	// d(Cap, Particle) = |P - P'|
	glm::vec3 B = Capsule::posBCapsule;
	glm::vec3 BP = p - B;
	glm::vec3 AB = B - Capsule::posACapsule;

	glm::vec3 P_ = B + (BP * AB) * AB;
	glm::vec3 P__ = B + glm::clamp((BP * AB), 0.0f, 1.0f) * AB;

	float d = glm::sqrt(glm::pow(p.x - P_.x , 2) + glm::pow(p.y - P_.y, 2) + glm::pow(p.z - P_.z, 2));

	if (d < Capsule::radiusCapsule) {
		// Get collision point
		glm::vec3 PC;

		/// To calculate tangent plane at collision point in a don't flat surface
		// Equation of the line r: A + AB(alfa) = P'
		glm::vec3 A = Capsule::posACapsule;
		glm::vec3 AB = Capsule::posBCapsule - A;

		// To get "alfa" to do vector PP' and PP' x u = PP' x AB
		glm::vec3 PC_P = A - PC;
		glm::vec3 PC_P_alfa = AB;

		float x = glm::dot(PC_P, AB);
		float y = glm::dot(PC_P_alfa, AB);

		// x + y(alfa) = 0 -> alfa = -x / y
		float alfa = -x / y;

		// Replace "landa" at equation of the line to get P', where P' = _P
		glm::vec3 _P = A + (AB * alfa);

		glm::vec3 n = glm::normalize(PC - _P);
		float D = -glm::dot(n, PC);

		CollisionParticlePlane(p0, p, v0, v, n, D);
	}
}

void CollisionParticleWithSphere(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	// Calculamos si la distancia entre la particula y el centro de la esfera - el Radio <=0
	// Fórmula: d |C-P| = sqrt((pow((Cx - Px),2), pow((Cy - Py),2), pow((Cz - Pz),2))
	float d = glm::sqrt(glm::pow(Sphere::posSphere.x - p.x, 2) + glm::pow(Sphere::posSphere.y - p.y, 2) + glm::pow(Sphere::posSphere.z - p.z, 2));
	if (d < Sphere::radiusSphere) {
		glm::vec3 p0p = p - p0;
		float alfa1 = 0, alfa2 = 0, resultAlfa = 0;

		// Usaremos la equación de la recta y la equacion de la esfera.
		// Sacaremos una equiacion Ax^2 + bx + c = 0 para descubrir la alfa
		float a = glm::pow(p0.x - Sphere::posSphere.x, 2) + glm::pow(p0.y - Sphere::posSphere.y, 2) + glm::pow(p0.z - Sphere::posSphere.z, 2) - glm::pow(Sphere::radiusSphere, 2) +
			2 * ((p0.x - Sphere::posSphere.x) + (p0.y - Sphere::posSphere.y) + (p0.z - Sphere::posSphere.z));
		float b = 2 * ((p.x - p0.x) + (p.y - p0.y) + (p.z - p0.z));
		float c = glm::pow(glm::pow(p.x - p0.x, 2) + glm::pow(p.y - p0.y, 2) + glm::pow(p.z - p0.z, 2),2);

		alfa1 = -b;
		alfa1 += glm::sqrt(glm::pow(b, 2) - 4 * a*c);
		alfa1 /= 2 * a;
		alfa2 = -b;
		alfa2 -= glm::sqrt(glm::pow(b, 2) - 4 * a*c);
		alfa2 /= 2 * a;

		(glm::abs(alfa1) > glm::abs(alfa2)) ? resultAlfa = alfa2 : resultAlfa = alfa1;

		// recta: r = p0 + &p0p; buscaremos el valor "&".
		// Una vez tengamos alfa conoceremos el punto de colision.
		glm::vec3 PuntoColision = { p0.x + p0p.x *resultAlfa, p0.y + p0p.y *resultAlfa, p0.z + p0p.z *resultAlfa };
		glm::vec3 n = glm::normalize(PuntoColision - Sphere::posSphere);
		float D = -glm::dot(n, PuntoColision);

		CollisionParticlePlane(p0, p, v0, v, n, D);
	}
}

void CollisionParticleWithWallsAndGround(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	Plane currPlane;
	glm::vec3 n;
	float d;

	for (int i = 0; i < Constants::FACES_CUBE; i++) {
		currPlane = planes[i];
		n = currPlane.normal;

		d = (glm::dot(n, p) + currPlane.D) * (glm::dot(n, p0) + currPlane.D);
		if (d < 0.0f) {
			CollisionParticlePlane(p0, p, v0, v, n, currPlane.D);
			break;
		}
	}
}

void CheckCollisions(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	CollisionParticleWithWallsAndGround(p0, p, v0, v);

	if (renderSphere)
		CollisionParticleWithSphere(p0, p, v0, v);
	if (renderCapsule)
		CollisionParticleWithCapsule(p0, p, v0, v);
}

void UpdateParticleMovement(int currParticle, float dt) {
	glm::vec3 p0 = ptrParticlesPos[currParticle];
	glm::vec3 v0 = ptrParticlesSpeed[currParticle];

	// Euler Method
	glm::vec3 p = p0 + dt * v0;
	glm::vec3 v = v0 + dt * Constants::GRAVITY;

	CheckCollisions(p0, p, v0, v);

	ptrParticlesPos[currParticle] = p;
	ptrParticlesSpeed[currParticle] = v;
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

		LilSpheres::updateParticles(startDrawingFromParticle, MAX_BUFFER_PARTICLES - startDrawingFromParticle, (float*)&ptrParticlesPos[startDrawingFromParticle]);
		LilSpheres::updateParticles(0, endIndexParticlesToDraw, (float*)&ptrParticlesPos[0]);
	}
	else {
		for (int i = startDrawingFromParticle; i < endIndexParticlesToDraw; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}
		LilSpheres::updateParticles(startDrawingFromParticle, numParticlesEnabled, (float*)&ptrParticlesPos[startDrawingFromParticle]);
	}
}

void PhysicsUpdate(float dt) {
	if (playingSimulation) {
		if (useGravity) {
			acceleration = Constants::GRAVITY;
		}

		UpdateEmitter(dt);
		UpdateParticles(dt);

		Sphere::updateSphere(Sphere::posSphere, Sphere::radiusSphere);
		Capsule::updateCapsule(Capsule::posACapsule, Capsule::posBCapsule, Capsule::radiusCapsule);
	}
}

void PhysicsCleanup() {
	delete[] ptrParticlesPos;
	delete[] ptrParticlesSpeed;
}