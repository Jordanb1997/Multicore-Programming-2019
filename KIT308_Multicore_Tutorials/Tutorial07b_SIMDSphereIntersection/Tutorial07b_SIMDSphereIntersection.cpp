#include <cmath>
#include <stdio.h>
#include <immintrin.h>

// a small value (used to make sure we don't get stuck detecting collision of the same object over and over) 
const float EPSILON = 0.01f;

// maximum ray distance
const float MAX_RAY_DISTANCE = 2000000.0f; 

// rays are cast from a starting point in a direction
typedef struct Ray
{
	float xstart, ystart, zstart;
	float xdir, ydir, zdir;
} Ray;

// sphere object
typedef struct Sphere
{
	float xpos, ypos, zpos;		// centre point
	float size;					// radius of sphere
} Sphere;

// debug counts of number of intersections
int intersectionsFound = 0;
int totalImprovedIntersections = 0;

// number of spheres in the spheres list
const int NUM_SPHERES = 1024;

//: declare a struct to contain a replacement for the spheres list as a SIMD SoA structure (need to store xpos, ypos, zpos, and size)
typedef struct Spheres
{
	__m256 xpos[NUM_SPHERES / 8], ypos[NUM_SPHERES / 8], zpos[NUM_SPHERES / 8];		// centre point
	__m256 size[NUM_SPHERES / 8];													// radius of sphere
} Spheres;
// test to see if collision between ray and any sphere happens before time t (equivalent to distance)
// updates closest collision time (/distance) if collision occurs
bool areSpheresIntersected(const Spheres* sphereList, const Ray* r, float* t)	//: change first parameter to be SoA SIMD structure
{
	bool result = false;

	// search for sphere collisions, storing closest one found
	for (unsigned int i = 0; i < NUM_SPHERES/8; ++i)								//: change maximum to length of SIMD array
	{
		//: add loop across SIMD elements
		for (unsigned int j = 0; j < 8; ++j)
		{
			// Intersection of a ray and a sphere, check the articles for the rationale
			float xdist = sphereList->xpos[i].m256_f32[j] - r->xstart;
			float ydist = sphereList->ypos[i].m256_f32[j] - r->ystart;
			float zdist = sphereList->zpos[i].m256_f32[j] - r->zstart;
			float B = r->xdir * xdist + r->ydir * ydist + r->zdir * zdist;
			float D = B * B - (xdist * xdist + ydist * ydist + zdist * zdist) +
				sphereList->size[i].m256_f32[j] * sphereList->size[i].m256_f32[j];	//: update to use SoA structure

			// if D < 0, no intersection, so don't try and calculate the point of intersection
			if (D >= 0.0f)
			{
				// if we've got this far an intersection between the ray and sphere has been found
				intersectionsFound++;

				// calculate both intersection times(/distances)
				float t0 = B - sqrtf(D);
				float t1 = B + sqrtf(D);

				// check to see if either of the two sphere collision points are closer than time parameter
				if ((t0 > EPSILON) && (t0 < *t))
				{
					*t = t0;
					totalImprovedIntersections++;
					result = true;
				}
				else if ((t1 > EPSILON) && (t1 < *t))
				{
					*t = t1;
					totalImprovedIntersections++;
					result = true;
				}
			}
		}
	}	

	return result;
}

// read command line arguments, render, and write out BMP file
int main(int argc, char* argv[])
{
	float t = MAX_RAY_DISTANCE;
	Ray viewRay = { 100.0f, -100.0f, 100.0f, 0.003162246f, 0.999999f, -0.003162246f };
	Spheres sphereList;										//: use SoA SIMD structure

	// initialise the spheres to some random locations and sizes

	//: either:
	//  - change loop maximum to length of SIMD array and add another loop over the SIMD elements
    //or
	//  - change use of index to use i/8 for SIMD vectors, and i%8 for SIMD elements
	for (unsigned int i = 0; i < NUM_SPHERES; ++i)						
	{

		sphereList.xpos[i / 8].m256_f32[i % 8] = (float)(rand() % 200);						//: update to use SoA structure
		sphereList.ypos[i / 8].m256_f32[i % 8] = (float)(rand() % 1000);
		sphereList.zpos[i / 8].m256_f32[i % 8] = (float)(rand() % 200);
		sphereList.size[i / 8].m256_f32[i % 8] = (float)(rand() % 100 + 10);			//: update to use SoA structure
	}

	// search for sphere collisions, storing closest one found
	areSpheresIntersected(&sphereList, &viewRay, &t);					//: pass pointer to SoA SIMD sphereList

	// output some stats for testing
	printf("Total intersections found:           %d\n", intersectionsFound);
	printf("Total improved intersections found:  %d\n", totalImprovedIntersections);
	printf("Closest intersection found:          %f\n", t);
}
