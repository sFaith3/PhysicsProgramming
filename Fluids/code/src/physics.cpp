#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtx\intersect.hpp>
#include <vector>
#include <array>
#include <iostream>
#include <vector>


///////////////

/////// constants
const float INIT_RESET_TIME = 10.f;

const float INIT_PARTICLE_LINK = .5f;

const float min_Pos_Sphere = 0.f;
const float max_Pos_Sphere = 4.f;
const glm::vec3 INIT_DIR_WAVE = glm::vec3(1.f, 0.f, 0.f);
const float INIT_AMP_WAVE = 1.f;
const float INIT_W = 2.f;
const glm::vec3 default_Grav_Accel = glm::vec3(0.0f, -9.81f, 0.0f);
const float mass = 1.0f;

const float default_Elastic_Coef = 0.5f;
const float default_Friction_Coef = 0.1f;

/////// fw declarations
namespace ClothMesh {
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern const int numCols;
	extern const int numRows;
	extern const int numVerts;
}

namespace Sphere {

	glm::vec3 SpherePos;
	glm::vec3 SpherePos0;
	glm::vec3 SphereForce;
	glm::vec3 SphereSpeed;
	float radiusSphere;

	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
}

void PhysicsInit();

/////// structs



/////// local variables
bool isPlaying;

float resetTime;
float currentTime;

float particleLink;

glm::vec3 vGravAccel = default_Grav_Accel;
glm::vec3 *ptrWaveParticlesPos0 = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrWaveParticlesPos = new glm::vec3[ClothMesh::numVerts];

std::vector<glm::vec3> dirWave(3,glm::vec3(1.0f, 0.0f, 0.0f)); //TODO array/vector de vec3 que indica el num de olas
std::vector<float> amplitudeWave(3, 0.0f); 
std::vector<float> w(3, 0.0f); 
/*
amplitudeWave = INIT_AMP_WAVE;
w = INIT_W;*/

//float amplitudeWave; // Probamos con 2 o 1
//float w; // Frecuéncia ola

float elasticCoef, frictionCoef;
bool show_test_window = false;

///////////////

void InitSphere() {
	float x = min_Pos_Sphere + (rand() % static_cast<int>((max_Pos_Sphere - 2) - min_Pos_Sphere + 1));
	float y = (min_Pos_Sphere + 1) + (rand() % static_cast<int>((max_Pos_Sphere + 3) - (min_Pos_Sphere + 1) + 1));
	float z = 0.0f;


	Sphere::SpherePos = Sphere::SpherePos0 = glm::vec3(x,y,z);
	Sphere::SphereSpeed = vGravAccel;
	Sphere::SphereForce = mass * vGravAccel;
	Sphere::radiusSphere = 1.0f;
	Sphere::updateSphere(Sphere::SpherePos, Sphere::radiusSphere);
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		
		ImGui::Checkbox("Play Simulation", &isPlaying);
		if (ImGui::Button("Reset Simulation")) {
			PhysicsInit();
		}

		ImGui::DragFloat("Reset time", &resetTime, 1.0f, 0.0f, 10.0f);
		ImGui::DragFloat("Particle Link", &particleLink, 1.0f, 0.0f, 10.0f);

		/*if (ImGui::TreeNode("Wave")) {
			ImGui::DragFloat3("Direction", &dirWave[0].x, 1.0f);
			ImGui::DragFloat("Amplitude", &amplitudeWave, 1.0f, 0.0f, 10.0f);
			ImGui::DragFloat("Frecuency", &w, 1.0f, 0.0f, 10.0f);

			ImGui::TreePop();
		}*/
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
		ptrWaveParticlesPos0[i] = glm::vec3(0.f, 0.f, 0.f);
		ptrWaveParticlesPos[i] = glm::vec3(0.f, 0.f, 0.f);
	}
}

void InitClothMesh() {
	float posX, posY, posZ;
	posY = 0.0f;

	posX = 4.0f;
	int cont = 0;
	for (int j = 0; j < ClothMesh::numRows; j++) {
		posZ = -4.5f;
		for (int i = 0; i < ClothMesh::numCols; i++) {
			ptrWaveParticlesPos0[cont] = { posX, posY, posZ };
			ptrWaveParticlesPos[cont] = { posX, posY, posZ };
			cont++;
			posZ += particleLink;
		}

		posX -= particleLink;
	}
}

void UpdateWave() {
	float modDirWave;

		for (int i = 0; i < ClothMesh::numVerts; i++) {

			
			ptrWaveParticlesPos[i] = ptrWaveParticlesPos0[i];
		
			for (int j = 0; j < dirWave.size(); j++)
			{
			modDirWave = glm::length(dirWave[j]);
			ptrWaveParticlesPos[i] -= (dirWave[j] / modDirWave) * amplitudeWave[j] * sin(glm::dot(dirWave[j], ptrWaveParticlesPos0[i]) - (w[j] * currentTime));
			ptrWaveParticlesPos[i].y += amplitudeWave[j] * cos(glm::dot(dirWave[j], ptrWaveParticlesPos0[i]) - (w[j] * currentTime));
			
			
			}
			ptrWaveParticlesPos[i].y += 3.0f;
	}
}

void Reset() {
	currentTime = 0.f;

	InitParticles();
	InitClothMesh();
	InitSphere();
	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}

void PhysicsInit() {
	isPlaying = false;

	resetTime = INIT_RESET_TIME;
	currentTime = 0.f;
	elasticCoef = default_Elastic_Coef;
	frictionCoef = default_Friction_Coef;

	particleLink = INIT_PARTICLE_LINK;

	/*for (int i = 0; i < dirWave.size(); i++)
	{*/
		dirWave[0] = glm::vec3(0.8f, 0.0f, 0.0f);//INIT_DIR_WAVE;
		dirWave[1] = glm::vec3(0.0f, 0.0f, 0.8f);
		dirWave[2] = glm::vec3(0.8f, 0.0f, 0.0f);
		/*dirWave[3] = glm::vec3(1.0f, 0.0f, 0.0f);
		dirWave[4] = glm::vec3(0.0f, 0.0f, 0.5f);*/
	//}

		amplitudeWave[0] = 0.5f;
		amplitudeWave[1] = 0.8f;
		amplitudeWave[2] = 0.4f;

		w[0] = 1.2f;
		w[1] = 1.8f;
		w[2] = 1.6f;
	//amplitudeWave = INIT_AMP_WAVE;
	//w = INIT_W;

	InitParticles();
	InitClothMesh();
	InitSphere();
	for (int i = 0; i < ClothMesh::numVerts; i++) {
		//ptrWaveParticlesPos[i].y -= 9.0f;

	}
	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}


void SphereMovement(float dt) {

	glm::vec3 _p0 = Sphere::SpherePos0;
	glm::vec3  p0 = Sphere::SpherePos;
	glm::vec3  v0 = Sphere::SphereSpeed;

	//Verlet
	glm::vec3 f = Sphere::SphereForce;
	Sphere::SphereForce = mass *vGravAccel;
	glm::vec3 p = p0 + (p0 - _p0) + (f / mass) * glm::pow(dt, 2);
	glm::vec3 v = (p - p0) / dt;

 
	//glm::vec3 tmp = amplitudeWave * cos(glm::dot(dirWave[j], p) - (w * currentTime));
	float distColisionSphera=0.0f;
	for (int i = 0; i < dirWave.size(); i++)
	{
		distColisionSphera += amplitudeWave[i] * cos(glm::dot(dirWave[i], p) - (w[i]*currentTime));
	}

	//if(tmp.y) ? > possphera - Radio -> Está hundido != -> Aun no ha llegado.
	//Si está hundido -> Fflotacion = 1.f * 9.8 (modulo de la gravedad = mag grav) * Volumen Sumergido =(dEsphera * dEsphera * alturaSumergida (tmp- (posSphera - radio))) * y (0,1,0);  
	float Esferadistance = Sphere::SpherePos.y - Sphere::radiusSphere;
	if (distColisionSphera > Esferadistance)
	{
		float dEsfera = Sphere::radiusSphere * 2;
		float alturaSumerjida = distColisionSphera - Esferadistance;
		glm::vec3 fContraria = { 0.0f, 1.0f, 0.0f };
		float VolumenSumerjido = dEsfera * dEsfera * alturaSumerjida;
		if (VolumenSumerjido > dEsfera*dEsfera*dEsfera)
		{
			VolumenSumerjido = dEsfera*dEsfera*dEsfera;
		}
		glm::vec3 Fflotacion = 1.0f * 9.8f * VolumenSumerjido * fContraria;
		Sphere::SphereForce += Fflotacion;
	}
	
	Sphere::SpherePos0 = p0;
	Sphere::SpherePos = p;
	Sphere::SphereSpeed = v;

}
void PhysicsUpdate(float dt) {
	if (isPlaying) {
		currentTime += dt;
		if (currentTime >= resetTime) {
			Reset();
		}

		UpdateWave();
		
		SphereMovement(dt);
		Sphere::updateSphere(Sphere::SpherePos, 1);
	
		ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
	}
}

void PhysicsCleanup() {
}