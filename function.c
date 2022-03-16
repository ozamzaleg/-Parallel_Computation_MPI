
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "function.h"

// return the array of pointes(contain double x and double y)
double *readFromFile(const char *fileName, int *numberOfPoints, int *param)
{
	FILE *fp;
	double *points;

	// Open file for reading points
	if ((fp = fopen(fileName, "r")) == 0)
	{
		printf("cannot open file %s for reading\n", fileName);
		exit(0);
	}

	// Param
	fscanf(fp, "%d", param);

	// Number of points
	fscanf(fp, "%d", numberOfPoints);

	// Allocate array of points end Read data from the file
	points = (double *)malloc(2 * *numberOfPoints * sizeof(double));
	if (points == NULL)
	{
		printf("Problem to allocate memory\n");
		exit(0);
	}
	for (int i = 0; i < *numberOfPoints; i++)
	{
		fscanf(fp, "%le %le", &points[2 * i], &points[2 * i + 1]);
	}

	fclose(fp);

	return points;
}

// return the sum
double heavy(double x, double y, int param) {
	double center[2] = { 0.4, 0.2 };
	int i, loop, size = 1, coeff = 10000;
	double sum = 0, dx, dy, radius = 0.2 * size;
	int longLoop = 1000, shortLoop = 1;
	double pi = 3.14;
	dx = (x - center[0]) * size;
	dy = (y - center[1]) * size;
	loop = (sqrt(dx * dx + dy * dy) < radius) ? longLoop : shortLoop;

	for (i = 1; i < loop * coeff; i++)
		sum += cos(2 * pi * dy * dx + 0.1) * sin(exp(10 * cos(pi * dx))) / i;

	return  sum;
}


double fmax(double answer, double sumPoint)
{
	return answer > sumPoint ? answer : sumPoint;
}
