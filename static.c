#include <stdio.h>
#include <stdlib.h>
#include "function.h"

enum
{
    chunk = 10
};

int main(int argc, char *argv[])
{
    int splitSize = chunk * 2, indexMaster = 0;
    double x, y, answer;
    double *pointsFromFile = NULL;
    int myRank, numProcsses, numberOfPoints, param, finishTask = 0;
    double slaveData[splitSize];
    double *getDataFromSlave = NULL;
    double t1;
    int numberOfPointArr;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcsses);

    /////////////////////////////// Work //////////////////////////////////////////////////

    // master read the pointes from file
    if (myRank == MASTER)
    {
        // we got 100 pointes in array that contain 200 elementes
        pointsFromFile = readFromFile(FILE_NAME, &numberOfPoints, &param);
        if (!pointsFromFile)
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
      numberOfPointArr = numberOfPoints * 2;
        // we get from seleves the sub array 10 element because the chunk is 10
        getDataFromSlave = (double *)malloc((numberOfPoints) * sizeof(double));
        if (!getDataFromSlave)
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        t1 = MPI_Wtime();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // to update all the process include the master the number of Points and parms
    MPI_Bcast(&numberOfPointArr, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&param, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    // we stay in the while until we dont have point to split to slave (task finish!)
    while (finishTask + (splitSize * numProcsses) <= numberOfPointArr)
    {

        MPI_Scatter(pointsFromFile + finishTask, splitSize, MPI_DOUBLE, slaveData, splitSize, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

        x = slaveData[0];
        y = slaveData[1];

        answer = heavy(x, y, param);

        for (int i = 2; i < splitSize; i += 2)
        {
            x = slaveData[i];
            y = slaveData[i + 1];
            answer = fmax(answer, heavy(x, y, param));
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////

        MPI_Gather(&answer, 1, MPI_DOUBLE, getDataFromSlave + numProcsses * indexMaster, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
        indexMaster++;
        finishTask += (splitSize * numProcsses);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    if (myRank == MASTER)
    {
        double maxFromSlave = getDataFromSlave[0];
        if (numberOfPointArr - finishTask > 0)
        {
            for (int i = finishTask; i < numberOfPointArr; i += 2)
            {
                x = slaveData[i];
                y = slaveData[i + 1];
                maxFromSlave = fmax(maxFromSlave, heavy(x, y, param));
            }
        }
        for (int i = 1; i < numberOfPointArr / splitSize; i++)
        {
            maxFromSlave = fmax(maxFromSlave, getDataFromSlave[i]);
        }
        printf("Time: %e\n", MPI_Wtime() - t1);
        printf("Answer: %e\n", maxFromSlave);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
