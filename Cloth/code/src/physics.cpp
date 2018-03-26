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
	for (int j = 0; j < ClothMesh::numRows; j++) {
		posZ = -4.5f;
		for (int i = 0; i < ClothMesh::numCols; i++) {
			ptrParticlesPos0[cont] = { posX, posY, posZ };
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

	glm::vec3 f = -(kStrech.x * (glm::distance(P1, P2) - L) + kStrech.y * glm::dot((v1 - v2), glm::normalize(P1 - P2))) * glm::normalize(P1 - P2);
	
	return f;
}

void ParticlesForces(){
	int currPos, nextPos;
	glm::vec3 P1, P2;
	glm::vec3 v1, v2;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = i + (ClothMesh::numCols * j);
			currForce = ptrParticlesForce[currPos];

			/// First Col
			if (i == 0 && (j > 0 && j < ClothMesh::numRows - 1)) {
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos , P1, P2, v1, v2, particleLink);

				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos + 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos + 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				if (j < (ClothMesh::numRows - 1) - 1) {
					nextPos = currPos + (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (j >= 2) {
						nextPos = currPos - (ClothMesh::numCols * 2);
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}

				// Horizontal
				nextPos = currPos + 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// Last Col
			else if (i == ClothMesh::numCols - 1 && (j > 0 && j < ClothMesh::numRows - 1)) {
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos - 1) - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				if (j < (ClothMesh::numRows - 1) - 1) {
					nextPos = currPos + (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (j >= 2) {
						nextPos = currPos - (ClothMesh::numCols * 2);
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}

				// Horizontal
				nextPos = currPos - 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// First row
			else if ((i > 0 && i < ClothMesh::numCols - 1) && j == 0) {
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos + 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos + (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				if (i < (ClothMesh::numCols - 1) - 1) {
					nextPos = currPos + 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (i >= 2) {
						nextPos = currPos - 2;
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}
			}

			/// Last row
			else if ((i > 0 && i < ClothMesh::numCols - 1) && j == ClothMesh::numRows - 1) {
				/// Structural
				// Vertical
				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos + 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos - (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				if (i < (ClothMesh::numCols - 1) - 1) {
					nextPos = currPos + 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (i >= 2) {
						nextPos = currPos - 2;
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}
			}

			/// Esquina superior izquierda
			else if (i == 0 && j == 0) {
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos + 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos + (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// Esquina superior derecha
			else if (i == ClothMesh::numCols - 1 && j == 0) {
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos + (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos - 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// Esquina inferior izquierda
			else if (i == 0 && j == ClothMesh::numRows - 1) {
				/// Stretch
				// Vertical
				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos + 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos - (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// Esquina inferior derecha
			else if (i == ClothMesh::numCols - 1 && j == ClothMesh::numRows - 1) {
				/// Stretch
				// Vertical
				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				nextPos = currPos - (ClothMesh::numCols * 2);
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos - 2;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
			}

			/// Lo demás
			else if((i > 0 && j > 0) && (i < ClothMesh::numCols - 1 && j < ClothMesh::numRows - 1)){
				/// Stretch
				// Vertical
				nextPos = currPos + ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = currPos - ClothMesh::numCols;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				// Horizontal
				nextPos = currPos + 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = currPos - 1;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Shear
				nextPos = (currPos - 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos - 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos + 1) - ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				nextPos = (currPos + 1) + ClothMesh::numCols ;
				currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

				/// Bend
				// Vertical
				if (j < (ClothMesh::numRows - 1) - 1) {
					nextPos = currPos + (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (j >= 2) {
						nextPos = currPos - (ClothMesh::numCols * 2);
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - (ClothMesh::numCols * 2);
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}

				// Horizontal
				if (i < (ClothMesh::numCols - 1) - 1) {
					nextPos = currPos + 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);

					if (i >= 2) {
						nextPos = currPos - 2;
						currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
					}
				}
				else {
					nextPos = currPos - 2;
					currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink);
				}

			}

			ptrParticlesForce[currPos] += currForce;
		}
	}
}

void UpdateParticles(float dt) {
	ParticlesForces();

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