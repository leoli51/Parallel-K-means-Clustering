#ifndef K_MEANS_FILE_UTILITY
#define K_MEANS_FILE_UTILITY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "KMeansClusteringDefs.h"

// parsing prototypes
int parseArgs(int argc, char** argv, char** filename, int* clusters_size, int* max_iterations);
int parseFile(const char* filename,int* data_points_size, int* attributes_size, RawDataPoint** array_of_datapoints);
int readLine(FILE* file,char* line);

#endif
