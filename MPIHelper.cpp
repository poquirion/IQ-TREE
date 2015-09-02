//
// Created by tung on 6/18/15.
//

#include "MPIHelper.h"
#include "ObjectStream.h"

/**
 *  Initialize the single getInstance of MPIHelper
 */

MPIHelper& MPIHelper::getInstance() {
    static MPIHelper instance;
    return instance;
}

void MPIHelper::sendTreesToOthers(TreeCollection &trees, bool blocking) {
    MPI_Request requestNull;
    for (int i = 0; i < getNumProcesses(); i++) {
        if (i != getProcessID()) {
            //ObjectStream* os = new ObjectStream();
            //os->initFromTreeCollection(trees);
            ObjectStream* os = new ObjectStream(trees);
            if (blocking) {
                MPI_Send(os->getObjectData(), os->getDataLength(), MPI_CHAR, i, 1, MPI_COMM_WORLD);
            } else {
                MPI_Isend(os->getObjectData(), os->getDataLength(), MPI_CHAR, i, 1, MPI_COMM_WORLD, &requestNull);
            }
        }
    }
}

TreeCollection MPIHelper::getTreesFromOthers(int numNodes) {
    if (numNodes > (getNumProcesses() - 1)) {
        numNodes = getNumProcesses() - 1;
    }
    MPI_Status status;
    char* recvBuffer;
    int MPISource, flag;
    int numBytes;
    TreeCollection allTrees;
    // Process all pending messages
    while (numNodes > 0) {
        do {
            // Check for incoming messages
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            // flag == true if there is a message
            if (flag) {
                MPISource = status.MPI_SOURCE;
                MPI_Get_count(&status, MPI_CHAR, &numBytes);
                recvBuffer = new char[numBytes];
                MPI_Recv(recvBuffer, numBytes, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, NULL);
                ObjectStream os(recvBuffer, numBytes);
                TreeCollection curTrees = os.getTreeCollection();
                allTrees.addTrees(curTrees);
                delete [] recvBuffer;
                numNodes--;
            }
        } while (flag);
    }

    return allTrees;
}
