
#include "KMeansFileUtility.h"
#include <unistd.h>
#include <fcntl.h>

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
  if(file == NULL)
   {
     printf("There was an error in trying to open the file in parseFile()\n");
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
  int max_line_len = sizeof(char) * ((MAX_FLOAT_LENGTH*(*attributes_size))+(*attributes_size));
  char* line = malloc(max_line_len),*token;
  RawDataPoint *data_points = malloc(sizeof(RawDataPoint)*(*data_points_size));
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

/**
printMyData writes in filename a line for every point where in the i-th line there is the cluster_id of point i
**/
int printMyData(char *filename, int* clusters_id, int num_points)
{
  int fd = open(filename,O_WRONLY | O_RDONLY | O_CREAT | O_TRUNC,0666);
  if(fd == -1)
   {
     printf("There was an error in trying to open the file in printMyData()\n");
     return -1;
   }
  char buffer[MAX_INTEGER_LENGTH];
  for(int i = 0; i < num_points; i++)
   {
     for(int s = 0; s < MAX_INTEGER_LENGTH ; s++) buffer[s] = 0;
     if(snprintf(buffer,MAX_INTEGER_LENGTH,"%d\n",clusters_id[i]) == -1)
          {
            printf("snprintf failure in printMyData()\n");
            return -1;
          }
     write(fd,buffer,strlen(buffer));
   }
  if(close(fd) == -1) { printf("Error in closing the file in printResult()\n"); return -1; }
  return 0;
}




/**
Funzione che scrive i centroidi finali dei cluster nel file filename
**/
int printResult(char *filename,Cluster* clusters, int num_clusters, int num_attributes)
{
  int fd = open(filename,O_WRONLY | O_CREAT| O_TRUNC,0666);
  if(fd == -1)
   {
     printf("There was an error in trying to open the file in printResult()\n");
     return -1;
   }
  char *buffer = malloc(MAX_FLOAT_LENGTH+1);
  for(int c = 0; c < num_clusters; c++)
   {
     for(int attr = 0; attr < num_attributes; attr++)
      {
        memset(buffer,0,MAX_FLOAT_LENGTH+1);
        if(snprintf(buffer,MAX_FLOAT_LENGTH+1,"%f ",clusters[c].centroid.attributes[attr]) == -1)
          {
            printf("snprintf failure in printResult()\n");
            return -1;
          }
        write(fd,buffer,strlen(buffer));
      }
     write(fd,"\n",strlen("\n"));
   }
  free(buffer);
  if(close(fd) == -1) { printf("Error in closing the file in printResult()\n"); return -1; }
  return 0;
}
