#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <limits.h>
#include <glm/vec3.hpp>
#include <SDL2\SDL.h>

/// CONSTANTS
// EMITTER
const int MINIMUM_RATE_PARTICLES_EMITTER(100);
const int MAXIMUM_RATE_PARTICLES_EMITTER(1000);
const float MINIMUM_TIME_PARTICLE_LIFE(1.0f);
const float MAXIMUM_TIME_PARTICLE_LIFE(100.0f);

// FOUNTAIN
const float MINIMUM_ANGLE_FOUNTAIN(75.0f);
const float MAXIMUM_ANGLE_FOUNTAIN(115.0f);

const float INITIAL_SPEED_PARTICLES(7.0f);

const glm::vec3 GRAVITY(0.0f, -9.81f, 0.0f);

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

//bool show_test_window = false;

bool playingSimulation;

uint32_t lastTime;

enum class EnumTypeMovement
{
	FOUNTAIN, CASCADE
};

// EMITTER
int rateParticleEmitter;		//It must be >= 100
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
int numParticlesEnabled;
extern int startDrawingFromParticle;
extern int endIndexParticlesToDraw;

float particlesLifeTime[MAXIMUM_RATE_PARTICLES_EMITTER];
glm::vec3 *ptrPosParticles;	  //Posiciones de las partículas
glm::vec3 *ptrSpeedParticles; //Velocidades de las partículas

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
		ImGui::DragInt("Rate", &rateParticleEmitter, 1.0f, MINIMUM_RATE_PARTICLES_EMITTER, MAXIMUM_RATE_PARTICLES_EMITTER);
		ImGui::DragFloat("Particle life", &particleLifeTimeEmitter, 1.0f, MINIMUM_TIME_PARTICLE_LIFE, MAXIMUM_TIME_PARTICLE_LIFE);

		ImGui::RadioButton("Fountain", &currTypeEmitter, 0); ImGui::SameLine();
		ImGui::RadioButton("Cascade", &currTypeEmitter, 1);

		/// FOUNTAIN & CASCADE
		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN:
			ImGui::DragFloat3("Fountain Position", &posEmitter.x);
			ImGui::DragFloat3("Fountain Direction", &dirEmitter.x, 1.0f, -1.0f, 1.0f);
			ImGui::DragFloat("Fountain Angle", &angleEmitter, 1.0f, MINIMUM_ANGLE_FOUNTAIN, MAXIMUM_ANGLE_FOUNTAIN);
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
	/*if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}*/
}

void InitEmitter() {
	rateParticleEmitter = MINIMUM_RATE_PARTICLES_EMITTER;
	particleLifeTimeEmitter = MINIMUM_TIME_PARTICLE_LIFE;
	currTypeEmitter = (int)EnumTypeMovement::FOUNTAIN;
	posEmitter = { 0.0f, 0.0f, 0.0f };
	dirEmitter = { 0.0f, 0.0f, 0.0f };
	angleEmitter = MINIMUM_ANGLE_FOUNTAIN;
}

void InitColliders() {
	// Sphere
	useSphereCollider = false;
	posSphere = { 0.0f, 0.0f, 0.0f };
	radiusSphere = 1.0f;

	// Capsule
	useCapsuleCollider = false;
	posACapsule = { 0.0f, 0.0f, 0.0f };
	posBCapsule = { 0.0f, 0.0f, 0.0f };
	radiusCapsule = 1.0f;
}

void PhysicsInit() {
	playingSimulation = false;

	InitEmitter();

	// ELASTICITY & FRICTION
	elasticCoef = 1.0f;
	frictionCoef = 0.0f;

	InitColliders();

	// FORCES
	useGravity = false;
	acceleration = GRAVITY;

	// DATA FOR PARTICLES
	endIndexParticlesToDraw = 0;
	startDrawingFromParticle = 0;
	numParticlesEnabled = 0;

	for (int i = 0; i < MAXIMUM_RATE_PARTICLES_EMITTER; i++) {
		particlesLifeTime[i] = 0.0f;
	}
	ptrPosParticles = new glm::vec3[MAXIMUM_RATE_PARTICLES_EMITTER]{ glm::vec3(0.0f, 0.0f, 0.0f) };
	ptrSpeedParticles = new glm::vec3[MAXIMUM_RATE_PARTICLES_EMITTER]{ acceleration };
}

void NewParticle(int currTypeEmitter) {
	if (numParticlesEnabled < MAXIMUM_RATE_PARTICLES_EMITTER) {
		particlesLifeTime[endIndexParticlesToDraw] = particleLifeTimeEmitter;
		ptrPosParticles[endIndexParticlesToDraw] = posEmitter;

		switch (static_cast<EnumTypeMovement>(currTypeEmitter)) {
		case EnumTypeMovement::FOUNTAIN: //TODO
			ptrSpeedParticles[endIndexParticlesToDraw] = INITIAL_SPEED_PARTICLES * dirEmitter;
			
			break;

		case EnumTypeMovement::CASCADE: //TODO
			//ptrSpeedParticles[endIndexParticlesToDraw] = { , ,  };
			//
			break;

		default:
			break;
		}
		
		endIndexParticlesToDraw = (endIndexParticlesToDraw + 1) % MAXIMUM_RATE_PARTICLES_EMITTER;
		numParticlesEnabled++;
	}
}

void DeleteLastParticle() {
	startDrawingFromParticle = (startDrawingFromParticle + 1) % MAXIMUM_RATE_PARTICLES_EMITTER;
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

/*
	Ring Buffer
*/
void UpdateParticles(float dt) {
	if (endIndexParticlesToDraw - startDrawingFromParticle < 0) {
		for (int i = startDrawingFromParticle; i < MAXIMUM_RATE_PARTICLES_EMITTER; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}
		for (int i = 0; i < endIndexParticlesToDraw; i++) {
			if (IsParticleAlive(i, dt)) {
				UpdateParticleMovement(i, dt);
			}
		}

		LilSpheres::updateParticles(startDrawingFromParticle, MAXIMUM_RATE_PARTICLES_EMITTER - startDrawingFromParticle, (float*)&ptrPosParticles[startDrawingFromParticle]);
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
			acceleration = GRAVITY;
		}

		// Spawn at specified time
		int numParticlesToEnable = dt * rateParticleEmitter;
		for (int i = 0; i < numParticlesToEnable; i++) {
			NewParticle(currTypeEmitter);
		}

		// Update particles
		UpdateParticles(dt);
	}
}

void PhysicsCleanup() {
	delete[] ptrPosParticles;
	delete[] ptrSpeedParticles;
}