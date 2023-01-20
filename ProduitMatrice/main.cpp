//
// Created by tpuser on 20/01/23.
//


#include <iostream>
#include <mpi.h>
#include <chrono>
#include <fstream>

using namespace std;
void generation_vecteur(int n, int* vecteur, int nb_zero) {
    for (int i=0; i<nb_zero; i++)
        vecteur[i] = 0;
    for (int i=nb_zero; i<n; i++)
        vecteur[i] = rand()%20;
    for (int i=0; i<n; i++)
        cout << vecteur[i] << " ";
    cout << endl;
}

void matrice_vecteur(int n, int* matrice, int* v1, int* v2) {
    int ptr = 0;
    while(v1[ptr]==0)
        ptr++;
    for (int i=0; i<n; i++) {
        v2[i] = 0;
        for (int j = ptr; j < n; j++)
            v2[i] += v1[j] * matrice[i * n + j];
    }
}

int main(int argc, char **argv){

    int provided; // renvoi le mode d'initialisation effectué
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); // MPI_THREAD_MULITPLE chaque processus MPI peut faire appel à plusieurs threads.

    // Pour connaître son pid et le nombre de processus de l'exécution paralléle (sans les threads)
    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int n = atoi(argv[1]); // taille de la matrice carrée
    int m = atoi(argv[2]); // nombre de vecteurs en entrée
    int root = atoi(argv[3]); // processeur root : référence pour les données

    string name = argv[4]; // le nom du fichier pour que le processus root copie les données initiales et les résultats

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    int *matrice = new int[n * n]; // la matrice
    int *vecteurs; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.

    fstream f;
    if (pid == root) {
        f.open(name, std::fstream::out);
        srand(time(NULL));
        for (int i = 0; i < n * n; i++) matrice[i] = rand() % 20;
        f << "Matrice" << endl;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) f << matrice[i * n + j] << " ";
            f << endl;
        }
        f << endl;

        vecteurs = new int[m * n];
        for (int i = 0; i < m; i++) {
            int nb_zero = rand() % (n / 2);
            generation_vecteur(n, vecteurs + i * n, nb_zero);
        }
        f << "Les vecteurs" << endl;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) f << vecteurs[i * n + j] << " ";
            f << endl;
        }
    }

    if (pid == root) debut = chrono::system_clock::now();

    MPI_Win TheWinMatrice; // Déclaration de la fenêtre
    MPI_Win_create(matrice, n * n *sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &TheWinMatrice);
    // TODO : distribuer la matrice aux processus
    for(int i = 0; i < nprocs; i++){
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, i, 0, TheWinMatrice);
        MPI_Put(matrice, n * n, MPI_INT, root, i, n * n, MPI_INT, TheWinMatrice);
        MPI_Win_unlock(i, TheWinMatrice);
    }

    // TODO : recup les vecteurs
    MPI_Win_fence(0, TheWinMatrice);
    //chaque processus vient lire x vecteurs sur le processus root
    //chaque processus effectue ses calculs et dépose au fur et à
    //mesure le résultat sur le processus root
    // à partir de là, vecteurs contient tous les vecteurs à traiter
    int nbVecteursLocal = m / nprocs; // nombre de vecteurs à traiter par chaque processeur
    if(pid < m % nprocs) nbVecteursLocal++; // gestion des vecteurs restants
    if(pid !=root) vecteurs = new int[nbVecteursLocal * n]; // allocation de la mémoire pour les vecteurs locaux

    MPI_Win TheWinVecteur; // Déclaration de la fenêtre
    if(pid != root) vecteurs = new int[nbVecteursLocal * n];
    if(pid==root) MPI_Win_create(vecteurs, m * n*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &TheWinVecteur);
    else MPI_Win_create(vecteurs, nbVecteursLocal * n*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &TheWinVecteur);

    int offset = 0;
    for(int i = 0; i < pid; i++){
        if(i < m % nprocs) offset++;
        offset += m / nprocs;
    }
    for(int i = 0; i < nbVecteursLocal; i++){
        int offset =
        MPI_Win_lock(MPI_LOCK_Shared, root, 0, TheWinMatrice);
        MPI_Get(vecteurs + i * n, n, MPI_INT, root, offset + i, n, MPI_INT, TheWinMatrice);
        MPI_Win_unlock(root, TheWinMatrice);
    }

    // TODO : calculer les produits vecteurs matrices et les placer sur root au fur et à mesure



    // Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    if (pid == root) {
        fin = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = fin - debut;
        cout << "temps en secondes : " << elapsed_seconds.count() << endl;

    }

    int *resultats;
    if (pid == root) resultats = new int[m * n];



    //affichage
    if (pid == root) {
        f << "Les vecteurs" << endl;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++)
                f << resultats[i * n + j] << " ";
            f << endl;
        }
        f.close();
    }

    MPI_Finalize();
    return 0;
}