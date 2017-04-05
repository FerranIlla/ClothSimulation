#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <math.h>

//Boolean variables allow to show/hide the primitives
bool renderSphere = true;
bool renderCapsule = false;
bool renderParticles = false;
bool renderCloth = true;
bool show_test_window = false;

glm::vec3 spherePos(0.f, 3.f, 0.f);
float sphereRadius = 1.f;
glm::vec3 gravity = { 0, -9.8f, 0 };

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
float springLength = 0.3f; //max = 0.5
float diagonalSpringLength = sqrt(pow(springLength, 2) + pow(springLength, 2));

void initializeCloth() {
	cloth[0].pos = { -(13*springLength/2),7,-(17 * springLength/2) };
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			cloth[i*col + j].pos = { cloth[0].pos.x + j*springLength ,cloth[0].pos.y ,cloth[0].pos.z + i*springLength };
			cloth[i*col + j].prePos = cloth[i*col + j].pos;
			cloth[i*col + j].velocity = { 0,0,0 };
			cloth[i*col + j].totalForce = gravity;
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

// FORCES
float keStruc = 1000.f;//500-1000
float kbStruc = 50.f;//30-70
float keShear = 1000.f;//500-1000
float kbShear = 50.f;//30-70
float keBend = 1000.f;//500-1000
float kbBend = 50.f;//30-70
glm::vec3 neighbourSpringForce(int index1, int index2, float ke, float kb, float L) { //retorna la força que rep la particula d'index 1 respecte la 2
	//passar distancia per parametre? probablement
	float modul = glm::distance(cloth[index1].pos, cloth[index2].pos);
	glm::vec3 vecToDot = ke * (modul - L) + kb * (cloth[index1].velocity - cloth[index2].velocity);
	float dotVec = glm::dot(vecToDot, (cloth[index1].pos - cloth[index2].pos) / modul);

	glm::vec3 force = -dotVec * (cloth[index1].pos - cloth[index2].pos) / modul;

	return force;
}

void addStructForces() {
	//tindrà una forma semblant a aixo
	//cloth[0].totalForce += neighbourSpringForce(...);
}
void addShearForces() {

}
void addBendingForces() {
	for (int i = 2; i < row - 2; i++) {
		for (int j = 2; j < col - 2; j++) {
			cloth[i*col + j].totalForce += neighbourSpringForce(i*col + j, i*col + j, keBend, keStruc, springLength * 2);
		}
	}
}

//	MOVEMENT
glm::vec3 tempParticlePos;
void moveParticle(float time) {
	//VERLET SOLVER
	for (int i = 0; i < clothLength; ++i) {
		tempParticlePos = cloth[i].pos;

		cloth[i].pos = cloth[i].pos + (cloth[i].pos - cloth[i].prePos) + cloth[i].totalForce * (time*time);

		cloth[i].prePos = tempParticlePos;

		cloth[i].velocity = (cloth[i].pos + cloth[i].prePos) / time;
	}
	cloth[0].pos = cloth[0].prePos;
	cloth[13].pos = cloth[13].prePos;
}


// COLLISIONS
void collidePlane(int index, int A, int B, int C, int d) {
	glm::vec3 normal = { A, B, C };
	float dotProdAct = glm::dot(normal, cloth[index].pos);
	float dotProdPrev = glm::dot(normal, cloth[index].prePos);
	float dotProdVelo = glm::dot(normal, cloth[index].velocity);
	float checkColl = (dotProdAct + d)*(dotProdPrev + d);

	if (checkColl <= 0) {
		cloth[index].pos = cloth[index].pos - 1.5f * (dotProdAct + d) * normal;
		cloth[index].velocity = cloth[index].velocity - 1.5f * dotProdVelo * normal;
	}
}

float a, b, c, resPos, resNeg, res, x, y, z; //variables for collide sphere
void collideSphere() {
	for (int i = 0; i < clothLength; ++i) {
		a = (cloth[i].pos.x - cloth[i].prePos.x) * (cloth[i].pos.x - cloth[i].prePos.x) +
			(cloth[i].pos.y - cloth[i].prePos.y) * (cloth[i].pos.y - cloth[i].prePos.y) +
			(cloth[i].pos.z - cloth[i].prePos.z) * (cloth[i].pos.z - cloth[i].prePos.z);
		b = 2 * ((cloth[i].pos.x - cloth[i].prePos.x) * (cloth[i].prePos.x - spherePos.x) +
			(cloth[i].pos.y - cloth[i].prePos.y) * (cloth[i].prePos.y - spherePos.y) +
			(cloth[i].pos.z - cloth[i].prePos.z) * (cloth[i].prePos.z - spherePos.z));
		c = spherePos.x * spherePos.x + spherePos.y * spherePos.y + spherePos.z * spherePos.z + cloth[i].prePos.x *
			cloth[i].prePos.x + cloth[i].prePos.y * cloth[i].prePos.y + cloth[i].prePos.z *
			cloth[i].prePos.z - 2 * (spherePos.x * cloth[i].prePos.x + spherePos.y * cloth[i].prePos.y +
				spherePos.z * cloth[i].prePos.z) - sphereRadius;

		if (b * b - 4 * a * c >= 0) {
			glm::vec3 auxil, colis;
			resPos = (-b + glm::sqrt(b*b - 4 * a * c)) / (2 * a);
			resNeg = (-b - glm::sqrt(b*b - 4 * a * c)) / (2 * a);
			x = cloth[i].pos.x - cloth[i].prePos.x;
			y = cloth[i].pos.y - cloth[i].prePos.y;
			z = cloth[i].pos.z - cloth[i].prePos.z;
			glm::vec3 coli1 = { cloth[i].prePos.x + x * resPos, cloth[i].prePos.y + y * resPos, cloth[i].prePos.z + z * resPos };
			glm::vec3 coli2 = { cloth[i].prePos.x + x * resNeg, cloth[i].prePos.y + y * resNeg, cloth[i].prePos.z + z * resNeg };
			if (glm::distance(cloth[i].pos, coli1) <= glm::distance(cloth[i].pos, coli2)) {
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

			float actAux = glm::dot(colisNormal, cloth[i].pos);
			float prevAux = glm::dot(colisNormal, cloth[i].prePos);
			float dotProdSpeed = glm::dot(colisNormal, cloth[i].velocity);
			float checkCol = (actAux + d) * (prevAux + d);
			if (checkCol <= 0) {
				cloth[i].pos = cloth[i].pos - 2 * (actAux + d) * colisNormal;
				cloth[i].velocity = cloth[i].velocity - 2 * dotProdSpeed * colisNormal;
			}
		}
	}
}

void boxCollision() {
	for (int i = 0; i < clothLength; ++i) {
		collidePlane(i, 0, 1, 0, 0);//Ground
		collidePlane(i, 0, -1, 0, 10);//Top
		collidePlane(i, 1, 0, 0, 5);//Left Wall
		collidePlane(i, -1, 0, 0, 5);//Right Wall
		collidePlane(i, 0, 0, 1, 5);//Depht Wall
		collidePlane(i, 0, 0, -1, 5);//Front Wall
	}
}



//PHYSICS MAIN FUNCTIONS
void PhysicsInit() {
	initializeCloth();
}
void PhysicsUpdate(float dt) {
	//calcular forces
	moveParticle(dt);
	boxCollision();
	if (renderSphere) collideSphere();
	//reiniciar forces
	
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
