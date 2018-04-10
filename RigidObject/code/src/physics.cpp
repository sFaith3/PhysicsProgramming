#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <vector>

namespace Cube {
	extern void updateCube(const glm::mat4& transform);
}

//

const float MASS = 1.f;
const glm::vec3 GRAVITY = MASS * glm::vec3(0.f, -9.81f, 0.f);

const glm::mat3 I_BODY_MAT = {
	(1.f / 12.f) * MASS * 2.f, 0.f, 0.f,
	0.f, (1.f / 12.f) * MASS * 2.f, 0.f,
	0.f, 0.f, (1.f / 12.f) * MASS * 2.f
};
const glm::mat3 I_BODY_MAT_INVERSE = glm::inverse(I_BODY_MAT);

glm::vec3 posCube;
glm::quat rotCube;
glm::vec3 linearMomentumCube;
glm::vec3 angMomentumCube;

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
	rotCube = glm::quat(0.f, 0.f, 0.f, 0.f);
	linearMomentumCube = glm::vec3(0.f);
	angMomentumCube = glm::vec3(0.f);
}

void UpdateCube(float dt) {
	glm::vec3 F = GRAVITY;
	// P(t + dt) = P(t) + dt * F(t)
	linearMomentumCube += dt * F;

	glm::vec3 r = posCube + glm::vec3(1.f, 0.f, 0.f);
	// T(t) = sum(ri(t) - x(t)) x Fi(t)
	glm::vec3 T = glm::cross((r - posCube), F);
	// L(t + dt) = L(t) + dt * T(t)
	angMomentumCube += dt * T;

	// v(t + dt) = P(t + dt) / M
	glm::vec3 v = linearMomentumCube / MASS;
	// x(t + dt) = x(t) + dt * v(t + dt);
	posCube += dt * v;

	// I(t)^-1 = R(t) * I^-1 * R(t)^T
	glm::mat3 R = glm::mat3_cast(rotCube);
	glm::mat3 I = R * I_BODY_MAT_INVERSE * glm::transpose(R);
	// w(t) = I(t)^-1 * L(t + dt)
	glm::vec3 w = I * angMomentumCube;
	// R(t + dt) = R(t) + dt * (w(t) * R(t))
	/*glm::mat3 W = {
		0.f, -w.z, w.y,
		w.z, 0.f, -w.x,
		-w.y, w.x, 0.f
	};
	rotCube += dt * (W * rotCube);*/
	rotCube += dt * ((1.f / 2.f) * glm::quat(0.f, w) * rotCube);
	glm::normalize(rotCube);

	R = glm::mat3_cast(rotCube);
	glm::mat4 transMatrix = {
		R[0][0], R[0][1], R[0][2], 0.f,
		R[1][0], R[1][1], R[1][2], 0.f,
		R[2][0], R[2][1], R[2][2], 0.f,
		posCube.x, posCube.y, posCube.z, 1.f
	};

	Cube::updateCube(transMatrix);
}

void PhysicsUpdate(float dt) {
	UpdateCube(dt);
}

void PhysicsCleanup() {
}