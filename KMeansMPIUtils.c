#include "KMeansMPIUtils.h"

/**
 * Creates a raw data point as an MPI datatype.
 * */
int buildMPIPointBufferType(MPI_Datatype* data_type_pointer, int attributes_size, int stride, int point_count){
    //MPI_Type_contiguous(attributes_size, MPI_FLOAT, data_type_pointer);
    MPI_Type_vector(point_count, attributes_size, stride, MPI_FLOAT, data_type_pointer);
    return 0;
}

void printPoints(RawDataPoint* points, int num_points, int num_attributes){
    for (int i = 0; i < num_points; i++){
        printf("\npoint: %d\n\t", i);
        for (int j = 0; j < num_attributes; j++){
            printf("%f ", points[i].attributes[j]);
        }
    }
    printf("\n");
}
