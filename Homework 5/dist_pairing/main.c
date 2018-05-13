#include "mpi.h"
#include <stdio.h>

#define GROUP_SIZE 2

void teacher(int size) {
    int nof_remaining_students =  size - 1;

    int students[GROUP_SIZE];
    int curr_group_size = 0;

    while (nof_remaining_students > 0) {
        /* recieve request from student to be put in a group */
        MPI_Recv(&students[curr_group_size], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        curr_group_size++;
        nof_remaining_students--;

        /* group is full */
        if (curr_group_size == GROUP_SIZE || (nof_remaining_students == 0 && curr_group_size > 1)) {

            /* send students their group sizes */
            for (int i = 0; i < curr_group_size; i++) {
                MPI_Send(&curr_group_size, 1, MPI_INT, students[i], 0, MPI_COMM_WORLD);
            }

            /* send students assigned group members */
            for (int i = 0; i < curr_group_size; i++) {
                for (int j = 0; j < curr_group_size; j++) {
                    if (j != i) {
                        MPI_Send(&students[i], 1, MPI_INT, students[j], 0, MPI_COMM_WORLD);
                    }
                }
            }

            curr_group_size = 0;

        /* no other students to pair with */
        } else if (nof_remaining_students == 0 && curr_group_size == 1) {
            MPI_Send(&curr_group_size, 1, MPI_INT, students[0], 0, MPI_COMM_WORLD);
            break;
        }
    }
}

void student(int rank) {
    int group_size = 0;

    /* request to be put in group */
    MPI_Send(&rank, 1,	MPI_INT, 0, 0, MPI_COMM_WORLD);

    /* wait for group size */
    MPI_Recv(&group_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (group_size > 1) {
        int nof_partners = group_size-1;
        int group[nof_partners];

        /* recieve other group members */
        for (int i = 0; i < nof_partners; i++) {
            MPI_Recv(&group[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Student #%d: I have the following partners: ", rank);
        for (int i = 0; i < nof_partners; i++) {
            printf("student #%d, ", group[i]);
        }
        printf("\n");
    } else {
        /* no other students remaining */
        printf("Student #%d: I am in a group by myself", rank);
    }
}
int	main(int argc, char ** argv) {
    int	rank, size;

    MPI_Init(&argc,	&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        teacher(size);
    } else {
        student(rank);
    }

    MPI_Finalize();
}

