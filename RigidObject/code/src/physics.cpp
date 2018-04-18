#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include <vector>
#include <array>
#include <iostream>

namespace Cube {
	extern glm::vec3 verts[];
	extern void updateCube(const glm::mat4& transform);
}

///////////////

const float DEF_COEF_ELASTIC = .5f;
const float DEF_COEF_FRICTION = .1f;

const int FACES_CUBE = 6;
const int VERTEXS_CUBE = 8;

const float MASS = 1.f;
const glm::vec3 GRAVITY = MASS * glm::vec3(0.f, -9.81f, 0.f);

const glm::vec3 R_OFFSET = glm::vec3(1.f, 0.f, 0.f);

const glm::vec3 MIN_F_CUBE = glm::vec3(-200.f, 50.f, -200.f);
const glm::vec3 MAX_F_CUBE = glm::vec3(200.f, 100.f, 200.f);

const glm::mat3 I_BODY_MAT = {
	(1.f / 12.f) * MASS * 2.f, 0.f, 0.f,
	0.f, (1.f / 12.f) * MASS * 2.f, 0.f,
	0.f, 0.f, (1.f / 12.f) * MASS * 2.f
};
const glm::mat3 I_BODY_MAT_INVERSE = glm::inverse(I_BODY_MAT);

const float Epsilon = .1f;

/////// fw declarations
void CheckCollisions(glm::vec3 p0, glm::vec3 &p, glm::vec3 v,glm::vec3 w , glm::vec3 posCube, glm::mat3 I,glm::vec3 &P, glm::vec3 &L);

/////// Structs
struct Plane {
	glm::vec3 normal;
	float D;
};

struct StructCube {
	glm::vec3 pos;
	glm::vec3 vertexs0[VERTEXS_CUBE], vertexs[VERTEXS_CUBE];
	glm::quat rot;
	glm::vec3 linearMomentum;
	glm::vec3 angMomentum;
	glm::vec3 F; 
	glm::vec3 v;
	glm::vec3 w;
	glm::mat3 I;

	glm::mat4 GetTransMatrix() {
		glm::mat3 R = glm::mat3_cast(rot);

		glm::mat4 transMatrix = {
			R[0][0], R[0][1], R[0][2], 0.f,
			R[1][0], R[1][1], R[1][2], 0.f,
			R[2][0], R[2][1], R[2][2], 0.f,
			pos.x, pos.y, pos.z, 1.f
		};

		return transMatrix;
	}

	void UpdateCollisions(float dt) {
		for (int i = 0; i < VERTEXS_CUBE; i++) {
			CheckCollisions(vertexs0[i], vertexs[i],v,w,pos,I,linearMomentum,angMomentum);
			
		}
	}

	void UpdateVertexs() {
		glm::vec3 currVertex;
		for (int i = 0; i < VERTEXS_CUBE; i++) {
			currVertex = Cube::verts[i];
			vertexs0[i] = vertexs[i];
			vertexs[i] = pos + currVertex;
		}
	}

	void RigidBodyMotion(float dt) {
		// P(t + dt) = P(t) + dt * F(t)
		linearMomentum += dt * F;

		// v(t + dt) = P(t + dt) / M
		v = linearMomentum / MASS;
		// x(t + dt) = x(t) + dt * v(t + dt);
		pos += dt * v;

		// I(t)^-1 = R(t) * I^-1 * R(t)^T
		glm::mat3 R = glm::mat3_cast(rot);
		I = R * I_BODY_MAT_INVERSE * glm::transpose(R);
		// w(t) = I(t)^-1 * L(t + dt)
		w = I * angMomentum;
		// R(t + dt) = R(t) + dt * (w(t) * R(t))
		/*glm::mat3 W = {
		0.f, -w.z, w.y,
		w.z, 0.f, -w.x,
		-w.y, w.x, 0.f
		};
		rotCube += dt * (W * rotCube);*/
		rot += dt * ((0.5f * glm::quat(0.f, w)) * rot);
		rot = glm::normalize(rot);

		F = GRAVITY;
	}

	void UpdateMovement(float dt) {
		RigidBodyMotion(dt);
		UpdateVertexs();
	}

	void Update(float dt) {
		const int NUM_UPDATES = 10;
		for (int i = 1; i < NUM_UPDATES; i++) {
			UpdateCollisions(dt / i);
		}
		UpdateMovement(dt);

		glm::mat4 transMatrix = GetTransMatrix();
		Cube::updateCube(transMatrix);
	}

	void InitAngMomentum(float dt) {
		glm::vec3 r = pos + R_OFFSET;
		// T(t) = sum(ri(t) - x(t)) x Fi(t)
		glm::vec3 T = glm::cross((r - pos), F);
		// L(t + dt) = L(t) + dt * T(t)
		angMomentum = glm::vec3(dt * T);
	}

	void InitForce() {
		float x = MIN_F_CUBE.x + (rand() % static_cast<int>(MAX_F_CUBE.x - MIN_F_CUBE.x + 1));
		float y = MIN_F_CUBE.y + (rand() % static_cast<int>(MAX_F_CUBE.y - MIN_F_CUBE.y + 1));
		float z = MIN_F_CUBE.z + (rand() % static_cast<int>(MAX_F_CUBE.z - MIN_F_CUBE.z + 1));

		F = glm::vec3(x, y, z) + GRAVITY;
	}

	void InitVertexs() {
		glm::vec3 currVertex;
		for (int i = 0; i < VERTEXS_CUBE; i++) {
			currVertex = Cube::verts[i];
			vertexs0[i] = vertexs[i] = pos + currVertex;
		}
	}

	void Init(float dt) {
		pos = glm::vec3(0.f, 3.f, 0.f);
		rot = glm::quat(0.f, 0.f, 0.f, 0.f);
		linearMomentum = glm::vec3(0.f);

		InitVertexs();
		InitForce();
		InitAngMomentum(dt);
	}
};

///////////////

Plane planes[6];

float coefElastic, coefFriction;

StructCube cube;

bool show_test_window = false;

///////////////

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

/*
* D = -(Ax + By + Cz)
*/
void InitPlanes() {
	Plane newPlane;

	// Plane 1
	newPlane.normal = glm::vec3(0.f, -1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 10.0f, 5.0f));
	planes[0] = newPlane;

	// Plane 2
	newPlane.normal = glm::vec3(0.f, 1.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes[1] = newPlane;

	// Plane 3
	newPlane.normal = glm::vec3(-1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, 5.0f));
	planes[2] = newPlane;

	// Plane 4
	newPlane.normal = glm::vec3(1.f, 0.f, 0.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, -5.0f));
	planes[3] = newPlane;

	// Plane 5
	newPlane.normal = glm::vec3(0.f, 0.f, -1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(-5.0f, 0.0f, 5.0f));
	planes[4] = newPlane;

	// Plane 6
	newPlane.normal = glm::vec3(0.f, 0.f, 1.f);
	newPlane.D = -glm::dot(newPlane.normal, glm::vec3(5.0f, 0.0f, -5.0f));
	planes[5] = newPlane;
}

/*
 * Collision Correction - Impulse
 */
void CollisionParticlePlane(glm::vec3 p0, glm::vec3 &p, glm::vec3 n, float d, glm::vec3 v, glm::vec3 w, glm::vec3 posCube, glm::mat3 I, glm::vec3 &P, glm::vec3 &L) {

	//Calculamos "pä": velocidad Vertice que ha colisionado en el plano.
	// pä = va(t0) + wa(t0) x (pa(t0)- xa(t0));
	// ra = (pa(t0) - xa(t0));
	glm::vec3 ra = p - posCube;
	glm::vec3 particleVel = v + glm::cross(w, ra);

	//Calculamos vrel
	//Vrel = n(t0) * (pä(t0)-pb(to)); pb = 0;
	float Vrel = glm::dot(n, particleVel);
	
	//Una vez Calculada Vrel la utilizaremos para calcular la J
	//j = -(1 - coefElasticidad)*Vrel / ((1/Ma) + n(t0) * (Ia^-1(t0) (ra x n(t0))) x ra);
	float j = -(1 + coefElastic)*Vrel / (1 / MASS) + glm::dot(n, glm::cross(I*(glm::cross(ra, n)), ra));
	//J=j*n;
	glm::vec3 J = j*n;
	// T = ra x J;
	glm::vec3 T = glm::cross(ra, J);

	P += J;
	L += T;

		
	 
}

void CollisionParticleWithWallsAndGround(glm::vec3 p0, glm::vec3 &p, glm::vec3 v,glm::vec3 w , glm::vec3 posCube,glm::mat3 I, glm::vec3 &P, glm::vec3 &L) {
	Plane currPlane;
	glm::vec3 n;
	float d;

	for (int i = 0; i < FACES_CUBE; i++) {
		currPlane = planes[i];
		n = currPlane.normal;

		d = (glm::dot(n, p) + currPlane.D) * (glm::dot(n, p0) + currPlane.D);
		if (d < Epsilon) {
			if (d < 0.f) break;

			CollisionParticlePlane(p0, p, n, currPlane.D,v,w,posCube,I,P,L);
			break;
		}
		
	}
}

void CheckCollisions(glm::vec3 p0, glm::vec3 &p, glm::vec3 v, glm::vec3 w, glm::vec3 posCube,glm::mat3 I, glm::vec3 &P, glm::vec3 &L) {
	CollisionParticleWithWallsAndGround(p0, p,v,w,posCube,I,P,L);
}

void PhysicsInit(float dt) {
	coefElastic = DEF_COEF_ELASTIC;
	coefFriction = DEF_COEF_FRICTION;

	InitPlanes();
	cube.Init(dt);
}

void PhysicsUpdate(float dt) {
	cube.Update(dt);
}

void PhysicsCleanup() {
}