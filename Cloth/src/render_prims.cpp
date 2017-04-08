#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>


//Boolean variables allow to show/hide the primitives
extern bool renderSphere;
bool renderCapsule = false;
bool renderParticles = false;
extern bool renderCloth;

extern glm::vec3 spherePos;
extern float sphereRadius;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = spherePos, float radius = sphereRadius);
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

	////TODO
	//int col = 14, row = 18;
	//int clothLength = col*row;
	//float* clothArray = new float[clothLength*3];
	//for (int i = 0; i < row; ++i) {
	//	for (int j = 0; j < col; ++j) {
	//		if (i == 0 && j == 0) {
	//			clothArray[0] = 0;
	//			clothArray[1] = 5;
	//			clothArray[2] = 0;
	//		}
	//		else {
	//			clothArray[(i*col + j) * 3 + 0] = clothArray[0] + j*0.2;
	//			clothArray[(i*col + j) * 3 + 1] = clothArray[1];
	//			clothArray[(i*col + j) * 3 + 2] = clothArray[2] + i*0.2;
	//		}
	//	}
	//}



	//ClothMesh::updateClothMesh(clothArray);
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
