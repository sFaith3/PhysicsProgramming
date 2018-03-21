#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <iostream>
using namespace std;


namespace ClothMesh {
	 void setupClothMesh();
	 void cleanupClothMesh();
	 void updateClothMesh(float* array_data);
	 extern const int numCols;
	 extern const int numRows;
	 extern const int numVerts;
}


bool playingSimulation;

float resetTime;

const glm::vec3 default_Grav_Accel = glm::vec3(0.0f, -9.81f, 0.0f);
glm::vec3 vGravAccel = default_Grav_Accel;

glm::vec2 kStrech, kShear, kBend;

float particleLink;

bool useCollisions, useSphereCollider;

float elasticCoef, frictionCoef;

glm::vec3 *ptrParticlesPos0 = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrParticlesPos = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrParticlesSpeed = new glm::vec3[ClothMesh::numVerts];
const float mass = 1.0f;
enum class SpringsType {
	STRETCH, SHEAR, BEND, NUMTYPES
};
glm::vec3 *ptrParticlesForce = new glm::vec3[ClothMesh::numVerts];



// Fw Declarations
void PhysicsInit();
void PhysicsCleanup();


bool show_test_window = false;
void GUI() {
	bool show = true;
	
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

	ImGui::Checkbox("Play Simulation", &playingSimulation);
	if (ImGui::Button("Reset Simulation")) {
		PhysicsInit();
	}

	ImGui::DragFloat("Reset time", &resetTime, 1.0f, 0.0f, 10.0f);
	ImGui::DragFloat3("Gravity Acceleration", &vGravAccel.x, 1.0f);

	if (ImGui::TreeNode("Spring Parameters")) {
		ImGui::DragFloat2("K Strech", &kStrech.x, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat2("K Shear", &kShear.x, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat2("K Bend", &kBend.x, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat("Particle Link", &particleLink, 1.0f, 0.0f, 10.0f);

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Collisions")) {
		ImGui::Checkbox("Use Collisions", &useCollisions);
		ImGui::Checkbox("Use Sphere Collider", &useSphereCollider);
		ImGui::DragFloat("Elastic Coeficient", &elasticCoef, 1.0f, 0.0f, 1.0f);
		ImGui::DragFloat("Friction Coeficient", &frictionCoef, 1.0f, 0.0f, 1.0f);

		ImGui::TreePop();
	}

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void InitClothMesh() {
	float posX, posY, posZ;
	posY = 10.0f;

	posX = 4.0f;
	int cont = 0;
	for (int i = 0; i < ClothMesh::numRows; i++) {
		posZ = -4.5f;
		for (int j = 0; j < ClothMesh::numCols; j++)
		{
			ptrParticlesPos0[cont] = {posX, posY, posZ};
			ptrParticlesPos[cont] = { posX, posY, posZ };
			cont++;
			posZ += particleLink;
		}

		posX -= particleLink;
	}
}

void InitParticles() {
	for (int i = 0; i < ClothMesh::numVerts; i++) {
		ptrParticlesPos0[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		ptrParticlesPos[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		ptrParticlesSpeed[i] = vGravAccel;
		ptrParticlesForce[i] = { mass * vGravAccel };
		
	}
}

void PhysicsInit() {
	playingSimulation = false;

	resetTime = 5.0f;

	kStrech = kShear = kBend = glm::vec2(10.0f, 0.0f);

	particleLink = 0.5f;

	useCollisions = useSphereCollider = true;

	elasticCoef = 0.5f;
	frictionCoef = 0.1f;

	InitParticles();

	InitClothMesh();
}

void CheckCollisions(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {

}

void ParticleMovement(int currParticle, float dt) {


	glm::vec3 _p0 = ptrParticlesPos0[currParticle];
	glm::vec3 p0 = ptrParticlesPos[currParticle];
	glm::vec3 v0 = ptrParticlesSpeed[currParticle];

	
	// Verlet Method
	glm::vec3 f = ptrParticlesForce[currParticle];
	glm::vec3 p = p0 + (p0 - _p0) + (f / mass) * glm::pow(dt, 2);
	glm::vec3 v = (p - p0) / dt;


	
	
	//CheckCollisions(p0, p, v0, v);

	ptrParticlesPos0[currParticle] = p0;
	ptrParticlesPos[currParticle] = p;
	ptrParticlesSpeed[currParticle] = v;
}

glm::vec3 CalculateCurrForce(int currPos, int nextPos, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	P1 = ptrParticlesPos[currPos];
	P2 = ptrParticlesPos[nextPos];
	v1 = ptrParticlesSpeed[currPos];
	v2 = ptrParticlesSpeed[nextPos];

	glm::vec3 f = -(kStrech[0] * (glm::length(P1 - P2) - particleLink) + kStrech[1] * glm::dot((v1 - v2), glm::normalize(P1 - P2)))*glm::normalize(P1 - P2);

	return f;
}

void CalculateStretchForce() {
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	int currPos, nextPos;
	glm::vec3 currForce;

	// Vertical
	for (int i = 0; i < ClothMesh::numRows; i++) {
		for (int j = 0; j < ClothMesh::numCols; j++) {
			int currPos = i + (ClothMesh::numCols * j);
			int nextPos = i + (ClothMesh::numCols* (j + 1));

			currForce = CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2);
			if (j != 0 && j != ClothMesh::numCols) {
				ptrParticlesForce[currPos] += -ptrParticlesForce[i + ClothMesh::numCols*(j - 1)] + currForce;
			}
			else if (j == 0) {
				ptrParticlesForce[currPos] += currForce;
			}
			else {
				ptrParticlesForce[currPos] += -ptrParticlesForce[i + ClothMesh::numCols*(j - 1)];
			}

		}

	}
}

void CalculateForces()
{
	//glm::vec3 SumFuerzas;
	CalculateStretchForce();
}

void UpdateParticles(float dt) {
	CalculateForces();
	for (int i = 0; i < ClothMesh::numVerts; i++) {
		if (i != 0 && i != ClothMesh::numCols - 1) {
			ParticleMovement(i, dt);
		}
	}
}

void PhysicsUpdate(float dt) {
	if (playingSimulation) {
		UpdateParticles(dt);
	}

	ClothMesh::updateClothMesh((float*)ptrParticlesPos);
}

void PhysicsCleanup() {
	delete[] ptrParticlesPos0;
	delete[] ptrParticlesPos;
	delete[] ptrParticlesSpeed;
	delete[] ptrParticlesForce;
}