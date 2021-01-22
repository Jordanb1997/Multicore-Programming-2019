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

// structure that stores SoA versus of a sphere list 
typedef struct Spheres
{
	__m256 xpos[NUM_SPHERES / 8], ypos[NUM_SPHERES / 8], zpos[NUM_SPHERES / 8];		// centre point
	__m256 size[NUM_SPHERES / 8];													// radius of sphere
} Spheres;

// test to see if collision between ray and any sphere happens before time t (equivalent to distance)
// updates closest collision time (/distance) if collision occurs
bool areSpheresIntersected(const Spheres* sphereList, const Ray* r, float* t)
{
	bool result = false;

	__m256 rayxstarts = _mm256_set1_ps(r->xstart);
	__m256 rayystarts = _mm256_set1_ps(r->ystart);
	__m256 rayzstarts = _mm256_set1_ps(r->zstart);
	__m256 rayxdirs = _mm256_set1_ps(r->xdir);
	__m256 rayydirs = _mm256_set1_ps(r->ydir);
	__m256 rayzdirs = _mm256_set1_ps(r->zdir);

	__m256 epsilons = _mm256_set1_ps(EPSILON);

	__m256 ts = _mm256_set1_ps(*t);

	// search for sphere collisions, storing closest one found
	for (unsigned int i = 0; i < NUM_SPHERES / 8; ++i)
	{
		// Intersection of a ray and a sphere, check the articles for the rationale
		__m256 xdists = _mm256_sub_ps(sphereList->xpos[i], rayxstarts);
		__m256 ydists = _mm256_sub_ps(sphereList->ypos[i], rayystarts);
		__m256 zdists = _mm256_sub_ps(sphereList->zpos[i], rayzstarts);
		__m256 Bs = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(rayxdirs, xdists), _mm256_mul_ps(rayydirs, ydists)), _mm256_mul_ps(rayzdirs, zdists));
		__m256 Ds = _mm256_add_ps(_mm256_sub_ps(_mm256_mul_ps(Bs, Bs),
			_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(xdists, xdists), _mm256_mul_ps(ydists, ydists)), _mm256_mul_ps(zdists, zdists))),
			_mm256_mul_ps(sphereList->size[i], sphereList->size[i]));

		__m256 sqrtDs = _mm256_sqrt_ps(Ds);
		__m256 t0s = _mm256_sub_ps(Bs, sqrtDs);
		__m256 t1s = _mm256_add_ps(Bs, sqrtDs);

		__m256 Dge0 = _mm256_cmp_ps(Ds, _mm256_setzero_ps(), _CMP_GE_OQ);

		__m256 t0gtEPSILON = _mm256_cmp_ps(t0s, epsilons, _CMP_GT_OQ);
		__m256 t1gtEPSILON = _mm256_cmp_ps(t1s, epsilons, _CMP_GT_OQ);

		//TODO: declare and initialise a SIMD variable to store whether (t0 < minimum intersect distance (*t originally))
		//TODO: declare and initialise a SIMD variable to store whether (t1 < minimum intersect distance (*t originally))

		//TODO: declare and initialise a SIMD variable to store whether (EPSILON < t0 < minimum intersect distance (*t originally))
		//TODO: declare and initialise a SIMD variable to store whether (EPSILON < t1 < minimum intersect distance (*t originally))

		for (unsigned int j = 0; j < 8; ++j)
		{
			float D = Ds.m256_f32[j];																// remove this

			// if D < 0, no intersection, so don't try and calculate the point of intersection
			if (_mm256_castps_si256(Dge0).m256i_i32[j])
			{
				// if we've got this far an intersection between the ray and sphere has been found
				intersectionsFound++;

				// calculate both intersection times(/distances)
				float t0 = t0s.m256_f32[j];															// remove this
				float t1 = t1s.m256_f32[j];															// remove this

				// check to see if either of the two sphere collision points are closer than time parameter
				if (_mm256_castps_si256(t0gtEPSILON).m256i_i32[j] && (t0 < ts.m256_f32[j]))			// replace this whole condition expression with new SIMD variable access
				{
					ts.m256_f32[j] = t0;															// access SIMD variable here (as we've removed t0)
					totalImprovedIntersections++;
					result = true;
				}
				else if (_mm256_castps_si256(t1gtEPSILON).m256i_i32[j] && (t1 < ts.m256_f32[j]))	// replace this whole condition expression with new SIMD variable access
				{
					ts.m256_f32[j] = t1;															// access SIMD variable here (as we've removed t1)
					totalImprovedIntersections++;
					result = true;
				}
			}
		}
	}

	for (unsigned int j = 0; j < 8; ++j)
	{
		if (ts.m256_f32[j] < *t) *t = ts.m256_f32[j];
	}

	return result;
}

// read command line arguments, render, and write out BMP file
int main(int argc, char* argv[])
{
	float t = MAX_RAY_DISTANCE;
	Ray viewRay = { 100.0f, -100.0f, 100.0f, 0.003162246f, 0.999999f, -0.003162246f };
	Spheres sphereList;

	// initialise the spheres to some random locations and sizes
	for (unsigned int i = 0; i < NUM_SPHERES; ++i)
	{
		sphereList.xpos[i / 8].m256_f32[i % 8] = (float)(rand() % 200);
		sphereList.ypos[i / 8].m256_f32[i % 8] = (float)(rand() % 1000);
		sphereList.zpos[i / 8].m256_f32[i % 8] = (float)(rand() % 200);
		sphereList.size[i / 8].m256_f32[i % 8] = (float)(rand() % 100 + 10);
	}

	// search for sphere collisions, storing closest one found
	areSpheresIntersected(&sphereList, &viewRay, &t);

	// output some stats for testing
	printf("Total intersections found:           %d\n", intersectionsFound);
	printf("Total improved intersections found:  %d\n", totalImprovedIntersections);
	printf("Closest intersection found:          %f\n", t);
}