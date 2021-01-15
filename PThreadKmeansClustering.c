/**
PThread K-Means Clustering
**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void updateClusters();
int smartParseFile(const char* filename,int* data_points_size, int* attributes_size, ClusterDataPoint** array_of_datapoints);
int assignPointsToNearestClusterSerial(void *void_args);
int threadParseArgs(int argc, char** argv, char** filename, int* max_iterations);

ClusterDataPoint* my_data_points;
Cluster *clusters;

//syncrhonizations
pthread_mutex_t lock; //lock on hasChanged

pthread_barrier_t barrierMemset,barrierCentroids;

//shared variables
_Bool hasChanged = 0;
int num_iterations = 0;
int num_attributes,num_clusters,num_data_points, data_per_thread, num_thread;

int *nums;
float *sums;


int main(int argc, char** argv)
{
  // K-means
  int max_iterations = -1;
  
  //timing
  clock_t start_t,finish;
  
  pthread_mutex_init(&lock,NULL);
  
  // parse file
  char* filename;
  if(threadParseArgs(argc, argv, &filename, &max_iterations) == -1) return -1;
  if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
  if(smartParseFile(filename, &num_data_points, &num_attributes, &my_data_points) == -1) return -1;
  if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } 
 
  start_t = clock();
  
  nums = malloc(sizeof(int) * num_clusters * num_thread);
  sums = malloc(sizeof(float) * num_clusters * num_attributes * num_thread);
  
  
  // initialize clusters as empty
  clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
  for (int i = 0; i < num_clusters; i++) 
   {
     clusters[i].centroid.attributes = malloc(sizeof(float)*num_attributes);
     for(int attr = 0; attr < num_attributes; attr++) clusters[i].centroid.attributes[attr] = my_data_points[i].data_point.attributes[attr]; 
     clusters[i].cluster_id = i; 
   }
 
  
  //do work
  data_per_thread = num_data_points/num_thread;
  
  pthread_barrier_init(&barrierMemset,NULL,num_thread);
  pthread_barrier_init(&barrierCentroids,NULL,num_thread);
  
  if(num_thread <= 0) num_thread = 1;
  pthread_t threads[num_thread];
  int start;
  for(int th = 0 ; th < num_thread; th++) //launch every thread to analyze a portion of data
      {
       start = th * data_per_thread;
       pthread_create(&threads[th],NULL,(void*)assignPointsToNearestClusterSerial,(void*)start);
      }
  for(int th = 0; th < num_thread; th++) pthread_join(threads[th],NULL); //joins

 finish = clock(); 
 
 pthread_mutex_destroy(&lock);
 pthread_barrier_destroy(&barrierMemset);
 pthread_barrier_destroy(&barrierCentroids);
 
 double elapsed = (double)(finish - start_t)/CLOCKS_PER_SEC;
 
 int *clusters_id = (int*) malloc(sizeof(int)*num_data_points);
 for(int i = 0; i < num_data_points; i++) clusters_id[i] = my_data_points[i].cluster_id;
 
 char *clusters_filename = "threadClusters.txt";
 char *result = "threadResult.txt";
 
 printResult(result,clusters,num_clusters,num_attributes);
 printMyData(clusters_filename,clusters_id,num_data_points);
 
 free(clusters_id);
 
 
 printf("Result obtained with %d iterations\n",num_iterations);
 printf("Final centroids written in %s, clusters_id of points written in %s\n",result,clusters_filename);
 printf("Elapsed time: %e seconds\n",elapsed);
 
 free(sums);
 free(nums);
 
 
 // free cluster memory
 for (int i = 0; i < num_clusters; i++)
     free(clusters[i].centroid.attributes);
    
 free(clusters);
 
 // free data points.
 for (int i = 0; i < num_data_points; i++)
    free(my_data_points[i].data_point.attributes);
    
 //free(raw_data_points);
 free(my_data_points);
 return 0;
}


int assignPointsToNearestClusterSerial(void* void_args)
{
  int start = (int) void_args,stop = start+data_per_thread; //every thread has its start point for the data and calculates his end point
  int thread_id = start/data_per_thread; //we get the thread_id that goes from 0 to num_thread-1
  if(stop > num_data_points) stop = num_data_points; //make sure we don't surpass num_data_points
  else if(thread_id == (num_data_points/data_per_thread)-1) stop = num_data_points; //the last thread takes up to the last point
  int i,j,n_attr,cluster_index;
  float distance,min_distance,temp;
  ClusterDataPoint point;
  Cluster cluster;
  //do work
  do{
     pthread_barrier_wait(&barrierMemset); //wait for other threads in order to make sure they entered the while
     hasChanged = 0;
     for(i = 0; i < num_clusters; i++)
         {
           nums[(thread_id * num_clusters)+i] = 0;
           for(j = 0; j < num_attributes; j++)
             sums[(thread_id * num_clusters * num_attributes) + (i*num_attributes)+j] = 0;
         }
     if(thread_id == 0) num_iterations++;
     for(i = start; i < stop; i++) //for every point to analyze
      {
        point = my_data_points[i]; //take a point
        for(j = 0; j < num_clusters; j++) //for every cluster
         {
           distance = 0;
           cluster = clusters[j]; //take a cluster
 	   for(n_attr = 0; n_attr < num_attributes; n_attr++) //calculate the distance between the point and the cluster
 	    {
 	      temp = (point.data_point.attributes[n_attr] - cluster.centroid.attributes[n_attr]);
 	      distance += temp * temp;
 	    }
 	   if(j == 0 || distance < min_distance) //if it is the first cluster or it is the nearest up to now
 	   {
 	     min_distance = distance; //consider it as the nearest
 	     cluster_index = j;
 	   }
        }
        if(point.cluster_id != cluster_index)
         {
           my_data_points[i].cluster_id = cluster_index; //assign the nearest cluster to the point
           if(!hasChanged) //if hasChanged is 0, change it to 1 using a mutex
            {
             pthread_mutex_lock(&lock);
             hasChanged = 1;
             pthread_mutex_unlock(&lock);
            }
         }
        nums[cluster_index + (thread_id * num_clusters)] += 1;
        for(n_attr = 0; n_attr < num_attributes; n_attr++) sums[(thread_id * num_clusters * num_attributes) + (cluster_index * num_attributes) + n_attr] += point.data_point.attributes[n_attr];
      }
     pthread_barrier_wait(&barrierCentroids);
     if(thread_id == 0) updateClusters(); //thread 0 update centroids gathering the local sums and nums of every thread
   }while(hasChanged);
  return 0;
}

//function that updates the centroids of the clusters
void updateClusters()
{
  //gathering sums and nums
  for(int t = 0; t < num_thread; t++)
   {
     for(int c = 1; c < num_clusters; c++)
       {
         nums[c] += nums[(t * num_clusters) + c];
         for(int attr = 0; attr < num_attributes; attr++)
           sums[(c*num_attributes)+attr] += sums[(t * num_clusters * num_attributes) + (c * num_attributes) +attr];
       }
   }
   
  for(int c = 0; c < num_clusters; c++) //make the average
     for(int i = 0; i < num_attributes; i++)
       if(nums[c] != 0) clusters[c].centroid.attributes[i] = sums[(c*num_attributes)+i] / nums[c];
       else clusters[c].centroid.attributes[i] = 0;
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
int smartParseFile(const char* filename,int* data_points_size, int* attributes_size, ClusterDataPoint** array_of_datapoints)
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
  ClusterDataPoint *data_points = malloc(sizeof(ClusterDataPoint)*(*data_points_size));
  for(int i = 0; i < (*data_points_size); i++)
   {
     line = (char*) memset(line,0,max_line_len);
     if(readLine(file,line) == -1)
   	{
   	 printf("In parseFile() error reading %d line of the file\n",i+2);
   	 return -1;
  	}
     data_points[i].data_point.attributes = malloc(sizeof(float)*(*attributes_size));
     for(int j = 0; j < (*attributes_size); j++)
      {
        if(j == 0) token = strtok(line," ");
        else token = strtok(NULL," ");
        if(token == NULL)
         {
           printf("Error in parsing the file, the number of attributes given exceed the real attributes defined\n");
           return -1;
         }
        data_points[i].data_point.attributes[j] = (float) atof(token);
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

//parse the input
int threadParseArgs(int argc, char** argv, char** filename, int* max_iterations)
{
  if(argc<4)
    {
      printf("Usage of k-means: ./kmeans threads filename clusters_size [max_iterations]\n");
      return -1;
    }
  num_thread = atoi(argv[1]);
  *filename = argv[2];
  num_clusters = atoi(argv[3]);

  if(argc == 4) *max_iterations = -1;
  else
   {
     int max = atoi(argv[3]);
     max_iterations = &max;
   }
  return 0;
}
