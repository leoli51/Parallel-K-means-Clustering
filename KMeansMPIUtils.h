#ifndef K_MEANS_MPI_UTILS
#define K_MEANS_MPI_UTILS

#include "KMeansClusteringDefs.h"
#include "mpi.h"

// MPI datatypes
int buildMPIRawDataPointType(MPI_Datatype* data_type_pointer, int attributes_size);

#endif
