#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <limits.h>
#include <glm/vec3.hpp>

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

bool show_test_window = false;

// EMITTER
glm::vec3 posEmitter;
int numParticlesToSpawn = 100; //TODO Para la GUI

// PARTICLES
glm::vec3 *ptrPosParticles = new glm::vec3[numParticlesToSpawn];	//Posiciones de las partículas
glm::vec3 *ptrSpeedParticles = new glm::vec3[numParticlesToSpawn]; //Velocidades de las partículas
int numParticlesEnabled = 0;

const glm::vec3 GRAVITY = { 0.0f, -9.81f, 0.0f };


void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	// Do your GUI code here....
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		//TODO A number field to set the position to spawn and number of particles to emit with "+="
	}
	// .........................
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void PhysicsInit() {
	posEmitter = { 0.0f, 2.0f, 0.0f }; //TODO Without magical numbers

	for (int i = numParticlesEnabled; i < numParticlesToSpawn; i++) {
		ptrPosParticles[i] = posEmitter;
		numParticlesEnabled++;
	}
}

void PhysicsUpdate(float dt) {
	//LilSpheres::updateParticles(1, numParticlesEnabled, (float*)ptrPosParticles);

	for (int i = numParticlesEnabled - 1; i >= 0; i--) {
		LilSpheres::updateParticles(i, 1, (float*)ptrPosParticles);

		// Movement
		ptrPosParticles[i] = ptrPosParticles[i] + dt * ptrSpeedParticles[i];
		ptrSpeedParticles[i] = ptrSpeedParticles[i] + dt * GRAVITY;

		// TODO Collisions
		
	}
}

void PhysicsCleanup() {
	delete[] ptrPosParticles;
	delete[] ptrSpeedParticles;
}