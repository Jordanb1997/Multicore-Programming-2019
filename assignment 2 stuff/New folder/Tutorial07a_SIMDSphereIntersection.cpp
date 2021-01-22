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

// test to see if collision between ray and any sphere happens before time t (equivalent to distance)
// updates closest collision time (/distance) if collision occurs
bool isSphereIntersected(const Sphere* s, const Ray* r, float* t)
{
	//TODO: declare a bool variable to store the return value

	//TODO: move the loop from main to here (and encompass most of the code below)

	// Intersection of a ray and a sphere, check the articles for the rationale
	float xdist = s->xpos - r->xstart;														//TODO: access current sphere position value from array
	float ydist = s->ypos - r->ystart;														//TODO: access current sphere position value from array
	float zdist = s->zpos - r->zstart;														//TODO: access current sphere position value from array
	float B = r->xdir * xdist + r->ydir * ydist + r->zdir * zdist;							
	float D = B * B - (xdist * xdist + ydist * ydist + zdist * zdist) + s->size * s->size;	//TODO: access current sphere size value from array

	// if D < 0, no intersection, so don't try and calculate the point of intersection
	if (D < 0.0f) return false;		//TODO: make this not exit the function early

	// if we've got this far an intersection between the ray and sphere has been found
	intersectionsFound++;

	// calculate both intersection times(/distances)
	float t0 = B - sqrtf(D);
	float t1 = B + sqrtf(D);

	// check to see if either of the two sphere collision points are closer than time parameter
	if ((t0 > EPSILON) && (t0 < *t))
	{
		*t = t0;
		//TODO: count that an improved intersection was found
		return true;				//TODO: make this not exit the function early
	}
	else if ((t1 > EPSILON) && (t1 < *t))
	{
		*t = t1;
		//TODO: count that an improved intersection was found
		return true;				//TODO: make this not exit the function early
	}

	return false; //TODO: return the stored bool
}

// read command line arguments, render, and write out BMP file
int main(int argc, char* argv[])
{
	float t = MAX_RAY_DISTANCE;
	Ray viewRay = { 100.0f, -100.0f, 100.0f, 0.003162246f, 0.999999f, -0.003162246f };
	Sphere sphereList[NUM_SPHERES];

	// initialise the spheres to some random locations and sizes
	for (unsigned int i = 0; i < NUM_SPHERES; ++i)
	{
		sphereList[i].xpos = (float) (rand() % 200);
		sphereList[i].ypos = (float) (rand() % 1000);
		sphereList[i].zpos = (float) (rand() % 200);
		sphereList[i].size = (float) (rand() % 100 + 10);
	}

	// search for sphere collisions, storing closest one found
	//TODO: replace this loop with a single call to the sphere intersection function (the version that checks them all)
	for (unsigned int i = 0; i < NUM_SPHERES; ++i)
	{
		if (isSphereIntersected(&sphereList[i], &viewRay, &t))
		{
			totalImprovedIntersections++;
		}
	}

	// output some stats for testing
	printf("Total intersections found:           %d\n", intersectionsFound);
	printf("Total improved intersections found:  %d\n", totalImprovedIntersections);
	printf("Closest intersection found:          %f\n", t);
}
