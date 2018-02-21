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
int numParticlesToSpawn = 100;

// PARTICLES
  //Buffer de posición de Particulas
glm::vec3 *posParticle = new glm::vec3[numParticlesToSpawn];  // Posición de Cada Particula
 //Buffer de velocidades de Particulas
glm::vec3 *velParticle = new glm::vec3[numParticlesToSpawn]; // Velocidad de Cada Particula
int numParticlesEnabled = 0;



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
	// Do your initialization code here...
	posEmitter = { 0.0f,2.0f,0.0f };
	for (int i = numParticlesEnabled; i < numParticlesToSpawn;i+=3) {
		/*posParticle = posEmitter;
		ptrPosParticle[i] = posParticle.x;
		ptrPosParticle[i+1] = posParticle.y;
		ptrPosParticle[i+2] = posParticle.z;
		numParticlesEnabled++;*/
		
	}
}

void PhysicsUpdate(float dt) {
	// Do your update code here...
	//for (int i = numParticlesEnabled; i > 0; i--) {
		
		//LilSpheres::updateParticles(0, numParticlesEnabled, ptrPosParticle);
		//TODO Movement and collisions
	//}
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	delete[] posParticle;
	delete[] velParticle;
	// ............................
}