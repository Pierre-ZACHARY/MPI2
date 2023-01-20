#include <iostream>
#include <mpi.h>
using namespace std;
int main(int argc, char**argv) {

    int nprocs;
    int pid;

    MPI_Init (&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);

    MPI_Win TheWin; // Déclaration de la fenêtre
    MPI_Win TheWin2; // Déclaration de la fenêtre


    int n = atoi(argv[1]);
    int root = atoi(argv[2]);

    if (n%nprocs!=0) {
        cout << "Attention ce programme demande une taille de tableau divisible par le nombre de processeurs" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int* tab;

    if (pid==root) {
        srand(time(NULL));
        tab = new int[n];
        for (int i = 0; i < n; i++) tab[i] = rand() % 10;
        cout << "tableau pour la réduction : ";
        for (int i=0; i<n; i++)
            cout << tab[i] << " ";
        cout << endl;
    }

    int n_local = n/nprocs;

    int * tab_local = new int[n_local];


    // A compléter à partir d'ici

    MPI_Win_create(tab, n*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &TheWin);
    MPI_Win_create(tab_local, n_local*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD, &TheWin2);


    //MPI_Get(void *origin_addr, int origin_count, MPI_Datatype, origin_datatype, int target_rank, MPI_Aint target_disp, int target_count,
        //MPI_Datatype target_datatype, MPI_Win win)
    MPI_Win_lock(MPI_LOCK_SHARED, root, 0, TheWin);
    MPI_Get(tab_local, n_local, MPI_INT, root, pid*n_local, n_local, MPI_INT, TheWin);
    MPI_Win_unlock(root, TheWin);

    cout << "je suis " << pid << " tab_local après: ";
    for (int i=0; i<n_local; i++)
        cout << tab_local[i] << " ";
    cout << "\n";



    MPI_Win_lock(MPI_LOCK_SHARED, root, 0, TheWin2);
    // mpi acumulate de la somme de tab_local de chaque processus vers tab_local de root
        //int MPI_Accumulate(const void *origin_addr, int origin_count, MPI_Datatype
        //                   origin_datatype, int target_rank, MPI_Aint
        //                   target_disp, int target_count, MPI_Datatype
        //                   target_datatype, MPI_Op op, MPI_Win win)
    if(pid!=root) MPI_Accumulate(tab_local, n_local, MPI_INT, root, 0, n_local, MPI_INT, MPI_SUM, TheWin2);


    MPI_Win_unlock(root,  TheWin2);

    MPI_Win_fence(0, TheWin2);
    if (pid==root) {
        cout << "le résultat : ";
        for(int i = 0; i < n_local; i++) cout << tab_local[i] << " ";
        cout << endl;
    }

//    MPI_Win_free(&TheWin);
    MPI_Finalize();

    return 0;
}
