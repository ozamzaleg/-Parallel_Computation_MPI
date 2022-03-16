#ifndef __Functions_H__
#define __Functions_H__

#include <mpi.h>

#define FILE_NAME "points.txt"

enum ranks { MASTER };


double* readFromFile(const char* fileName, int* numberOfPoints, int* param);
double heavy(double x, double y, int param);
double fmax(double answer,double sumPoint);


#endif
