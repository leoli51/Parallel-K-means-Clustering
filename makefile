run = mpirun
compiler = mpicc
target = kmeans.o
processes = 3
src = $(filter-out SerialKMeansClustering.c, $(wildcard *.c))
libs  = #-lkernel32 -luser32 -lgdi32 -lopengl32
cflags = #-Wall
filename = datasets/cloud_test.txt
num_clusters = 3

.PHONY : clean

$(target): $(src)
	$(compiler) -o $@ $^ $(cflags) $(libs)

run_local : $(target)
	$(run) -np $(processes) --oversubscribe -mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)
	python datasets/generate_plot.py $(filename) result.txt

run_distributed : $(target)
	$(run) -np $(processes) --oversubscribe --host node1,node2,node3 --mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)

serial.o : SerialKMeansClustering.c
	   gcc -Wall SerialKMeansClustering.c -o serial.o

run_serial : serial.o
	     ./serial.o $(filename) $(num_clusters)

clean :
	rm $(target)
