/*  The following code is a VERY heavily modified from code originally sourced from:
	Ray tracing tutorial of http://www.codermind.com/articles/Raytracer-in-C++-Introduction-What-is-ray-tracing.html
	It is free to use for educational purpose and cannot be redistributed outside of the tutorial pages. */

#ifndef __SCENE_H
#define __SCENE_H

#include "SceneObjects.h"
#include <immintrin.h>

// description of a single static scene
typedef struct Scene 
{
	Point cameraPosition;					// camera location
	float cameraRotation;					// direction camera points
    float cameraFieldOfView;				// field of view for the camera

	float exposure;							// image exposure

	unsigned int skyboxMaterialId;

	// scene object counts
	unsigned int numMaterials;
	unsigned int numSpheres;
	unsigned int numTriangles;
	unsigned int numLights;

	// scene objects
	Material* materialContainer;	
	Sphere* sphereContainer;
	Triangle* triangleContainer;
	Light* lightContainer;

	// SIMD spheres
	unsigned int numSpheresSIMD;
	__m256* spherePosX;
	__m256* spherePosY;
	__m256* spherePosZ;
	__m256* sphereSize;
	__m256i* sphereMaterialId;

	//SIMD triangles hold info for three points and the vector normal, material
	unsigned int numTrianglesSIMD;
	__m256* triangle1X, *triangle1Y, *triangle1Z;
	__m256* triangle2X, *triangle2Y, *triangle2Z;
	__m256* triangle3X, *triangle3Y, *triangle3Z;
	__m256* triangleNormalX, *triangleNormalY, *triangleNormalZ;
	__m256i* triangleMaterialId;

	//SIMD lighting
	unsigned int numLightsSIMD;
	__m256* posX, *posY, *posZ;
	__m256* red, *green, *blue;

} Scene;

bool init(const char* inputName, Scene& scene);

#endif // __SCENE_H