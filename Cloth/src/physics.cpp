#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <time.h>

//Boolean variables allow to show/hide the primitives
bool renderSphere = true;
bool renderCloth = true;
bool show_test_window = true;

float sphereRadius = rand() % 3 + 0.5f;
glm::vec3 spherePos(rand() % 7 - 3, rand() % 7 + 1 - sphereRadius, rand() % 7 - 3);
glm::vec3 gravity = { 0, -9.8f, 0 };

namespace Sphere {
	extern void updateSphere(glm::vec3 pos = spherePos, float radius = sphereRadius);
}

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
float springLength = 0.0f; 
float nextSpringLength = 0.5f;
float diagonalSpringLength;

void initializeCloth() {
	springLength = nextSpringLength;
	diagonalSpringLength = sqrt(pow(springLength, 2) + pow(springLength, 2));
	cloth[0].pos = { -(13 * springLength / 2),8,-(17 * springLength / 2) };
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
float keStruc = 600.f;
float kdStruc = 25.f;
float keShear = 600.f;
float kdShear = 25.f;
float keBend = 600.f;
float kdBend = 25.f;
glm::vec3 neighbourSpringForce(int index1, int index2, float ke, float kd, float L) { //retorna la força que rep la particula d'index 1 respecte la 2

	float modul = glm::distance(cloth[index1].pos, cloth[index2].pos);
	glm::vec3 vecUnitari = glm::normalize(cloth[index1].pos - cloth[index2].pos);
	float dampingTerm = glm::dot(kd * (cloth[index1].velocity - cloth[index2].velocity), vecUnitari);
	float primerTerme = ke * (modul - L) + dampingTerm;
	glm::vec3 force = -primerTerme*vecUnitari;

	return force;
}

glm::vec3 horizontalTempForce, verticalTempForce;
void addStructuralForces() {

	for (int i = 0; i < clothLength; ++i) {
		if (i % 14 != 13) {
			horizontalTempForce = neighbourSpringForce(i, i + 1, keStruc, kdStruc, springLength);
			cloth[i].totalForce += horizontalTempForce;
			cloth[i + 1].totalForce -= horizontalTempForce;
		}
		if (i < 238) {
			verticalTempForce = neighbourSpringForce(i, i + 14, keStruc, kdStruc, springLength);
			cloth[i].totalForce += verticalTempForce;
			cloth[i + 14].totalForce -= verticalTempForce;
		}
	}
}

glm::vec3 diagonalTempForce1, diagonalTempForce2; 
void addShearForces() {
	for (int i = 0; i < 237; ++i) {
		if (i % 14 != 13) {
			//calcular forces
			diagonalTempForce1 = neighbourSpringForce(i, i + 15, keShear, kdShear, diagonalSpringLength);
			diagonalTempForce2 = neighbourSpringForce(i + 14, i + 1, keShear, kdShear, diagonalSpringLength);
			//sumar forces
			cloth[i].totalForce += diagonalTempForce1;
			cloth[i + 15].totalForce -= diagonalTempForce1;
			cloth[i + 14].totalForce += diagonalTempForce2;
			cloth[i + 1].totalForce -= diagonalTempForce2;
		}
	}
}

glm::vec3 horizontalDoubleSpring, verticalDoubleSpring;
void addBendingForces() {
	for (int i = 0; i < clothLength; ++i) {
		if (i % 14 != 12 && i % 14 != 13) {
			horizontalDoubleSpring = neighbourSpringForce(i, i + 2, keBend, kdBend, springLength * 2);
			cloth[i].totalForce += horizontalDoubleSpring;
			cloth[i + 2].totalForce -= horizontalDoubleSpring;
		}
	}
	for (int i = 0; i < clothLength; ++i) {
		if (i < 224) {
			verticalDoubleSpring = neighbourSpringForce(i, i + 14 * 2, keBend, kdBend, springLength * 2);
			cloth[i].totalForce += verticalDoubleSpring;
			cloth[i + 14 * 2].totalForce -= verticalDoubleSpring;
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

		cloth[i].velocity = (cloth[i].pos - cloth[i].prePos) / time;
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

void offsetPos(int index, glm::vec3 v) { cloth[index].pos += v; }

void ballCollision() {
	for (int i = 0; i < clothLength; ++i) {
		glm::vec3 v = cloth[i].pos - spherePos;
		float l = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (l < sphereRadius)
		{
			offsetPos(i, glm::vec3(v.x / l, v.y / l, v.z / l)*(sphereRadius - l));
		}
	}
}

float a, b, c, resPos, resNeg, res, x, y, z; 
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
	
	ballCollision();
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

//Max Ellingation
void maxEllongationReposition(int numberOfLoops, int percentage) {
	glm::vec3 v; //vector between neighbour vertex
	float d; //distance between neighbour vertex
	float allowedEllongation = springLength + springLength*percentage / 100;
	for (int k = 0; k < numberOfLoops; ++k) {
		for (int i = 0; i < clothLength; ++i) {
				if (i % 14 != 13) { //horizontals
					d = glm::distance(cloth[i].pos, cloth[i + 1].pos);
					if (d > allowedEllongation) {
						v = glm::normalize(cloth[i + 1].pos - cloth[i].pos);
						if (i == 0) cloth[i + 1].pos -= v*(d - allowedEllongation);
						else if (i == 12) cloth[i].pos += v*(d - allowedEllongation);
						else {
							cloth[i].pos += v*((d - allowedEllongation) / 2);
							cloth[i + 1].pos -= v*((d - allowedEllongation) / 2);
						}
					}
				}
				if (i < 238) { //verticals
					d = glm::distance(cloth[i].pos, cloth[i + 14].pos);
					if (d > allowedEllongation) {
						v = glm::normalize(cloth[i + 14].pos - cloth[i].pos);
						if (i == 0) cloth[i + 14].pos -= v*(d - allowedEllongation);
						else if(i == 13) cloth[i+14].pos -= v*(d - allowedEllongation);
						else {
							cloth[i].pos += v*((d - allowedEllongation) / 2);
							cloth[i + 14].pos -= v*((d - allowedEllongation) / 2);
						}
					}
				}
		}
	}
}

//PHYSICS MAIN FUNCTIONS
void PhysicsInit() {
	initializeCloth();
	srand(time(NULL));
}

float seconds = 0.0f;
int secondsUntilRestart = 20;
int maxEllongation = 100;
void PhysicsUpdate(float dt) {

	if (seconds >= secondsUntilRestart) {
		initializeCloth();
		seconds = 0;
		sphereRadius = rand() % 3 + 0.5f;
		spherePos = { rand() % 7 - 3, rand() % 7 + 1 - sphereRadius,rand() % 7 - 3 };
		Sphere::updateSphere();
	}
	for (int i = 0; i < 10; ++i) {
	
		addStructuralForces();
		addShearForces();
		addBendingForces();
		
		moveParticle(dt/10);

		maxEllongationReposition(5, maxEllongation);

		boxCollision();
		if (renderSphere) collideSphere();

		for (int i = 0; i < clothLength; ++i) {
			cloth[i].totalForce = gravity;
		}

		particleToFloatConverter();
		ClothMesh::updateClothMesh(vertArray);
	}	
	seconds += dt;
}

void PhysicsCleanup() {
	delete[] cloth;
	delete[] vertArray;
}

void GUI() {
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Separator();
		ImGui::Text("Parametrizable seconds");
		ImGui::Separator();
		ImGui::Text("Seconds %.1f", seconds);
		ImGui::SliderInt("Seconds until restart", &secondsUntilRestart, 1, 30);
		ImGui::Separator();
		ImGui::Text("Elasticity and Damping coeficients");
		ImGui::Separator();
		ImGui::DragFloat("Ke Structural", &keStruc, 1.f, 1, 1000,"%.0f");
		ImGui::DragFloat("Kd Structural", &kdStruc, 1.f, 1, 50, "%.0f");
		ImGui::DragFloat("Ke Shear", &keShear, 1.f, 1, 1000, "%.0f");
		ImGui::DragFloat("kd Shear", &kdShear, 1.f, 1, 50, "%.0f");
		ImGui::DragFloat("Ke Bending", &keBend, 1.f, 1, 1000, "%.0f");
		ImGui::DragFloat("kd Bending", &kdBend, 1.f, 1, 50, "%.0f");
		ImGui::Separator();
		ImGui::Text("Links properties");
		ImGui::Separator();
		ImGui::SliderFloat("Link Length", &nextSpringLength,0.1f,0.5f,"%.2f");
		ImGui::SliderInt("Max Ellingation", &maxEllongation,0,200,"%.0f%");


		
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}
