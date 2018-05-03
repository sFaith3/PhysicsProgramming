#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <vector>
#include <array>
#include <iostream>
#include <vector>


///////////////

/////// constants
const float INIT_RESET_TIME = 10.f;

const float INIT_PARTICLE_LINK = .5f;

const glm::vec3 INIT_DIR_WAVE = glm::vec3(1.f, 0.f, 0.f);
const float INIT_AMP_WAVE = 1.f;
const float INIT_W = 2.f;

/////// fw declarations
namespace ClothMesh {
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern const int numCols;
	extern const int numRows;
	extern const int numVerts;
}

namespace Sphere {
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

glm::vec3 *ptrWaveParticlesPos0 = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrWaveParticlesPos = new glm::vec3[ClothMesh::numVerts];
std::vector<glm::vec3> dirWave(2,glm::vec3(0.0f, 0.0f, 0.0f)); //TODO array/vector de vec3 que indica el num de olas

float amplitudeWave; // Probamos con 2 o 1
float w; // Frecuéncia ola

bool show_test_window = false;

///////////////


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

		if (ImGui::TreeNode("Wave")) {
			ImGui::DragFloat3("Direction", &dirWave[0].x, 1.0f);
			ImGui::DragFloat("Amplitude", &amplitudeWave, 1.0f, 0.0f, 10.0f);
			ImGui::DragFloat("Frecuency", &w, 1.0f, 0.0f, 10.0f);

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
		ptrWaveParticlesPos0[i] = glm::vec3(0.f, 0.f, 0.f);
		ptrWaveParticlesPos[i] = glm::vec3(0.f, 0.f, 0.f);
	}
}

void InitClothMesh() {
	float posX, posY, posZ;
	posY = 10.0f;

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

			ptrWaveParticlesPos[i] = glm::vec3(0.0f);
			for (int j = 0; j < dirWave.size(); j++)
			{
			modDirWave = glm::length(dirWave[j]);
			ptrWaveParticlesPos[i] += ptrWaveParticlesPos0[i] - (dirWave[j] / modDirWave) * amplitudeWave * sin(glm::dot(dirWave[j], ptrWaveParticlesPos0[i]) - (w * currentTime));
			ptrWaveParticlesPos[i].y += /*posInit + */ amplitudeWave * cos(glm::dot(dirWave[j], ptrWaveParticlesPos0[i]) - (w * currentTime));
		}
	}
}

void Reset() {
	currentTime = 0.f;

	InitParticles();
	InitClothMesh();

	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}

void PhysicsInit() {
	isPlaying = false;

	resetTime = INIT_RESET_TIME;
	currentTime = 0.f;

	particleLink = INIT_PARTICLE_LINK;

	/*for (int i = 0; i < dirWave.size(); i++)
	{*/
		dirWave[0]=INIT_DIR_WAVE;
		dirWave[1] = glm::vec3(0.0f,0.0f,0.7f);
	//}

	amplitudeWave = INIT_AMP_WAVE;
	w = INIT_W;

	InitParticles();
	InitClothMesh();

	ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
}

void PhysicsUpdate(float dt) {
	if (isPlaying) {
		currentTime += dt;
		if (currentTime >= resetTime) {
			Reset();
		}

		UpdateWave();

		ClothMesh::updateClothMesh((float*)ptrWaveParticlesPos);
	}
}

void PhysicsCleanup() {
}