

int main(int argc, char* argv[])
{
	float a[100];
	float b[100];
	float res;
	int i;

	for (i = 0; i < 100; i++)
	{
		a[i] = (float)(i + 1);
		b[i] = (float)(100 - i);
	}

	res = 0.0f;
	for (i = 0; i < 100; i++)
	{
		res += a[i] * b[i];
	}

	printf("Res %f\n", res);
	return 0;
}