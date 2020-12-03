
/**
 * Creates a raw data point as an MPI datatype. 
 * */
int buildMPIRawDataPointType(MPI_Datatype* data_type_pointer, int attributes_size){
    MPI_Type_contiguous(attributes_size, MPI_FLOAT, data_type_pointer);
    MPI_Type_commit(data_type_pointer);

    return 0;
}    
