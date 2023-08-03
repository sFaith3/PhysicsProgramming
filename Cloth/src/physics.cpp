#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <glm\gtx\intersect.hpp>
#include <vector>
#include <iostream>
using namespace std;


namespace ClothMesh {
	 extern void cleanupClothMesh();
	 extern void updateClothMesh(float* array_data);
	 extern const int numCols;
	 extern const int numRows;
	 extern const int numVerts;
}

namespace Sphere {
	extern glm::vec3 posSphere;
	extern float radiusSphere;
	extern void updateSphere(glm::vec3 pos, float radius);
}

enum class SpringsType {
	STRETCH, SHEAR, BEND
};

struct Plane {
	glm::vec3 normal;
	float D;
};


const int faces_Cube = 6;

const float min_Pos_Sphere = 0.f;
const float max_Pos_Sphere = 4.f;

const float default_Reset_Time = 5.f;

const glm::vec2 default_Ks = glm::vec2(1000.0f,50.0f);

const float default_Particle_Link = 0.5f;

const float default_Elastic_Coef = 0.5f;
const float default_Friction_Coef = 0.1f;

const glm::vec3 default_Grav_Accel = glm::vec3(0.0f, -9.81f, 0.0f);
const float mass = 1.0f;


bool playingSimulation;

float resetTime;

glm::vec3 vGravAccel = default_Grav_Accel;

glm::vec2 kStrech, kShear, kBend;

float particleLink, lastParticleLink;

bool useCollisions;

float elasticCoef, frictionCoef;

vector<Plane> planes;

glm::vec3 *ptrParticlesPos0 = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrParticlesPos = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrParticlesSpeed = new glm::vec3[ClothMesh::numVerts];
glm::vec3 *ptrParticlesForce = new glm::vec3[ClothMesh::numVerts];

extern bool renderSphere;

float currTime;

// Fw Declarations
void PhysicsInit();
void PhysicsCleanup();


bool show_test_window = false;
void GUI() {
	bool show = true;
	
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); // FrameRate

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
		ImGui::Checkbox("Use Sphere Collider", &renderSphere);
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

void InitSphere() {
	float x = min_Pos_Sphere + (rand() % static_cast<int>((max_Pos_Sphere - 2) - min_Pos_Sphere + 1));
	float y = (min_Pos_Sphere + 1) + (rand() % static_cast<int>((max_Pos_Sphere + 3) - (min_Pos_Sphere + 1) + 1));
	float z = 0.0f;

	Sphere::posSphere = { x, y, z };
	Sphere::radiusSphere = 1.0f;
}

void InitColliders() {
	useCollisions = false;

	// Sphere
	renderSphere = false;
	InitSphere();
}

/*
* D = -(Ax + By + Cz)
*/
void InitPlanes() {
	Plane newPlane;

	// Plane 1
	newPlane.normal = glm::vec3(0.f, -1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 10.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 2
	newPlane.normal = glm::vec3(0.f, 1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);

	// Plane 3
	newPlane.normal = glm::vec3(-1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 4
	newPlane.normal = glm::vec3(1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);

	// Plane 5
	newPlane.normal = glm::vec3(0.f, 0.f, -1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, 5.0f));
	planes.push_back(newPlane);

	// Plane 6
	newPlane.normal = glm::vec3(0.f, 0.f, 1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, -5.0f));
	planes.push_back(newPlane);
}

void PhysicsInit() {
	playingSimulation = false;

	resetTime = default_Reset_Time;

	kStrech = kShear = kBend = default_Ks;

	particleLink = lastParticleLink = default_Particle_Link;

	elasticCoef = default_Elastic_Coef;
	frictionCoef = default_Friction_Coef;

	currTime = 0.f;

	InitColliders();
	InitPlanes();
	InitParticles();
	InitClothMesh();
}

void PhysicsReinit() {
	currTime = 0.f;

	InitSphere();
	InitParticles();
	InitClothMesh();
}

void CollisionParticlePlane(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v, glm::vec3 n, float d) {
	p0 = p;
	v0 = v;

	// With Elasticity
	p = p0 - (1 + elasticCoef) * (glm::dot(n, p0) + d) * n;
	v = v0 - (1 + elasticCoef) * glm::dot(n, v0) * n;

	// With Friction
	glm::vec3 vn = glm::dot(n, v) * n;
	glm::vec3 vt = v - vn;
	v = v - (frictionCoef * vt);
}



void CollisionParticleWithSphere(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	// Calculamos si la distancia entre la particula y el centro de la esfera - el Radio <=0
	// Fórmula: d |C-P| = sqrt((pow((Cx - Px),2), pow((Cy - Py),2), pow((Cz - Pz),2))
	float d = glm::sqrt(glm::pow(Sphere::posSphere.x - p.x, 2) + glm::pow(Sphere::posSphere.y - p.y, 2) + glm::pow(Sphere::posSphere.z - p.z, 2));
	if (d < Sphere::radiusSphere) {
		glm::vec3 p0p = p - p0;
		float alfa1 = 0, alfa2 = 0, resultAlfa = 0;

		// Usaremos la equación de la recta y la equacion de la esfera.
		// Sacaremos una equiacion Ax^2 + bx + c = 0 para descubrir la alfa
		//float a = glm::pow(p0.x - Sphere::posSphere.x, 2) + glm::pow(p0.y - Sphere::posSphere.y, 2) + glm::pow(p0.z - Sphere::posSphere.z, 2) - glm::pow(Sphere::radiusSphere, 2) +
		//	2 * ((p0.x - Sphere::posSphere.x) + (p0.y - Sphere::posSphere.y) + (p0.z - Sphere::posSphere.z));
		//float b = 2 * ((p.x - p0.x) + (p.y - p0.y) + (p.z - p0.z));
		//float c = glm::pow(glm::pow(p.x - p0.x, 2) + glm::pow(p.y - p0.y, 2) + glm::pow(p.z - p0.z, 2), 2);
		//
		//alfa1 = -b  +  glm::sqrt(glm::pow(b, 2) - (4 * a*c))  /  2 * a;
		//alfa2 = -b - glm::sqrt(glm::pow(b, 2) - (4 * a*c)) / 2 * a;
		//
		//(glm::abs(alfa1) > glm::abs(alfa2)) ? resultAlfa = alfa2 : resultAlfa = alfa1;
		//
		//// recta: r = p0 + &p0p; buscaremos el valor "&".
		//// Una vez tengamos alfa conoceremos el punto de colision.
		//glm::vec3 PuntoColision = { p0.x + p0p.x *resultAlfa, p0.y + p0p.y *resultAlfa, p0.z + p0p.z *resultAlfa };
		//glm::vec3 n = glm::normalize(PuntoColision - Sphere::posSphere);

		glm::vec3 PuntoColision;
		glm::vec3 n;
		glm::intersectLineSphere(p, p0, Sphere::posSphere, Sphere::radiusSphere, PuntoColision, n);
		float D = -glm::dot(n, PuntoColision);
		CollisionParticlePlane(p0, p, v0, v, n, D);
	}
}

void CollisionParticleWithWallsAndGround(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	Plane currPlane;
	glm::vec3 n;
	float d;

	for (int i = 0; i < faces_Cube; i++) {
		currPlane = planes[i];
		n = currPlane.normal;

		d = (glm::dot(n, p) + currPlane.D) * (glm::dot(n, p0) + currPlane.D);
		if (d < 0.0f) {
			CollisionParticlePlane(p0, p, v0, v, n, currPlane.D);
			break;
		}
	}
}

void CheckCollisions(glm::vec3 p0, glm::vec3 &p, glm::vec3 v0, glm::vec3 &v) {
	if (useCollisions) {
		CollisionParticleWithWallsAndGround(p0, p, v0, v);
	}

	if (renderSphere) {
		CollisionParticleWithSphere(p0, p, v0, v);
	}
}

void ParticleMovement(int currParticle, float dt) {
	
	glm::vec3 _p0 = ptrParticlesPos0[currParticle];
	glm::vec3 p0 = ptrParticlesPos[currParticle];
	glm::vec3 v0 = ptrParticlesSpeed[currParticle];
	
	// Verlet Method
	glm::vec3 f = ptrParticlesForce[currParticle];
	glm::vec3 p = p0 + (p0 - _p0) + (f / mass) * (float)glm::pow(dt, 2);
	glm::vec3 v = (p - p0) / dt;
	
	CheckCollisions(p0, p, v0, v);

	ptrParticlesPos0[currParticle] = p0;
	ptrParticlesPos[currParticle] = p;
	ptrParticlesSpeed[currParticle] = v;
	
}

glm::vec3 CalculateCurrForce(int currPos, int nextPos, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2, float L, SpringsType currSpringTypeEnum) {
	
	P1 = ptrParticlesPos[currPos];
	P2 = ptrParticlesPos[nextPos];
	v1 = ptrParticlesSpeed[currPos];
	v2 = ptrParticlesSpeed[nextPos];
	glm::vec3 f;

	switch (currSpringTypeEnum) {
	case SpringsType::STRETCH:
		f = -(kStrech.x * (glm::distance(P1, P2) - L) + kStrech.y * glm::dot((v1 - v2), glm::normalize(P1 - P2))) * glm::normalize(P1 - P2);
		break;

	case SpringsType::SHEAR:
		L = glm::sqrt(glm::pow(L, 2) + glm::pow(L, 2));
		f = -(kShear.x * (glm::distance(P1, P2) - L) + kShear.y * glm::dot((v1 - v2), glm::normalize(P1 - P2))) * glm::normalize(P1 - P2);
		break;

	case SpringsType::BEND:
		L *= 2.f;
		f = -(kBend.x * (glm::distance(P1, P2) - L) + kBend.y * glm::dot((v1 - v2), glm::normalize(P1 - P2))) * glm::normalize(P1 - P2);
		break;

	default:
		break;
	}

	return f;
}

void RemainingForces(int i, int j, int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos - 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos + 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos + 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	if (j < (ClothMesh::numRows - 1) - 1) {
		nextPos = currPos + (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (j >= 2) {
			nextPos = currPos - (ClothMesh::numCols * 2);
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}

	// Horizontal
	if (i < (ClothMesh::numCols - 1) - 1) {
		nextPos = currPos + 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (i >= 2) {
			nextPos = currPos - 2;
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}
}

void LowerRightCornerForces(int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos - (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	nextPos = currPos - 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void LowerLeftCornerForces(int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos + 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos - (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	nextPos = currPos + 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void UpperRightCornerForces(int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos + (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	nextPos = currPos - 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void UpperLeftCornerForces(int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos + 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos + (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	nextPos = currPos + 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void LastRowForces(int i, int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Structural
	// Vertical
	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos + 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos - (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	if (i < (ClothMesh::numCols - 1) - 1) {
		nextPos = currPos + 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (i >= 2) {
			nextPos = currPos - 2;
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}
}

void FirstRowForces(int i, int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos + 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	nextPos = currPos + (ClothMesh::numCols * 2);
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

	// Horizontal
	if (i < (ClothMesh::numCols - 1) - 1) {
		nextPos = currPos + 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (i >= 2) {
			nextPos = currPos - 2;
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - 2;
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}
}

void LastColumnForces(int j, int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos - 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos - 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos - 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	if (j < (ClothMesh::numRows - 1) - 1) {
		nextPos = currPos + (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (j >= 2) {
			nextPos = currPos - (ClothMesh::numCols * 2);
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}

	// Horizontal
	nextPos = currPos - 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void FirstColumnForces(int j, int currPos, int nextPos, glm::vec3 &currForce, glm::vec3 P1, glm::vec3 P2, glm::vec3 v1, glm::vec3 v2) {
	/// Stretch
	// Vertical
	nextPos = currPos + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	nextPos = currPos - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	// Horizontal
	nextPos = currPos + 1;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::STRETCH);

	/// Shear
	nextPos = (currPos + 1) + ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	nextPos = (currPos + 1) - ClothMesh::numCols;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::SHEAR);

	/// Bend
	// Vertical
	if (j < (ClothMesh::numRows - 1) - 1) {
		nextPos = currPos + (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);

		if (j >= 2) {
			nextPos = currPos - (ClothMesh::numCols * 2);
			currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
		}
	}
	else {
		nextPos = currPos - (ClothMesh::numCols * 2);
		currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
	}

	// Horizontal
	nextPos = currPos + 2;
	currForce += CalculateCurrForce(currPos, nextPos, P1, P2, v1, v2, particleLink, SpringsType::BEND);
}

void ParticlesForces(){
	int currPos, nextPos = 0;
	glm::vec3 P1, P2;
	glm::vec3 v1, v2;
	glm::vec3 currForce;

	for (int i = 0; i < ClothMesh::numCols; i++) {
		for (int j = 0; j < ClothMesh::numRows; j++) {
			currPos = i + (ClothMesh::numCols * j);
			
			currForce = glm::vec3(0.0f, -9.81f, 0.0f);
			
			if (i == 0 && (j > 0 && j < ClothMesh::numRows - 1)) {
				FirstColumnForces(j, currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if (i == ClothMesh::numCols - 1 && (j > 0 && j < ClothMesh::numRows - 1)) {
				LastColumnForces(j, currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if ((i > 0 && i < ClothMesh::numCols - 1) && j == 0) {
				FirstRowForces(i, currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if ((i > 0 && i < ClothMesh::numCols - 1) && j == ClothMesh::numRows - 1) {
				LastRowForces(i, currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if (i == 0 && j == 0) {
				UpperLeftCornerForces(currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if (i == ClothMesh::numCols - 1 && j == 0) {
				UpperRightCornerForces(currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if (i == 0 && j == ClothMesh::numRows - 1) {
				LowerLeftCornerForces(currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if (i == ClothMesh::numCols - 1 && j == ClothMesh::numRows - 1) {
				LowerRightCornerForces(currPos, nextPos, currForce, P1, P2, v1, v2);
			}
			else if((i > 0 && j > 0) && (i < ClothMesh::numCols - 1 && j < ClothMesh::numRows - 1)){
				RemainingForces(i, j, currPos, nextPos, currForce, P1, P2, v1, v2);
			}

			ptrParticlesForce[currPos] = currForce;
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


	/*glm::vec3 prevP;
	glm::vec3 v_P1P2; 
	for (int n = 0; n < 10; n++)
	{
		for (int i = 0; i < ClothMesh::numVerts; i++) {
			if (i != 0 && i != ClothMesh::numCols - 1) {
				prevP = ptrParticlesPos[i - 1];
				v_P1P2 = prevP - ptrParticlesPos[i];
				if (glm::length(v_P1P2) > particleLink)
				{
					ptrParticlesPos[i] = prevP + (glm::normalize(v_P1P2) * particleLink);
				}
			}
		}
	}*/
}

void PhysicsUpdate(float dt) {
	if (playingSimulation) {
		currTime += dt;
		if (currTime >= resetTime) {
			PhysicsReinit();
		}

		const float NUM_UPDATES = 10.f;
		for (int i = 0; i < static_cast<int>(NUM_UPDATES); i++){
			UpdateParticles(dt / NUM_UPDATES);
		}
	}
	else if (lastParticleLink != particleLink) {
		lastParticleLink = particleLink;
		InitClothMesh();
	}

	ClothMesh::updateClothMesh((float*)ptrParticlesPos);

	if (renderSphere) {
		Sphere::updateSphere(Sphere::posSphere, Sphere::radiusSphere);
	}
}

void PhysicsCleanup() {
	delete[] ptrParticlesPos0;
	delete[] ptrParticlesPos;
	delete[] ptrParticlesSpeed;
	delete[] ptrParticlesForce;
}