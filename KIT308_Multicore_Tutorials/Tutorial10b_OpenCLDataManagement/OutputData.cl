__kernel void func() {
	unsigned int i = get_global_id(0); 

	if (i == 0)
	{
		printf("\n---------------\nGPU-side data\n");
		//print(root);
		printf("\n");

	}
}
