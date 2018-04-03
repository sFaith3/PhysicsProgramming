#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>

namespace Cube {
	extern void updateCube(const glm::mat4& transform);
}

//

glm::vec3 posCube;
glm::vec3 rotCube;
glm::vec3 linearMomentumCube;
glm::vec3 angMomentumCube;

glm::mat4 transMatrix;

bool show_test_window = false;
void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	// Do your GUI code here....
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		
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
	posCube = glm::vec3(0.f, 3.f, 0.f);
	rotCube = glm::vec3(0.f);
	linearMomentumCube = glm::vec3(0.f);
	angMomentumCube = glm::vec3(0.f);

	transMatrix = glm::mat4(0.f);
}

void PhysicsUpdate(float dt) {
	transMatrix = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		posCube.x, posCube.y, posCube.z, 1.f
	};

	Cube::updateCube(transMatrix);
}

void PhysicsCleanup() {
}