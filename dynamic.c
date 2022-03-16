#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "function.h"

enum tags
{
    WORK_TAG,
    STOP_TAG
};
enum
{
    TWO_PROSES = 2
};

int main(int argc, char *argv[])
{
    int myRank, numProcses;
    int chunk = argc >= 2 ? atoi(argv[1]) : 2;
    int splitSize = chunk * 2;
    int param;
    int finishTask = 0;
    int numberOfPointArr;
    double t1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcses);

    if (numProcses < TWO_PROSES)
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

    if (myRank == MASTER)
    {
        double *points;
        double *answerFromSlave;
        int numberOfPoints, tag, extra = 0;
        MPI_Status status;
        double maxFromSlave;
        int offset = 0;

        points = readFromFile(FILE_NAME, &numberOfPoints, &param);
        numberOfPointArr = numberOfPoints * 2;
        answerFromSlave = (double *)malloc(sizeof(double) * numberOfPointArr);
        if (!points)
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        t1 = MPI_Wtime();

        do
        {
            if (numberOfPointArr <= finishTask + (numProcses - 1) * splitSize)
            {
                tag = STOP_TAG;
                splitSize = (numberOfPointArr - finishTask) / (numProcses - 1);
                extra = (numberOfPointArr - finishTask) % (numProcses - 1);
            }
            else
                tag = WORK_TAG;

            for (int i = 1; i < numProcses; i++)
            {

                MPI_Send(&splitSize, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                MPI_Send(points + finishTask + (i - 1) * splitSize, splitSize, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
            }

            for (int i = 1; i < numProcses; i++)
            {
                MPI_Recv(answerFromSlave + offset, 1, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);
                offset++;
            }
            finishTask += (numProcses - 1) * splitSize;

        } while (tag != STOP_TAG && finishTask < numberOfPointArr);

        maxFromSlave = answerFromSlave[0];
        for (int i = 1; i < numberOfPointArr / (chunk * 2); i++)
        {
            maxFromSlave = fmax(maxFromSlave, answerFromSlave[i]);
        }

        if (extra != 0)
        {
            points += finishTask * 2;
            double x = points[0];
            double y = points[1];

            for (int i = 2; i < extra; i += 2)
            {
                maxFromSlave = fmax(maxFromSlave, heavy(x, y, param));
                x = points[i];
                y = points[i + 1];
            }
        }
        free(answerFromSlave);
        printf("\nTime: %e\n", MPI_Wtime() - t1);
        printf("Answer = %e\n", maxFromSlave);
    }

    else
    {
        while (1)
        {
            double *slaveData;
            double answerToMaster, x, y;
            int chunk, tag;
            MPI_Status status;
            MPI_Recv(&chunk, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // create array that can get the data from master
            slaveData = (double *)malloc(chunk * sizeof(double));
            if (!slaveData)
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

            MPI_Recv(slaveData, chunk, MPI_DOUBLE, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // to get our tag from master to know when to stop
            tag = status.MPI_TAG;

            x = slaveData[0];
            y = slaveData[1];

            answerToMaster = heavy(x, y, param);

            for (int i = 2; i < chunk; i += 2)
            {
                x = slaveData[i];
                y = slaveData[i + 1];
                answerToMaster = fmax(answerToMaster, heavy(x, y, param));
            }
            MPI_Send(&answerToMaster, 1, MPI_DOUBLE, MASTER, tag, MPI_COMM_WORLD);
            if (tag == STOP_TAG)
                break;
            free(slaveData);
        }
    }

    MPI_Finalize();
    return 0;
}
