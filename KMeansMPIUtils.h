#ifndef K_MEANS_MPI_UTILS
#define K_MEANS_MPI_UTILS

#include "KMeansClusteringDefs.h"

// MPI datatypes
int createMPIRawDataPointType(MPI_Datatype* data_type_pointer, int attributes_size);

int createMPIClusterType(MPI_Datatype* data_type_pointer, int attributes_size);

#endif