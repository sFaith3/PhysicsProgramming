#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <limits.h>
#include <glm/vec3.hpp>

// CONSTANTS
const int MAX_PARTICLES(5000);
const int MINIMUM_RATE_PARTICLE_EMITTER(100);
const float MINIMUM_TIME_PARTICLE_LIFE(1.0f);
const glm::vec3 GRAVITY = { 0.0f, -9.81f, 0.0f };

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

bool show_test_window = true;//

bool playingSimulation = false;

enum class EnumTypeMovement
{
	FOUNTAIN, CASCADE
};

// EMITTER
int rateParticleEmitter; //It must be >= 100
float timeParticleLife;  //It must be >= 1.0
EnumTypeMovement currTypeMovement;
//bool isFountain, isCascade;
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
glm::vec3 *ptrPosParticles;	  //Posiciones de las partículas
glm::vec3 *ptrSpeedParticles; //Velocidades de las partículas
int numParticlesEnabled;
extern int numParticlesToDraw;



void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

		ImGui::Checkbox("Play Simulation", &playingSimulation);
		ImGui::Button("Reset Simulation");

		/// EMITER
		ImGui::Text("Emitter"); //TODO flechita desplegable

		ImGui::DragInt("Rate", &rateParticleEmitter, 1.0f, MINIMUM_RATE_PARTICLE_EMITTER);
		ImGui::DragFloat("Particle life", &timeParticleLife, 1.0f, MINIMUM_TIME_PARTICLE_LIFE);

		
		/*ImGui::RadioButton("Fountain", isFountain);
		ImGui::RadioButton("Cascade", isCascade);*/

		/// FOUNTAIN & CASCADE
		switch (currTypeMovement) {
		case EnumTypeMovement::FOUNTAIN:
			ImGui::DragFloat3("Fountain Position", &posEmitter.x);
			ImGui::DragFloat3("Fountain Direction", &dirEmitter.x);
			ImGui::DragFloat("Fountain Angle", &angleEmitter);
			break;

		case EnumTypeMovement::CASCADE:
			ImGui::DragFloat3("Cascade Position", &posEmitter.x);
			ImGui::DragFloat3("Cascade Direction", &dirEmitter.x);
			ImGui::DragFloat("Cascade Angle", &angleEmitter);
			break;

		default:
			break;
		}

		/// ELASTICITY & FRICTION
		ImGui::Text("Elasticity & Friction"); //TODO flechita desplegable

		ImGui::DragFloat("Elastic Coefficient", &elasticCoef);
		ImGui::DragFloat("Friction Coefficient", &frictionCoef);

		/// COLLIDERS
		ImGui::Text("Colliders");

		// SPHERE
		ImGui::Checkbox("Use Sphere Collider", &useSphereCollider);
		ImGui::DragFloat3("Sphere Position", &posSphere.x);
		ImGui::DragFloat("Sphere Radius", &radiusSphere);

		// CAPSULE
		ImGui::Checkbox("Use Capsule Collider", &useCapsuleCollider);
		ImGui::DragFloat3("Capsule Position A", &posACapsule.x);
		ImGui::DragFloat3("Capsule Position B", &posBCapsule.x);
		ImGui::DragFloat("Capsule Radius", &radiusCapsule);

		/// FORCES
		ImGui::Text("Forces");
		ImGui::Checkbox("Use Gravity", &useGravity);
		ImGui::DragFloat3("Acceleration", &acceleration.x);
	}
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

//TODO when reset button -> playingSimulation = false and PhysicsCleanUp()

void InitEmitter() {
	rateParticleEmitter = MINIMUM_RATE_PARTICLE_EMITTER;
	timeParticleLife = MINIMUM_TIME_PARTICLE_LIFE;
	currTypeMovement = EnumTypeMovement::FOUNTAIN;
	/*isFountain = true;
	isCascade = false;*/
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
	InitEmitter();
	InitColliders();
	
	// DATA FOR PARTICLES
	ptrPosParticles = new glm::vec3[MAX_PARTICLES];
	ptrSpeedParticles = new glm::vec3[MAX_PARTICLES];
	numParticlesEnabled = 0;

	// ELASTICITY & FRICTION
	elasticCoef = 1.0f;
	frictionCoef = 0.0f;

	// FORCES
	useGravity = false;
	acceleration = GRAVITY;
}

void NewParticles(int numParticles) {
	if (numParticlesEnabled < MAX_PARTICLES) {
		for (int i = numParticlesEnabled; i < numParticles; i++) {
			ptrPosParticles[i] = posEmitter;
			ptrSpeedParticles[i] = { rand() % 1 - 1, rand() % 1 - 1, rand() % 1 - 1 };//

			numParticlesEnabled++;
			numParticlesToDraw++;

			if (numParticlesEnabled >= MAX_PARTICLES) break;
		}
	}
}

void UpdateMovement(int currParticle, float dt) {
	switch (currTypeMovement) {
	case EnumTypeMovement::FOUNTAIN:
		//TODO
		break;

	case EnumTypeMovement::CASCADE:
		//TODO
		break;

	default:
		break;
	}

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
		//TODO do this each second
		NewParticles(rateParticleEmitter);

		//LilSpheres::updateParticles(1, numParticlesEnabled, (float*)ptrPosParticles);
		for (int i = numParticlesEnabled - 1; i >= 0; i--) {
			LilSpheres::updateParticles(i, 1, (float*)ptrPosParticles);
			UpdateMovement(i, dt);
			
		}
	}
}

void PhysicsCleanup() {
	delete[] ptrPosParticles;
	delete[] ptrSpeedParticles;

	numParticlesEnabled = 0;
	numParticlesToDraw = 0;
}