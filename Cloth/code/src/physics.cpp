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


glm::vec3 *ptrParticlesPos = new glm::vec3[ClothMesh::numVerts]; // Posiciones de las partículas


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
	// Do your initialization code here...
	// ...................................
	ClothMesh::setupClothMesh();
	float posX, posY, posZ;
	posY = 10.0f;

	posX = 4.0f;
	int cont = 0;
	for (int i = 0; i < ClothMesh::numRows; i++)
	{
		posZ = -4.5f;
		for (int j = 0; j < ClothMesh::numCols; j++)
		{

			ptrParticlesPos[cont] = { posX,posY,posZ };
			cont++;
			posZ += 0.4f;
		}

		posX -= 0.4f;

	}
}

void PhysicsUpdate(float dt) {
	// Do your update code here...
	// ...........................
	
	
	ClothMesh::updateClothMesh((float*)ptrParticlesPos);
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
	ClothMesh::cleanupClothMesh();
	delete[] ptrParticlesPos;
}