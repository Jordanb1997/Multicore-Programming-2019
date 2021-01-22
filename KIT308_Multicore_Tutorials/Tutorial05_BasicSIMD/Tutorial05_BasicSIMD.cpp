// Tutorial05_BasicSIMD.cpp 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

void init()
{
	__m128i array[40 / 4];
	int value = 5;

	for (int i = 0; i < 40; i += 4)
	{
		for (int j = 0; j < 4; j++)
		{
			array[i / 4].m128i_i32[j] = value;
		}
	}

	printf("zero:\n%d %d %d %d %d %d %d %d\n\n", array[0], array[5], array[10], array[15], array[20], array[25], array[30], array[35]);
}

void branches1()
{
	__m128i array = _mm_setr_epi32(4, 7, -2, 9);
	__m128i value = _mm_set1_epi32(5);
	__m128i adjustment1 = _mm_set1_epi32(10);
	__m128i adjustment2 = _mm_set1_epi32(3);


	__m128i is_gt = _mm_cmpgt_epi32(array, value);
	__m128i rhs = _mm_or_si128(_mm_and_si128(is_gt, adjustment1), _mm_andnot_si128(is_gt, adjustment2));
	array = _mm_add_epi32(array, rhs);

	printf("branches1:\n%d %d %d %d\n\n", array.m128i_i32[0], array.m128i_i32[1], array.m128i_i32[2], array.m128i_i32[3]);
}

void branches2()
{
	int array[8] = { 4, -2, 9, 7, 3, 2, 4, 6 };
	int value = 5, adjustment = 3;

	for (int i = 0; i < 8; i++)
	{
		if (array[i] <= value)
			array[i] += adjustment;
	}

	printf("branches2:\n%d %d %d %d %d %d %d %d\n\n", array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]);
}

void branches3()
{
	float a[8] = { 1, 2, 7, -3, 4, 2, 6, -2 };
	float b[8] = { 5, 1, 3, -3, 5, 2, 5, 1 };

	for (int i = 0; i < 8; i++)
	{
		if (a[i] <= b[i])
		{
			a[i] = b[i];
			b[i] = 0;
		}
	}

	printf("branches3:\n%f %f %f %f %f %f %f %f\n", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
	printf("%f %f %f %f %f %f %f %f\n\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
}

void branches4()
{
	float a[8] = { 1, 2, 7, -3, 4, 2, 6, -2 };
	float b[8] = { 5, 1, 3, -3, 5, 2, 5, 1 };
	float c[8] = { 9, 8, 7, 6, 5, 4, 3, 2 };
	float e[8];

	for (int i = 0; i < 8; i++)
		if (a[i] <= b[i])
			if (a[i] == b[i])
				e[i] = a[i] * b[i] - c[i];
			else
				e[i] = c[i] + a[i];
		else
			e[i] = b[i];

	printf("branches4:\n%f %f %f %f %f %f %f %f\n\n", e[0], e[1], e[2], e[3], e[4], e[5], e[6], e[7]);
}

void branches5()
{
	float a[8] = { 1, 2, 7, -3, 4, 2, 6, -2 };
	float b[8] = { 5, 1, 3, -3, 5, 2, 5, 1 };
	float c[8] = { 9, 8, 7, 6, 5, 4, 3, 2 };

	for (int i = 0; i < 8; i++)
	{
		if (a[i] == b[i]) continue;

		if (a[i] < c[i])
		{
			b[i] = a[i] + c[i];
		}
		else if (b[i] < c[i])
		{
			a[i] = a[i] * 0.1f;
			b[i] = a[i];
		}
	}

	printf("branches5:\n%f %f %f %f %f %f %f %f\n", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
	printf("%f %f %f %f %f %f %f %f\n\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
}

void horiz()
{
	float sum = 0;
	float a[80] = {
		1, 6, 5, 4, 3, 9, 8, 4,
		2.1f, 4.6f, 2.1f, 5.4f, 5.66f, 7.3f, 1.2f, 1.6f,
		1, 2, 7, -3, 4, 2, 6, -2,
		5, 1, 3, -3, 5, 2, 5, 1,
		9, 8, 7, 6, 5, 4, 3, 2,
		100, 100, 100, 100, 50, 50, 50, 50,
		4, 3, 2, 1, 0, -1, -2, -3,
		9, 6, 1, 9, 4, 3, 2, 1 
	};

	for (int i = 0; i < 64; i++)
	{
		sum += a[i];
	}

	printf("horiz:\n%f\n\n", sum);
}

int main(int argc, char **argv)
{
	init();

	branches1();
	branches2();
	branches3();
	branches4();
	branches5();

	horiz();

	// success!
	return 0;
}

