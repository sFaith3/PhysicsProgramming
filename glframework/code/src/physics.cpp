#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <limits.h>

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
}

bool show_test_window = false;

// PARTICLES
float posParticles[SHRT_MAX][3];
int numParticlesEnabled = 0;

// EMITTER
float posEmitter[3];
int numParticlesToSpawn = 0;

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
	for (int i = numParticlesEnabled; i < numParticlesToSpawn; i++) {
		posParticles[i][0] = posEmitter[0];
		posParticles[i][1] = posEmitter[1];
		posParticles[i][2] = posEmitter[2];

		numParticlesEnabled++;
	}
}

void PhysicsUpdate(float dt) {
	// Do your update code here...
	for (int i = numParticlesEnabled; i > 0; i--) {
		//TODO Movement and collisions
	}
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
}