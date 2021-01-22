//: make appropriate datastructures
typedef struct Pair
{
	unsigned short littleNum;
	int bigNum;
}Pair;

typedef struct InnerData
{
	char c;
	int x;
	__global Pair* pairs;
	char d;
}InnerData;

typedef struct ComplexData
{
	__global int* values;
	__global InnerData* inner;
}ComplexData;

//: add an appropriate set of parameters to transfer the data
__kernel void func(__global ComplexData *data, __global int *v, __global InnerData *inner, __global Pair *p0, __global Pair *p1, __global Pair *p2, __global Pair *p3) {
	unsigned int i = get_global_id(0); 

	//TODO: relink the pointers
	data->values = v;
	data->inner = inner;
	data->inner[0].pairs = p0;
	data->inner[1].pairs = p1;
	data->inner[2].pairs = p2;
	data->inner[3].pairs = p3;

	if (i == 0)
	{
		printf("\n---------------\nGPU-side data\n");

		//: print out the data
		int sum = 0;
		for (int i = 0; i < 100; i++) sum += data->values[i];
		printf("%d, %d, ... %d, %d :: %d\n", data->values[0], data->values[1], data->values[98], data->values[99], sum);
		for (int i = 0; i < 4; i++)
		{
			printf("\ninner %d: %c, %d, %c\n", i, data->inner[i].c, data->inner[i].x, data->inner[i].d);

			sum = 0;
			for (int j = 0; j < 50; j++)
			{
				sum += data->inner[i].pairs[j].littleNum;
			}
			printf("%d, %d, ... %d, %d :: %d\n", data->inner[i].pairs[0].littleNum, data->inner[i].pairs[1].littleNum,
				data->inner[i].pairs[48].littleNum, data->inner[i].pairs[49].littleNum, sum);

			sum = 0;
			for (int j = 0; j < 50; j++)
			{
				sum += data->inner[i].pairs[j].bigNum;
			}
			printf("%d, %d, ... %d, %d :: %d\n", data->inner[i].pairs[0].bigNum, data->inner[i].pairs[1].bigNum,
				data->inner[i].pairs[48].bigNum, data->inner[i].pairs[49].bigNum, sum);

		}
	}
}