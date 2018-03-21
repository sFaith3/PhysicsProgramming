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


enum class SpringsType {
	STRETCH, SHEAR, BEND, NUMTYPES
};


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
		ptrParticlesForce[i] = mass * vGravAccel;
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

glm::vec3 CalculateCurrForce(int currPos, int nextPos, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2, float L) {
	P1 = ptrParticlesPos[currPos];
	P2 = ptrParticlesPos[nextPos];
	v1 = ptrParticlesSpeed[currPos];
	v2 = ptrParticlesSpeed[nextPos];

	glm::vec3 f = -(kStrech[0] * (glm::length(P1 - P2) - L) + kStrech[1] * glm::dot((v1 - v2), glm::normalize(P1 - P2))) * glm::normalize(P1 - P2);

	return f;
}

void BendHorizontalForces() {
	int lastPos, currPos, nextPos;
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	float particleLinkBend = 2.f * particleLink;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = (ClothMesh::numCols * i) + j;

			currForce = CalculateCurrForce(currPos, currPos + 2, P1, P2, v1, v2, particleLinkBend);
			if (j == 0) {
				ptrParticlesForce[currPos] += currForce;
			}
			else if (j >= ClothMesh::numRows - 1) {
				ptrParticlesForce[currPos] += ptrParticlesForce[currPos - 2];
			}
			else {
				ptrParticlesForce[currPos] += ptrParticlesForce[currPos - 2] + currForce;
			}
		}
		//-----------
		/*currForce = ptrParticlesForce[currPos];
		cout << "(" << currForce.x << ", " << currForce.y << ", " << currForce.z << ")" << endl;*/
		//-----------
	}
}

void BendVerticalForces() {
	int lastPos, currPos, nextPos;
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	float particleLinkBend = 2.f * particleLink;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			lastPos = i + (ClothMesh::numRows * (j - 2));
			currPos = i + (ClothMesh::numRows * j);
			nextPos = i + (ClothMesh::numRows * (j + 2));

			currForce = CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLinkBend);
			if (i == 0) {
				ptrParticlesForce[currPos] += currForce;
			}
			else if (i >= ClothMesh::numCols - 1) {
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos];
			}
			else {
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos] + currForce;
			}
		}
	}
}

void CalculateBendForces() {
	BendVerticalForces();
	BendHorizontalForces();
}

void CalculateShearForces() {
	int lastPos, currPos, nextPos;
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	float particleLinkShear = glm::sqrt(glm::pow(particleLink, 2) + glm::pow(particleLink, 2));
	glm::vec3 currForce;

	// Diag sup izq a inf derecha
	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = (ClothMesh::numCols * i) + j;
			lastPos = (currPos - 1) + (i + (ClothMesh::numRows * (j - 1)));
			nextPos = (currPos + 1) + (i + (ClothMesh::numRows * (j + 1)));

			currForce = CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLinkShear);
			if ((i == 0 && j == ClothMesh::numRows) || (j == 0 && i == ClothMesh::numCols)) { // No hay F
			}
			else if (i == 0 || j == 0) { // Solo hay 1 F
				ptrParticlesForce[currPos] += currForce;
			}
			else if (i == ClothMesh::numCols || j == ClothMesh::numRows) { // 1 F sentido opuesto
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos];
			}
			else { // 2 Fs. Normal
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos] + currForce;
			}
		}
	}

	// Diag inf izq a sup derecha
	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = (ClothMesh::numCols * i) + j;
			lastPos = (currPos - 1) + (i + (ClothMesh::numRows * (j + 1)));
			nextPos = (currPos + 1) + (i + (ClothMesh::numRows * (j - 1)));

			currForce = CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLinkShear);
			if ((i == 0 && j == 0) || (j == ClothMesh::numRows && i == ClothMesh::numCols)) { // No hay F
			}
			else if (j == 0 || i == ClothMesh::numCols) { // 1 F sentido normal
				ptrParticlesForce[currPos] += currForce;
			}
			else if (i == 0 || j == ClothMesh::numRows) { // 1 F sentido opuesto
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos];
			}			
			else { // 2 Fs. Normal
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos] + currForce;
			}
		}
	}
}

void StretchHorizontalForces() {
	int lastPos, currPos, nextPos;
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = (ClothMesh::numCols * i) + j;

			currForce = CalculateCurrForce(currPos, currPos + 1, P1, P2, v1, v2, particleLink);
			if (j == 0) {
				ptrParticlesForce[currPos] += currForce;
			}
			else if(j == ClothMesh::numRows) {
				ptrParticlesForce[currPos] += ptrParticlesForce[currPos - 1];
			}
			else {
				ptrParticlesForce[currPos] += ptrParticlesForce[currPos - 1] + currForce;
			}
		}
	}
}

void StretchVerticalForces() {
	int lastPos, currPos, nextPos;
	glm::vec3 P1;
	glm::vec3 P2;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			lastPos = i + (ClothMesh::numRows * (j - 1));
			currPos = i + (ClothMesh::numRows * j);
			nextPos = i + (ClothMesh::numRows * (j + 1));

			currForce = CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			if (i == 0) {
				ptrParticlesForce[currPos] += currForce;
			}
			else if(i == ClothMesh::numCols) {
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos];
			}
			else {
				ptrParticlesForce[currPos] += ptrParticlesForce[lastPos] + currForce;
			}
		}
	}
}

void CalculateStretchForces() {
	StretchVerticalForces();
	StretchHorizontalForces();
}

void CalculateForces()
{
	CalculateStretchForces();
	CalculateShearForces();
	CalculateBendForces();
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