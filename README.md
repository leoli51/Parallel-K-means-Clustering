# K-means clusetering with OpenMPI and OpenMP/Pthread

## conventions

Usiamo lo stile di MPI per le funzioni (?):
tutte le funzioni restituiscono un intero che rappresenta lo stato 
i parametri che devono essere modificati vengono passati come pointer

Camel case metodi: parseFile();<br>
Snake case variabili: raw_data_point;<br>

nomi descrittivi

## datatypes

numero cluster : int
[numero massimo iterazioni : int]

struct cluster-data-point
    - cluster id : int
    - data point : raw-data-point

struct raw-data-point
    - attributi : float*

struct cluster
    - cluster id : int
    - centroide : (raw-data-point)

## scheletro algoritmo

1. parsing del file
2. inizializzazione cluster
3. distribuire i data-point alle varie macchine
4. analisi data-point proprietari e assegnazione cluster
5. calcolo centroide locale
6. calcolo centroide distrubuito
7. ripeti dal punto 4 se ci sono state modifiche ai cluster
8. output

## 1

linee contenenti attributi separati da spazi.
Ogni linea è un data point.

Master esegue il parsing del file.

## 2

Crea i cluster gli assegna punti a caso e li distribuisce come array di cluster.

## 3

distribuisce i dati in maniera equa.
Distribuisce come array di raw-data-points.

## 4

Ogni macchina analizza la sua parte di data-points e assegna un cluster a ciascuno. 
Ottimizzazioe locale con openMP / PThread.

## 5 

Calcolo centroide locale 