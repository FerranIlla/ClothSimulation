#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>

//Boolean variables allow to show/hide the primitives
bool renderSphere = true;
bool renderCapsule = false;
bool renderParticles = false;
bool renderCloth = true;
bool show_test_window = false;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}
namespace Capsule {
	extern void setupCapsule(glm::vec3 posA = glm::vec3(-3.f, 2.f, -2.f), glm::vec3 posB = glm::vec3(-4.f, 2.f, 2.f), float radius = 1.f);
	extern void cleanupCapsule();
	extern void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius = 1.f);
	extern void drawCapsule();
}
namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}
namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

void setupPrims() {
	Sphere::setupSphere();
	Capsule::setupCapsule();
	LilSpheres::setupParticles(LilSpheres::maxParticles);
	ClothMesh::setupClothMesh();	
}
void cleanupPrims() {
	Sphere::cleanupSphere();
	Capsule::cleanupCapsule();
	LilSpheres::cleanupParticles();
	ClothMesh::cleanupClothMesh();
}
void renderPrims() {
	if (renderSphere)
		Sphere::drawSphere();
	if (renderCapsule)
		Capsule::drawCapsule();

	if (renderParticles) {
		LilSpheres::drawParticles(0, LilSpheres::maxParticles);
	}

	if (renderCloth)
		ClothMesh::drawClothMesh();

}


int col = 14, row = 18;
int clothLength = col*row;
float* clothArray = new float[clothLength * 3];

class Particle{
	public:
	glm::vec3 pos;
	glm::vec3 prePos;
	glm::vec3 velocity;
};

Particle* cloth = new Particle[252];
float* vertArray = new float[252 * 3];

void initializeCloth() {
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			if (i == 0 && j == 0) {
				cloth[0].pos = { 0,5,0 };
			}
			else {
				cloth[i*col + j].pos = { cloth[0].pos.x + j*0.2 ,cloth[0].pos.y ,cloth[0].pos.z + i*0.2 };
			}
		}
	}
}



void particleToFloatConverter() {
	for (int i = 0; i < 252; ++i) {
		vertArray[i * 3 + 0] = cloth[i].pos.x;
		vertArray[i * 3 + 1] = cloth[i].pos.y;
		vertArray[i * 3 + 2] = cloth[i].pos.z;
	}
}

void PhysicsInit() {
	//TODO
	initializeCloth();
}
void PhysicsUpdate(float dt) {
	//TODO
	particleToFloatConverter();
	ClothMesh::updateClothMesh(vertArray);
	
}
void PhysicsCleanup() {
	//TODO
}

void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}
