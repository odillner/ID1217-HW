#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"

void table() {
    bool forks[5] = {[0 ... 4] true};
    bool forks_available;

    bool philosophers_eating[5] = {[0 ... 4] false};

    int left_fork;
    int right_fork;

    int curr_philosopher;

    while (true) {
        MPI_Recv(&curr_philosopher, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        curr_philosopher--;

        left_fork = curr_philosopher;
        right_fork = (curr_philosopher != 4) ? curr_philosopher + 1 : 0;

        if (philosophers_eating[curr_philosopher]) {

            philosophers_eating[curr_philosopher] = false;
            forks[left_fork] = true;
            forks[right_fork] = true;

        } else {

            forks_available = (forks[left_fork] && forks[right_fork]);

            MPI_Send(&forks_available, 1, MPI_C_BOOL, curr_philosopher+1, 0, MPI_COMM_WORLD);

            if (forks_available) {
                philosophers_eating[curr_philosopher] = true;

                forks[left_fork] = false;
                forks[right_fork] = false;
            }
        }
    }
}

void philosopher(int rank){
    bool forks_available;

    time_t t;

    srand((unsigned) time(&t));


    /*
    printf("Philosopher #%d will think for a random amount of time\n", rank);

     */

    while (true) {
        /* ask table if forks are available */

        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        MPI_Recv(&forks_available, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (forks_available) {
            printf("Philosopher #%d has both forks in his possession will now eat\n", rank);

            sleep(rand() % 5);

            MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("Philosopher #%d is done eating and has returned his forks, he will now think\n", rank);

            sleep(rand() % 5);
        } else {
            sleep(1);
        }
    }
}
int	main(int argc, char ** argv) {
    int	rank, size;

    MPI_Init(&argc,	&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        table();
    } else {
        philosopher(rank);
    }

    MPI_Finalize();
}

