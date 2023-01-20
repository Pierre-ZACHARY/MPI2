#include <iostream>
#include <mpi.h>
using namespace std;
int main2(int argc, char**argv) {

    int nprocs;
    int pid;

    MPI_Init (&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);

    MPI_Win TheWin; // Déclaration de la fenêtre

    int* tab = new int[nprocs];
    for (int i=0; i<nprocs; i++)
        tab[i]=0;

    srand(time(NULL)+pid);
    int a = rand()%100;



    // Création de la fenêtre associée au tableau tab
    // MPI_INFO_NULL pour indiquer qu'on ne donne aucune information
    MPI_Win_create(&a, sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD,&TheWin);

    tab[pid] = a;

    cout << "je suis " << pid << " tab avant:";
    for (int i=0; i<nprocs; i++)
        cout << tab[i] << " ";
    cout << " et a =" << a << endl;

    for (int i = 0; i < nprocs; i++)
        if (i != pid)
        {
            //MPI_Get(void *origin_addr, int origin_count, MPI_Datatype, origin_datatype, int target_rank, MPI_Aint target_disp, int target_count,
            //MPI_Datatype target_datatype, MPI_Win win)
            MPI_Win_lock(MPI_LOCK_SHARED, i, 0, TheWin);
            MPI_Get(tab + i, 1, MPI_INT, i, 0, 1, MPI_INT,
                    TheWin); // On va chercher la donnée dans la fenêtre distante pour la copier dans tab
            MPI_Win_unlock(i, TheWin);
        }

    // Barrière de fin sans assertion (on peut en donner pour indiquer des règles sur les instructions entre les 2 barrières)
    cout << "je suis " << pid << " tab après: ";
    for (int i=0; i<nprocs; i++)
        cout << tab[i] << " ";
    cout << endl;

    // Libération de la fenêtre.
    MPI_Win_free(&TheWin);
    MPI_Finalize();

    delete tab;

    return 0;
}
