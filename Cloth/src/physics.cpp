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

glm::vec3 spherePos(0.f, 3.f, 0.f);
float sphereRadius = 1.f;
float gravity = 9.8f;

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
int clothLength = col*row; // 14*18=252

class Particle{
	public:
	glm::vec3 pos;
	glm::vec3 prePos;
	glm::vec3 velocity;
	glm::vec3 totalForce;
};

Particle* cloth = new Particle[clothLength];
float* vertArray = new float[clothLength * 3];
float springLenght = 0.2f; 

void initializeCloth() {
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			if (i == 0 && j == 0) {
				cloth[0].pos = { 0,5,0 };
				cloth[0].prePos = {cloth[0].pos.x, cloth[0].pos.y,cloth[0].pos.z};
				cloth[0].velocity = { 0,0,0 };
			}
			else {
				cloth[i*col + j].pos = { cloth[0].pos.x + j*springLenght ,cloth[0].pos.y ,cloth[0].pos.z + i*springLenght };
				cloth[i*col + j].prePos = { cloth[0].pos.x + j*springLenght ,cloth[0].pos.y,cloth[0].pos.z + i*springLenght };
				cloth[0].velocity = { 0,0,0 };
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

glm::vec3 tempParticlePos;
void moveParticle(int index, float time) {
	//VERLET SOLVER
	tempParticlePos = cloth[index].pos;


	cloth[index].pos.x = cloth[index].pos.x + (cloth[index].pos.x - cloth[index].prePos.x);
	cloth[index].pos.y = cloth[index].pos.y + (cloth[index].pos.y - cloth[index].prePos.y) - gravity * (time * time);
	cloth[index].pos.z = cloth[index].pos.z + (cloth[index].pos.z - cloth[index].prePos.z);

	cloth[index].prePos = tempParticlePos;


	cloth[index].velocity.x = (cloth[index].pos.x + cloth[index].prePos.x) / time;
	cloth[index].velocity.y = (cloth[index].pos.y + cloth[index].prePos.y) / time;
	cloth[index].velocity.z = (cloth[index].pos.z + cloth[index].prePos.z) / time;
}

// COLLISIONS
void collidePlane(int index, int A, int B, int C, int d) {
	glm::vec3 normal = { A, B, C };
	float dotProdAct = glm::dot(normal, cloth[index].pos);
	float dotProdPrev = glm::dot(normal, cloth[index].prePos);
	float dotProdVelo = glm::dot(normal, cloth[index].velocity);
	float checkColl = (dotProdAct + d)*(dotProdPrev + d);

	if (checkColl <= 0) {
		cloth[index].pos = cloth[index].pos - 2 * (dotProdAct + d) * normal;
		cloth[index].velocity = cloth[index].velocity - 2 * dotProdVelo * normal;
	}
}

float a, b, c, resPos, resNeg, res, x, y, z; //variables for collide sphere
void collideSphere(int index) {
	a = (cloth[index].pos.x - cloth[index].prePos.x) * (cloth[index].pos.x - cloth[index].prePos.x) +
		(cloth[index].pos.y - cloth[index].prePos.y) * (cloth[index].pos.y - cloth[index].prePos.y) +
		(cloth[index].pos.z - cloth[index].prePos.z) * (cloth[index].pos.z - cloth[index].prePos.z);
	b = 2 * ((cloth[index].pos.x - cloth[index].prePos.x) * (cloth[index].prePos.x - spherePos.x) +
		(cloth[index].pos.y - cloth[index].prePos.y) * (cloth[index].prePos.y - spherePos.y) +
		(cloth[index].pos.z - cloth[index].prePos.z) * (cloth[index].prePos.z - spherePos.z));
	c = spherePos.x * spherePos.x + spherePos.y * spherePos.y + spherePos.z * spherePos.z + cloth[index].prePos.x *
		cloth[index].prePos.x + cloth[index].prePos.y * cloth[index].prePos.y + cloth[index].prePos.z *
		cloth[index].prePos.z - 2 * (spherePos.x * cloth[index].prePos.x + spherePos.y * cloth[index].prePos.y +
			spherePos.z * cloth[index].prePos.z) - sphereRadius;

	if (b * b - 4 * a * c >= 0) {
		glm::vec3 auxil, colis;
		resPos = (-b + glm::sqrt(b*b - 4 * a * c)) / (2 * a);
		resNeg = (-b - glm::sqrt(b*b - 4 * a * c)) / (2 * a);
		x = cloth[index].pos.x - cloth[index].prePos.x;
		y = cloth[index].pos.y - cloth[index].prePos.y;
		z = cloth[index].pos.z - cloth[index].prePos.z;
		glm::vec3 coli1 = { cloth[index].prePos.x + x * resPos, cloth[index].prePos.y + y * resPos, cloth[index].prePos.z + z * resPos };
		glm::vec3 coli2 = { cloth[index].prePos.x + x * resNeg, cloth[index].prePos.y + y * resNeg, cloth[index].prePos.z + z * resNeg };
		if (glm::distance(cloth[index].pos, coli1) <= glm::distance(cloth[index].pos, coli2)) {
			res = resPos;
			colis = coli1;
		}
		else {
			res = resNeg;
			colis = coli2;
		}

		glm::vec3 colisNormal = glm::normalize(colis - spherePos);
		float d = colisNormal.x * colis.x + colisNormal.y * colis.y + colisNormal.z * colis.z;
		d = -d;

		float actAux = glm::dot(colisNormal, cloth[index].pos);
		float prevAux = glm::dot(colisNormal, cloth[index].prePos);
		float dotProdSpeed = glm::dot(colisNormal, cloth[index].velocity);
		float checkCol = (actAux + d) * (prevAux + d);
		if (checkCol <= 0) {
			cloth[index].pos = cloth[index].pos - 2 * (actAux + d) * colisNormal;
			cloth[index].velocity = cloth[index].velocity - 2 * dotProdSpeed * colisNormal;
		}
	}
}

void boxCollision(int index) {
	collidePlane(index, 0, 1, 0, 0);//Ground
	collidePlane(index, 0, -1, 0, 10);//Top
	collidePlane(index, 1, 0, 0, 5);//Left Wall
	collidePlane(index, -1, 0, 0, 5);//Right Wall
	collidePlane(index, 0, 0, 1, 5);//Depht Wall
	collidePlane(index, 0, 0, -1, 5);//Front Wall
}

float ke = 0.5f;
float kb = 0.5f;
glm::vec3 neighbourSpringForce(int index1, int index2) { //retorna la for�a que rep la particula d'index 1 respecte la 2
	
	float modul = glm::distance(cloth[index1].pos, cloth[index2].pos);
	glm::vec3 velVec = cloth[index1].velocity - cloth[index2].velocity;
	glm::vec3 vecPos = cloth[index1].pos - cloth[index2].pos;
	glm::vec3 vecToDot = ke * (modul - springLenght) + kb * (velVec);
	glm::vec3 unitVec = vecPos / modul;
	float dotVec = glm::dot(vecToDot, unitVec);

	glm::vec3 force = -dotVec * unitVec;
	
	return force;
}

//PHYSICS MAIN FUNCTIONS
void PhysicsInit() {
	//TODO
	initializeCloth();
}
void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < 252; ++i) {
		moveParticle(i, dt);
		boxCollision(i);
		if(renderSphere) collideSphere(i);
		/*for (int j = 1; i < 252; ++i)
			neighbourSpringForce(i, j);*/
	}
	particleToFloatConverter();
	ClothMesh::updateClothMesh(vertArray);
	
	

}
void PhysicsCleanup() {
	//TODO
	delete[] cloth;
	delete[] vertArray;
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
