#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtx\intersect.hpp>
#include <vector>
#include <array>
#include <string>
#include <iostream>


///////////////

/////// constants
const glm::vec3 DEF_GRAV_ACCEL	  = glm::vec3(0.f, -9.81f, 0.f);
const float		DEF_ELASTIC_COEF  = .5f;
const float		DEF_FRICTION_COEF = .1f;
const float     DEF_DENSI_FLUID   = 1.f;

const float PI = 3.14159265f;

const float DEF_RESET_TIME = 15.f;

// Cloth Mesh
const float DEF_PARTICLE_LINK = .5f;
const float MAX_PARTICLE_LINK = 5.f;
const float POS_X_CLOTH		  = 4.f;
const float POS_Z_CLOTH	      = -4.5f;

// Sphere
const float DEF_MASS_SPHERE   = 3.f;
const float DEF_RADIUS_SPHERE = 1.f;
const float MIN_POS_SPHERE    = 2.f;
const float MAX_POS_SPHERE    = 4.f;
const float MIN_RADIUS_SPHERE = .2f;
const float MAX_RADIUS_SPHERE = 3.f;

// Waves
const int DEF_NUM_WAVES = 3;
const int MAX_NUM_WAVES = 6;

const float MAX_AMPLI_WAVE = 10.f;
const float MAX_FREC_WAVE  = 10.f;

const glm::vec3 DEF_WAVE_DIRS[MAX_NUM_WAVES] = { glm::vec3(.8f, 0.f, 0.f), glm::vec3(0.f, 0.f, .8f), glm::vec3(.8f, 0.f, 0.f), glm::vec3(.8f, 0.f, 0.f), glm::vec3(.8f, 0.f, 0.f), glm::vec3(.8f, 0.f, 0.f) };
const float		DEF_WAVE_AMPLIS[MAX_NUM_WAVES] = { .5f, .8f, .4f, .5f, .5f, .5f };
const float		DEF_WAVE_Ws[MAX_NUM_WAVES]	   = { 1.2f, 1.8f, 1.6f, 1.5f, 1.5f, 1.5f };

/////// fw declarations
namespace ClothMesh {
	extern const int numCols;
	extern const int numRows;
	extern const int numVerts;

	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
}

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
}

void PhysicsInit();
void PhysicsReset();

/////// global variables
bool isPlaying;

glm::vec3 vGravAccel;

float elasticCoef, frictionCoef;

float pFluid; // densidad del fluido
float Cd;

float resetTime;
double currentTime;

float particleLink;

glm::vec3 *ptrWaveParticlesPos0 = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrWaveParticlesPos  = new glm::vec3[ClothMesh::numVerts];

int numWaves;

bool show_test_window = false;

/////// classes & structs
class Wave {
public:
	glm::vec3 dir;
	float amplitude;
	float w; // frecuéncia de la ola

	Wave() {
	};

	~Wave() {
	};
};
std::vector<Wave*> waves;
Wave *currWave;

struct s_Sphere {
	glm::vec3 pos0, pos;
	float mass;
	glm::vec3 force;
	glm::vec3 speed;
	float radius;

	void Init() {
		float x = MIN_POS_SPHERE + (rand() % static_cast<int>((MAX_POS_SPHERE - 2) - MIN_POS_SPHERE + 1));
		float y = (MIN_POS_SPHERE + 1) + (rand() % static_cast<int>((MAX_POS_SPHERE + 3) - (MIN_POS_SPHERE + 1) + 1));
		float z = 0.0f;
		pos = pos0 = glm::vec3(x, y, z);
		speed = vGravAccel;
		mass = DEF_MASS_SPHERE;
		force = mass * vGravAccel;
		radius = DEF_RADIUS_SPHERE;

		Sphere::updateSphere(pos, radius);
	}

	void Movement(float dt) {
		// Verlet
		glm::vec3 _p0 = pos0;
		glm::vec3 p0 = pos;
		glm::vec3 v0 = speed;
		glm::vec3 f = force;
		force = mass * vGravAccel;

		glm::vec3 p = p0 + (p0 - _p0) + (f / mass) * (float)glm::pow(dt, 2);
		glm::vec3 v = (p - p0) / dt;

		// Distancia para que la esfera colisione con la ola
		float distColisionSphera = 0.f;
		for (int i = 0; i < numWaves; i++) {
			currWave = waves[i];
			distColisionSphera += currWave->amplitude * cos(glm::dot(currWave->dir, p) - (currWave->w * (float)currentTime));
		}
		distColisionSphera += 3.0f;

		// Comprobar si se ha hundido/colisionado
		float esferaDistance = pos.y - radius;
		if (distColisionSphera > esferaDistance) { // Está hundida
			float dEsfera = radius * 2.f;
			float alturaSumerjida = distColisionSphera - esferaDistance;

			float VolumenSumerjido = dEsfera * dEsfera * alturaSumerjida;
			float volSphere = glm::pow(dEsfera, 3);

			if (VolumenSumerjido > volSphere) {
				VolumenSumerjido = volSphere;
			}

			// Fflotacion = 1.f * 9.8 (modulo de la gravedad = mag grav) * Volumen Sumergido = (dEsphera * dEsphera * alturaSumergida (tmp- (posSphera - radio))) * y (0,1,0);  
			glm::vec3 fContraria = { 0.0f, 1.0f, 0.0f };
			glm::vec3 Fflotacion = 1.0f * 9.8f * VolumenSumerjido * fContraria;

			/// Fuerza de Drag
			float A, modU;
			glm::vec3 U = speed;
			modU = glm::length(U);

			if (alturaSumerjida > radius) {
				A = PI * glm::pow(radius, 2);
			}
			else {
				A = PI * glm::pow(glm::distance(distColisionSphera, esferaDistance), 2);
			}

			//F = -0.5f * p * Cd * |A| * |U| * U
			glm::vec3 FDraf = -0.5f * pFluid * Cd * A * modU * U;
			force += (Fflotacion + FDraf);
		}

		pos0 = p0;
		pos = p;
		speed = v;
	}

	void Update(float dt) {
		Movement(dt);
		Sphere::updateSphere(pos, radius);
	}
};
s_Sphere sphere;


///////////////

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); //FrameRate
		
		ImGui::Checkbox("Play Simulation", &isPlaying);
		if (ImGui::Button("Reset Simulation")) {
			PhysicsReset();
		}
		ImGui::DragFloat("Reset time", &resetTime, 1.f, 0.f);

		ImGui::DragFloat("Particle Link", &particleLink, 1.f, 0.f, MAX_PARTICLE_LINK);
		ImGui::DragFloat("Fluid Density", &pFluid, 1.f);

		if (ImGui::TreeNode("Waves")) {
			ImGui::DragInt("Size Waves", &numWaves, 1.f, 0, MAX_NUM_WAVES);
			for (int i = 0; i < numWaves; i++) {
				if (ImGui::TreeNode(std::to_string(i).c_str())) {
					currWave = waves[i];
					ImGui::DragFloat3("Direction", &currWave->dir.x, 1.f);
					ImGui::DragFloat("Amplitude", &currWave->amplitude, 1.f, 0.f, MAX_AMPLI_WAVE);
					ImGui::DragFloat("Frecuency", &currWave->w, 1.f, 0.f, MAX_FREC_WAVE);

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Sphere")) {
			ImGui::DragFloat("Mass", &sphere.mass, 1.f);
			ImGui::DragFloat("Radius", &sphere.radius, 1.f, MIN_RADIUS_SPHERE, MAX_RADIUS_SPHERE);

			ImGui::TreePop();
		}
	}
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void InitParticles() {
	for (int i = 0; i < ClothMesh::numVerts; i++) {
		ptrWaveParticlesPos0[i] = ptrWaveParticlesPos[i] = glm::vec3(0.f, 0.f, 0.f);
	}
}

void InitClothMesh() {
	float posX, posY, posZ;
	posX = POS_X_CLOTH;
	posY = 0.f;
	int cont = 0;

	for (int j = 0; j < ClothMesh::numRows; j++) {
		posZ = POS_Z_CLOTH;
		for (int i = 0; i < ClothMesh::numCols; i++) {
			ptrWaveParticlesPos0[cont] = { posX, posY, posZ };
			ptrWaveParticlesPos[cont] = { posX, posY, posZ };
			cont++;
			posZ += particleLink;
		}

		posX -= particleLink;
	}

	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}

void InitWaves() {
	Wave *newWave;
	for (int i = 0; i < MAX_NUM_WAVES; i++) {
		newWave = new Wave();
		newWave->dir = DEF_WAVE_DIRS[i];
		newWave->amplitude = DEF_WAVE_AMPLIS[i];
		newWave->w = DEF_WAVE_Ws[i];
		waves.push_back(newWave);
	}
}

void ResetWaves() {
	for (int i = 0; i < MAX_NUM_WAVES; i++) {
		currWave = waves[i];
		currWave->dir = DEF_WAVE_DIRS[i];
		currWave->amplitude = DEF_WAVE_AMPLIS[i];
		currWave->w = DEF_WAVE_Ws[i];
	}
}

void UpdateWavesCloth() {
	float modDir;
	const float dy = 3.f;

	for (int i = 0; i < ClothMesh::numVerts; i++) {
		ptrWaveParticlesPos[i] = ptrWaveParticlesPos0[i];

		for (int j = 0; j < numWaves; j++) {
			currWave = waves[j];
			modDir = glm::length(currWave->dir);
			ptrWaveParticlesPos[i] -= (currWave->dir / modDir) * currWave->amplitude * sin(glm::dot(currWave->dir, ptrWaveParticlesPos0[i]) - (currWave->w * (float)currentTime));
			ptrWaveParticlesPos[i].y += currWave->amplitude * cos(glm::dot(currWave->dir, ptrWaveParticlesPos0[i]) - (currWave->w * (float)currentTime));
		}
		ptrWaveParticlesPos[i].y += dy;
	}

	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}

void PhysicsInit() {
	isPlaying = false;

	vGravAccel = DEF_GRAV_ACCEL;

	elasticCoef = DEF_ELASTIC_COEF;
	frictionCoef = DEF_FRICTION_COEF;

	pFluid = DEF_DENSI_FLUID;
	Cd = 2.0f;

	resetTime = DEF_RESET_TIME;
	currentTime = 0.f;
	
	particleLink = DEF_PARTICLE_LINK;

	numWaves = DEF_NUM_WAVES;

	InitParticles();
	InitClothMesh();
	InitWaves();
	sphere.Init();
}

void PhysicsReset() {
	currentTime = 0.f;
	numWaves = DEF_NUM_WAVES;

	InitParticles();
	InitClothMesh();
	ResetWaves();
	sphere.Init();
}

void PhysicsUpdate(float dt) {
	if (isPlaying) {
		currentTime += dt;
		if (currentTime >= resetTime) {
			PhysicsReset();
		}

		UpdateWavesCloth();
		sphere.Update(dt);
	}
}

void PhysicsCleanup() {
	delete[] ptrWaveParticlesPos0;
	delete[] ptrWaveParticlesPos;

	waves.clear();
	waves.resize(0);
}