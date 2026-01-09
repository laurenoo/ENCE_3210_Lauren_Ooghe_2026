
#include <stdio.h>
#include <math.h>

struct s_value {
	float x1;
	float x2;

};


struct s_value quadratic(int a, int b, int c) {

	struct s_value x;

	float s = sqrt(b * b - 4 * a * c);
	if (s >= 0)
	{
		x.x1 = ( -b + s) / (2 * a);
		x.x2 = ( -b - s) / (2 * a);

	}
	else
	{
		x.x1 = 0;
		x.x2 = 0;
		printf("Complex roots\n");
	}
	return x;
}

int main()
{
	struct s_value y;
	int a, b, c;
	scanf("%d %d %d", &a, &b, &c);
	y = quadratic(a, b, c);
	printf("Results: %.2f, %.2f", y.x1, y.x2);
}