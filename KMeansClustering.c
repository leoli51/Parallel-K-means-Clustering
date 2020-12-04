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
    int data_points_size;
    int attributes_size;
    int clusters_size; // how many clusters.. TODO: change "size" to more clear name
    int max_iterations; 
    int my_data_points_size;

    Cluster* clusters;
    ClusterDataPoint* my_data_points;

    if (my_rank == 0){
        // parse file
        char* filename;
        RawDataPoint* data_points;

        parseArgs(argc, argv, &filename, &clusters_size, &max_iterations);
        parseFile(filename, &data_points_size, &attributes_size, &data_points);

        // initialize clusters
        clusters = (Cluster*) malloc(sizeof(Cluster) * clusters_size);
        for (int i = 0; i < clusters_size; i++){
            clusters[i].cluster_id = i;
            clusters[i].centroid = data_points[i]; // Warning: This line of code probably doesnt work
        }
        // send data
    }
    else {
        // receive data
    }

    // do work


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
