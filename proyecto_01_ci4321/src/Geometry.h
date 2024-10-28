#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <numbers>
#include "Shader.h"

using namespace std;

const double PI = numbers::pi;

class Geometry
{
public:
	unsigned int VBO, VAO;
	std::vector<float> attributes;
	glm::vec3 position;
	glm::vec3 rotation;

	virtual void SetupGL() = 0;
	virtual void CleanGL() = 0;
	virtual void Draw(const Shader& shader) = 0;

	inline void SetPosition(glm::vec3 newPos) 
	{
		position = newPos;

	};

	inline void SetRotation(glm::vec3 newRot)
	{
		rotation = newRot;

	};

};

class Sphere : public Geometry
{
public:
	unsigned int IBO;

	std::vector<unsigned int> indices;
	float radius;
	int sectorCount;
	int stackCount;

	Sphere(float radius = 1.0, int sectorCount = 36, int stackCount = 18, bool full = true);

	void SetupGL() override;
	void CleanGL() override;
	void Draw(const Shader& shader) override;
	void moveForward();
	void moveBackwards();
};

class Cube : public Geometry
{
public:

	float width;
	float height;
	float depth;

	Cube(float width = 1.0, float height = 1.0, float depth = 1.0);

	void SetupGL() override;
	void CleanGL() override;
	void Draw(const Shader& shader) override;
	void moveForward();
	void moveBackwards();
	void moveRight();
	void moveLeft();
};

class Cylinder : public Geometry
{
public:
	unsigned int IBO;

	std::vector<float> unitCircleVertices;
	std::vector<unsigned int> indices;
	
	float radius;
	float height;
	float sectorCount;	


	Cylinder(float radius = 1.0, float height = 1.0, int sectorCount = 36);

	void SetupGL() override;
	void CleanGL() override;
	void Draw(const Shader& shader) override;
	void DrawCanon(const Shader& shader);
	void moveForward();
	void moveBackwards();
};


#endif