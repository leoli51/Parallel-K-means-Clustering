#ifndef K_MEANS_MPI_UTILS
#define K_MEANS_MPI_UTILS

#include <stdio.h>
#include "KMeansClusteringDefs.h"
#include "mpi.h"

// MPI datatypes
int buildMPIPointBufferType(MPI_Datatype* data_type_pointer, int attributes_size, int stride, int point_count);

void printPoints(RawDataPoint* points, int num_points, int num_attributes);
#endif
