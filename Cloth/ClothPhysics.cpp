#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <math.h>

//Boolean variables allow to show/hide the primitives

bool renderCloth = true;

glm::vec3 gravity = { 0, -9.8f, 0 };

namespace ClothMesh {
	extern void updateClothMesh(float* array_data);
}

int col = 14, row = 18;
int clothLength = col*row; // 14*18=252

class Particle {
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
	cloth[0].pos = { -(13 * springLength / 2),7,-(17 * springLength / 2) };
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
float keStruc = 100;//500-1000
float kdStruc = 7;//30-70
float keShear = 100.f;//500-1000
float kdShear = 7.f;//30-70
float keBend = 100.f;//500-1000
float kdBend = 7.f;//30-70
glm::vec3 neighbourSpringForce(int index1, int index2, float ke, float kd, float L) { //retorna la força que rep la particula d'index 1 respecte la 2

	float modul = glm::distance(cloth[index1].pos, cloth[index2].pos);
	glm::vec3 vecUnitari = glm::normalize(cloth[index1].pos - cloth[index2].pos);
	float dampingTerm = glm::dot(kd * (cloth[index1].velocity - cloth[index2].velocity), vecUnitari);
	float primerTerme = ke * (modul - L) + dampingTerm;
	glm::vec3 force = -primerTerme*vecUnitari;

	return force;
}

//STRUCTURAL
glm::vec3 horizontalTempForce, verticalTempForce;
void addStructuralForces() {

	for (int i = 0; i < clothLength; ++i) {
		/*if (i % 14 != 13) {
		horizontalTempForce = neighbourSpringForce(i, i + 1, keStruc, kdStruc, springLength);
		cloth[i].totalForce += horizontalTempForce;
		cloth[i + 1].totalForce -= horizontalTempForce;
		}*/
		if (i < 238) {
			verticalTempForce = neighbourSpringForce(i, i + 14, keStruc, kdStruc, springLength);
			cloth[i].totalForce += verticalTempForce;
			cloth[i + 14].totalForce -= verticalTempForce;
		}
	}
}

//SHEAR
//glm::vec3 diagonalTempForce1, diagonalTempForce2; //1: top-left to bot-right. 2: bot-left to top-right
//void addShearForces() {
//	for (int i = 0; i < 237; ++i) {
//		if (i % 14 != 13) {
//			//calcular forces
//			diagonalTempForce1 = neighbourSpringForce(i, i + 15, keShear, kdShear, diagonalSpringLength);
//			diagonalTempForce2 = neighbourSpringForce(i + 14, i + 1, keShear, kdShear, diagonalSpringLength);
//			//sumar forces
//			cloth[i].totalForce += diagonalTempForce1;
//			cloth[i + 15].totalForce -= diagonalTempForce1;
//			cloth[i + 14].totalForce += diagonalTempForce2;
//			cloth[i + 1].totalForce -= diagonalTempForce2;
//		}
//	}
//}

//BENDING
//glm::vec3 horizontalDoubleSpring, verticalDoubleSpring;
//void addBendingForces() {
//	for (int i = 0; i < clothLength; ++i) {
//		if (i % 14 != 12 && i % 14 != 13) {
//			horizontalDoubleSpring = neighbourSpringForce(i, i + 2, keBend, kdBend, springLength * 2);
//			cloth[i].totalForce += horizontalDoubleSpring;
//			cloth[i + 2].totalForce -= horizontalDoubleSpring;
//		}
//	}
//	for (int i = 0; i < clothLength; ++i) {
//		if (i < 224) {
//			verticalDoubleSpring = neighbourSpringForce(i, i + 14 * 2, keBend, kdBend, springLength * 2);
//			cloth[i].totalForce += verticalDoubleSpring;
//			cloth[i + 14 * 2].totalForce -= verticalDoubleSpring;
//		}
//	}
//}

//	MOVEMENT
glm::vec3 tempParticlePos;
void moveParticle(float time) {
	//VERLET SOLVER
	for (int i = 0; i < clothLength; ++i) {
		tempParticlePos = cloth[i].pos;

		cloth[i].pos = cloth[i].pos + (cloth[i].pos - cloth[i].prePos) + cloth[i].totalForce * (time*time);

		cloth[i].prePos = tempParticlePos;

		cloth[i].velocity = (cloth[i].pos - cloth[i].prePos) / time;
	}
	cloth[0].pos = cloth[0].prePos;
	cloth[13].pos = cloth[13].prePos;
}


//PHYSICS MAIN FUNCTIONS
void PhysicsInit() {
	initializeCloth();
}

void PhysicsUpdate(float dt) {

	for (int i = 0; i < 10; ++i) {
		//calcular forces
		addStructuralForces();
		//addShearForces();
		//addBendingForces();
		moveParticle(dt / 10);
	}

	//reiniciar forces
	for (int i = 0; i < clothLength; ++i) {
		cloth[i].totalForce = gravity;
	}

	particleToFloatConverter();
	ClothMesh::updateClothMesh(vertArray);


}

void PhysicsCleanup() {
	//TODO
	delete[] cloth;
	delete[] vertArray;
}
