run = mpirun
compiler = mpicc
target = kmeans.o
processes = 3
src=$(wildcard *.c)
libs  = #-lkernel32 -luser32 -lgdi32 -lopengl32
cflags = #-Wall
filename = datasets/test_dataset2.txt
num_clusters = 2

.PHONY : clean

$(target): $(src)
	$(compiler) -o $@ $^ $(cflags) $(libs)

run_local : $(target)
	$(run) -np $(processes) --oversubscribe -mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)

run_distributed : $(target)
	$(run) -np $(processes) --oversubscribe --host node1,node2,node3 --mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)

clean :
	rm $(target)
