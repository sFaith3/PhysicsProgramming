#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <limits.h>
#include <glm/vec3.hpp>
#include <SDL2\SDL.h>

/// CONSTANTS
const int MAX_PARTICLES(5000);

// EMITTER
const int MINIMUM_RATE_PARTICLE_EMITTER(100);
const int MAXIMUM_RATE_PARTICLE_EMITTER(500);
const float MINIMUM_TIME_PARTICLE_LIFE(1.0f);
const float MAXIMUM_TIME_PARTICLE_LIFE(100.0f);
const uint32_t TIME_NEW_PARTICLES(uint32_t(1000.0f));

const glm::vec3 GRAVITY = { 0.0f, -9.81f, 0.0f };

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

bool show_test_window = true;//

bool playingSimulation = false;

uint32_t lastTime;

enum class EnumTypeMovement
{
	FOUNTAIN, CASCADE
};

// EMITTER
int rateParticleEmitter; //It must be >= 100
float particleLifeTimeEmitter;  //It must be >= 1.0
int currTypeEmitter;
glm::vec3 posEmitter;
glm::vec3 dirEmitter;
float angleEmitter;

// ELASTICITY & FRICTION
float elasticCoef;
float frictionCoef;

// SPHERE COLLIDER
bool useSphereCollider;
glm::vec3 posSphere;
float radiusSphere;

// CAPSULE COLLIDER
bool useCapsuleCollider;
glm::vec3 posACapsule;
glm::vec3 posBCapsule;
float radiusCapsule;

// FORCES
bool useGravity;
glm::vec3 acceleration;

// PARTICLES
float particlesLifeTime[MAX_PARTICLES];
glm::vec3 *ptrPosParticles;	  //Posiciones de las partículas
glm::vec3 *ptrSpeedParticles; //Velocidades de las partículas
int numParticlesEnabled;
extern int numParticlesToDraw;

void PhysicsCleanup();



void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

	ImGui::Checkbox("Play Simulation", &playingSimulation);
	if (ImGui::Button("Reset Simulation")) {
		PhysicsCleanup();
	}

	/// EMITER
	if (ImGui::TreeNode("Emitter")) {
		ImGui::DragInt("Rate", &rateParticleEmitter, 1.0f, MINIMUM_RATE_PARTICLE_EMITTER, MAXIMUM_RATE_PARTICLE_EMITTER);
		ImGui::DragFloat("Particle life", &particleLifeTimeEmitter, 1.0f, MINIMUM_TIME_PARTICLE_LIFE, MAXIMUM_TIME_PARTICLE_LIFE);

		ImGui::RadioButton("Fountain", &currTypeEmitter, 0); ImGui::SameLine();
		ImGui::RadioButton("Cascade", &currTypeEmitter, 1);

		/// FOUNTAIN & CASCADE
		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN:
			ImGui::DragFloat3("Fountain Position", &posEmitter.x);
			ImGui::DragFloat3("Fountain Direction", &dirEmitter.x, 1.0f, 0.0f, 1.0f);
			ImGui::DragFloat("Fountain Angle", &angleEmitter);
			break;

		case EnumTypeMovement::CASCADE:
			ImGui::DragFloat3("Cascade Position", &posEmitter.x);
			ImGui::DragFloat3("Cascade Direction", &dirEmitter.x, 1.0f, 0.0f, 1.0f);
			ImGui::DragFloat("Cascade Angle", &angleEmitter);
			break;

		default:
			break;
		}

		ImGui::TreePop();
	}

	/// ELASTICITY & FRICTION
	if (ImGui::TreeNode("Elasticity & Friction")) {
		ImGui::DragFloat("Elastic Coefficient", &elasticCoef);
		ImGui::DragFloat("Friction Coefficient", &frictionCoef);

		ImGui::TreePop();
	}

	/// COLLIDERS
	if (ImGui::TreeNode("Colliders")) {
		// SPHERE
		ImGui::Checkbox("Use Sphere Collider", &useSphereCollider);
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
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void InitEmitter() {
	rateParticleEmitter = MINIMUM_RATE_PARTICLE_EMITTER;
	particleLifeTimeEmitter = MINIMUM_TIME_PARTICLE_LIFE;
	currTypeEmitter = (int)EnumTypeMovement::FOUNTAIN;
	posEmitter = { 0.0f, 0.0f, 0.0f };
	dirEmitter = { 0.0f, 0.0f, 0.0f };
	angleEmitter = 0.0f;
}

void InitSphereCollider() {
	useSphereCollider = false;
	posSphere = { 0.0f, 0.0f, 0.0f };
	radiusSphere = 1.0f;
}

void InitCapsuleCollider() {
	useCapsuleCollider = false;
	posACapsule = { 0.0f, 0.0f, 0.0f };
	posBCapsule = { 0.0f, 0.0f, 0.0f };
	radiusCapsule = 1.0f;
}

void InitColliders() {
	InitSphereCollider();
	InitCapsuleCollider();
}

void PhysicsInit() {
	lastTime = uint32_t(0.0f);

	InitEmitter();
	InitColliders();

	// FORCES
	useGravity = false;
	acceleration = GRAVITY;

	// DATA FOR PARTICLES
	for (int i = 0; i < MAX_PARTICLES; i++) {
		particlesLifeTime[i] = 0.0f;
	}
	ptrPosParticles = new glm::vec3[MAX_PARTICLES]{ glm::vec3(0.0f, 0.0f, 0.0f) };
	ptrSpeedParticles = new glm::vec3[MAX_PARTICLES]{ acceleration };
	numParticlesEnabled = 0;

	// ELASTICITY & FRICTION
	elasticCoef = 1.0f;
	frictionCoef = 0.0f;
}

void AddNewParticle() {
	numParticlesEnabled++;
	numParticlesToDraw++;
}

void NewParticle(int currTypeEmitter) {
	if (numParticlesEnabled < MAX_PARTICLES) {
		particlesLifeTime[numParticlesEnabled] = particleLifeTimeEmitter;
		ptrPosParticles[numParticlesEnabled] = posEmitter;

		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN:
			ptrSpeedParticles[numParticlesEnabled] = acceleration;
			break;

		case EnumTypeMovement::CASCADE:
			//TODO ptrSpeedParticles[numParticlesEnabled] = { , ,  };
			break;

		default:
			break;
		}
		AddNewParticle();
	}
}

void DeleteLastParticle(int currParticle) {
	particlesLifeTime[currParticle] = 0.0f;
	numParticlesEnabled--;
	numParticlesToDraw--;
}

bool IsParticleAlive(int currParticle, float dt) {
	if (particlesLifeTime[currParticle] <= 0.0f) {
		DeleteLastParticle(currParticle);
		return false;
	}
	else {
		particlesLifeTime[currParticle] -= dt;
		return true;
	}
}

void UpdateParticleMovement(int currParticle, float dt) {
	// Euler Method
	ptrPosParticles[currParticle] = ptrPosParticles[currParticle] + dt * ptrSpeedParticles[currParticle];
	ptrSpeedParticles[currParticle] = ptrSpeedParticles[currParticle] + dt * GRAVITY;
}

void UpdateCollisions(int currParticle) {
	// WALLS


	// GROUND PLANES


	// SPHERE


	// CAPSULE

}

void PhysicsUpdate(float dt) {
	if (playingSimulation) {
		// Spawn each specified time
		uint32_t currTime = SDL_GetTicks();
		if ((currTime - lastTime >= TIME_NEW_PARTICLES || lastTime == 0.0f) && numParticlesEnabled < MAX_PARTICLES) {
			NewParticle(currTypeEmitter);
			lastTime = SDL_GetTicks();
		}

		LilSpheres::updateParticles(0, numParticlesEnabled, (float*)ptrPosParticles);
		for (int i = numParticlesEnabled - 1; i >= 0; i--) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}
	}
}

void PhysicsCleanup() {
	playingSimulation = false;

	delete[] ptrPosParticles;
	delete[] ptrSpeedParticles;

	numParticlesToDraw = 0;

	PhysicsInit();
}