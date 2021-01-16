# K-means clustering with OpenMPI and OpenMP/Pthread

## Generazione di dataset 

Per testare il programma abbiamo creato uno script python che genera dei dataset di punti. Per generare un dataset il comando da dare è il seguente:

```bash 
python3 datasets/generate_dataset.py num_punti num_attributi nome_file

# dataset con 100000 punti e 2 attributi
python3 datasets/generate_dataset.py 100000 2 datasets/centomila_2.txt
```
## Utilizzo

Per settare i parametri del programma si utilizzano le variabili scritte all'inizio del makefile:<br>
  * processes = numero di processi/thread
  * dataset = dataset su cui lavorare
  * num_clusters = numero di cluster

per lanciare il programma una volta settate le variabili si usano i seguenti comandi:

```bash 
make run_serial   # esegue la versione seriale
make run_mpi      # esegue la versione OpenMPI
make run_omp      # esegue la versione OpenMP
make run_pthread  # esegue la versione pthread

make clean         # rimuove i file eseguibili
make clean_results # rimuove i file .txt con i risultati
```

