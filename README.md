# K-means clustering with OpenMPI and OpenMP/Pthread

## Dataset generation 

To test the program we created a python script that generates datasets. A dataset is a collection of points. To generate a dataset you must run the following command in the terminal:

```bash 
python3 datasets/generate_dataset.py num_points num_attributes/feature/dimension_of_space filename

# dataset with 100000 points and 2 attributes (100000 points in 2d space)
python3 datasets/generate_dataset.py 100000 2 datasets/centomila_2.txt
```
## How to use

To set the parameters of the program you must change the variables at the beginning of the makefile:<br>
  * processes = number of processes/threads
  * dataset = dataset filename
  * num_clusters = number of clusters

To execute the program, once all the desired parameters are set, you can use the following commands:

```bash 
make run_serial   # executes the serial version
make run_mpi      # executes the OpenMPI version
make run_omp      # executes the OpenMP version
make run_pthread  # executes the pthread version

make clean         # removes the executables (overall cleaning)
make clean_results # removes the output .txt files containing the results
```

