/*
* Parallel K-means clustering 
*/

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
//#include <omp.h> TODO: uncomment
#include "KMeansClusteringDefs.h"

int main(int argc, char** argv){
    // Standard MPI code
    int communicator_size;
    int my_rank;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // K-means
    int num_data_points;
    int num_attributes;
    int num_clusters;
    int max_iterations; 
    int my_data_points_num;

    Cluster* clusters;
    ClusterDataPoint* my_data_points;
    RawDataPoint* data_points;
    RawDataPoint* my_raw_data_points

    if (my_rank == 0){
        // parse file
        char* filename;
        
        parseArgs(argc, argv, &filename, &clusters_size, &max_iterations);
        parseFile(filename, &data_points_size, &attributes_size, &data_points);
    }

    // send/receive information to create datatypes
    int information_buffer[5] = {data_points_size, my_data_points_size, attributes_size, clusters_size, max_iterations};
    MPI_Bcast(information_buffer, 5, MPI_INT, 0, MPI_COMM_WORLD);

    // copy values from buffer to vars
    if (my_rank != 0){
        data_points_size = information_buffer[0];
        data_points_per_process = information_buffer[1];
        attributes_size = information_buffer[2];
        clusters_size = information_buffer[3];
        max_iterations = information_buffer[4];

        my_data_points = (RawDataPoint*) malloc(sizeof(RawDataPoint) * data_points_per_process);
    }

    // create MPI datatype
    MPI_Datatype mpi_raw_point_type;
    createMPIRawDataPoint(&mpi_raw_point_type, attributes_size);
    MPI_Type_commit(mpi_raw_point_type);

    
    // initialize clusters
    clusters = (Cluster*) malloc(sizeof(Cluster) * clusters_size);
    for (int i = 0; i < clusters_size; i++){
        clusters[i].cluster_id = i;
    }

    // initialize cluster buffer
    float** cluster_buffer = (float**) malloc(clusters_size*sizeof(float*));
    for (int i = 0; i < clusters_size; i++){
        cluster_buffer[i] = (float*) malloc(sizeof(float) * attributes_size);
        if (my_rank == 0)
          memcpy(cluster_buffer[i], data_points[i].attributes, sizeof(float) * attributes_size); 
    }

    // send/receive cluster information
    MPI_Bcast(cluster_buffer, clusters_size, mpi_raw_point_type, 0, MPI_COMM_WORLD);

    // assign buffer attributes to cluster struct
    for (int i = 0; i < clusters_size; i++){
        clusters[i].centroid.attributes = cluster_buffer[i];
    }

    #if DEBUG
    printf("Rank:%d, Clusters:\n");
    for (int i = 0; i < clusters_size; i++){
      printf("Cluster %d: ", clusters[i].cluster_id);
      for (int j = 0; j < attributes_size; j++)
        printf("%f", clusters[i].centroid.attributes[j]);
      printf("\n");
    }
    #endif

    // send data points
    //MPI_Scatter(data_points, data_points_per_process, );

    // TODO: handle non int divisible data

    // do work

    // free cluster memory
    for (int i = 0; i < clusters_size; i++)
      free(clusters[i].attributes);
    free(clusters);

    // free data points.

    MPI_Finalize();
    return 0; // Return correct status 
}

/**
function that parses the input arguments in:
-filename, the filename of the file where the data_points are defined
-clusters_size, the number of cluster to create
-max_iterations, if defined by the user is the maximum number of iterations of the alghoritm, otherwise is NULL

The function returns:
-1 if there was an error in parsing the arguments
0 otherwise.
**/ 
int parseArgs(int argc, char** argv, char** filename, int* clusters_size, int* max_iterations)
{
  if(argc<3)
    {
      printf("Usage of k-means: ./kmeans filename clusters_size [max_iterations]\n");
      return -1;
    }
  *filename = argv[1];
  *clusters_size = atoi(argv[2]);

  if(argc == 3) *max_iterations = -1;
  else
   {
     int max = atoi(argv[3]); 
     max_iterations = &max;
   }
  return 0;
}

/**
function that reads the line of the file pointed by "file" and put it in "line"
returns 0 on success, -1 otherwise
**/
int readLine(FILE* file, char* line)
{
 char read_char,*pointer = line;
 do
 {
   read_char = (char) fgetc(file);
   if(read_char != '\n' && read_char != EOF)
   	*pointer++ = read_char;
   else if(read_char == EOF) return -1;
   else break;
 }while(1);
 return 0;
}

/**
function that parses a formatted file to get:
-data_points_size: the pointer to the number of data_points
-attributes_size : the pointer to the number of attributes of each data_point
-array_of_datapoints: which is an array of the data_points that are going to be divided in the clusters

The function returns:
-1 if there was an error in parsing the file
0 otherwise
**/
int parseFile(const char* filename,int* data_points_size, int* attributes_size, RawDataPoint** array_of_datapoints)
{
  FILE *file = fopen(filename,"r");
  char read_char;
  if(file == NULL)
   {
     printf("There was an error in trying to open the file\n");
     return -1;
   }
  char* firstRowBuffer = malloc(sizeof(char)*((MAX_INTEGER_LENGTH*2)+2));
  if(readLine(file,firstRowBuffer) == -1)
   {
    printf("In parseFile() error reading the file\n");
   }
  *data_points_size = atoi(strtok(firstRowBuffer," "));
  *attributes_size = atoi(strtok(NULL," "));
  free(firstRowBuffer);
  int max_line_len = sizeof(char) * ((MAX_INTEGER_LENGTH*(*attributes_size))+(*attributes_size));
  char* line = malloc(max_line_len),*token;
  RawDataPoint *data_points = malloc(sizeof(RawDataPoint)*(*attributes_size));
  for(int i = 0; i < (*data_points_size); i++)
   {
     line = (char*) memset(line,0,max_line_len);
     if(readLine(file,line) == -1)
   	{
   	 printf("In parseFile() error reading %d line of the file\n",i+2);
   	 return -1;
  	}
     data_points[i].attributes = malloc(sizeof(float)*(*attributes_size));
     for(int j = 0; j < (*attributes_size); j++)
      {
        if(j == 0) token = strtok(line," ");
        else token = strtok(NULL," ");
        if(token == NULL)
         {
           printf("Error in parsing the file, the number of attributes given exceed the real attributes defined\n");
           return -1;
         }
        data_points[i].attributes[j] = (float) atof(token);
      }
   }
  free(line);
  *array_of_datapoints = data_points;
  if(fclose(file) == EOF)
   {
     printf("There was an error in trying to close the file\n");
     return -1;
   }
 return 0;
}
