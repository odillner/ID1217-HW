#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>

#define N 8                                 /* size of problem, higher than 10 breaks printout */
#define NOF_RUNS 100                           /* how many times to run problem */

static char initial_print_out[N*5+1];

bool get_pos(int board_state[N], int* row, int column) {
    int i = *row, j, l;
    bool safe;

    for (i; i<N; i++) {

        safe = true;

        /* checks left side of row */
        for (l = 0; l < column; l++) {
            if (board_state[l] == i) {
                safe = false;
                goto end_of_loop;
            }
        }

        /* checks upper left diagonal */
        for (l=i, j=column; l>=0 && j>=0; l--, j--) {
            if (board_state[j] == l) {
                safe = false;
                goto end_of_loop;
            }
        }

        /* checks lower left diagonal */
        for (l=i, j=column; j>=0 && l<N; l++, j--) {
            if (board_state[j] == l) {
                safe = false;
                goto end_of_loop;
            }
        }

        end_of_loop:

        if (safe) {
            *row = i;
            return true;
        }
    }
    return false;
}

/* main work function, recursively solves N queens for given board state */
void solve_N_queens(int board_state[N], int column) {
    /* if all queens are placed on board, submit solution to board buffer and return */
    if (column == N) {
        int i;
        char final_print_out[N*5+1];
        strcpy(final_print_out, initial_print_out);

        for (i=0; i<N; i++){
            final_print_out[i*5+3] = board_state[i]+48;
        }

        puts(final_print_out);
        return;
    }
    /* end of printing code */
    int row = 0;
    bool done = get_pos(board_state, &row, column);

    while (done) {
        board_state[column] = row;

        solve_N_queens(board_state, column + 1);

        done = get_pos(board_state, &row, column);

        if (!done && column != 1) { /* no need to backtrack if on first column since all threads are given first queen pos */
            board_state[column] = -1; /* backtrack */
        }
    }

    /*if thread reaches end of function, no queen can be placed, or if column==0 no more solutions exist*/
}
/* creates a board state with given first queen pos */
void init_board_state(int board_state[N], int init_value){
    int i;

    board_state[0] = init_value;
    for (i = 1; i < N; i++) {
        board_state[i] = -1;
    }
}

void init_print_out(){
    int i;

    for (i=0; i<N; i++) {
        initial_print_out[i*5] = 91;
        initial_print_out[i*5+1] = i+48;
        initial_print_out[i*5+2] = 44;
        initial_print_out[i*5+3] = 48;
        initial_print_out[i*5+4] = 93;
    }
    initial_print_out[N*5] = 0;
}
/* printer/consumer handler function */

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

int main(){
    double start_time, end_time;                /* start and end times for individual runs */
    int runs, i, j;

    omp_set_num_threads(N);

#if NOF_RUNS > 1
    double total_start_time, total_end_time;    /* total start and end times */

    total_start_time = read_timer();
#endif

    for (runs = 0; runs < NOF_RUNS; runs++) {
        start_time = read_timer();

        init_print_out();

#pragma omp parallel for
        for (i = 0; i < N; i++) {
            int board[N];
            init_board_state(board, i);
            solve_N_queens(board, 1);
        }


        end_time = read_timer();

        printf("\nThe execution time for run #%d is %g sec\n\n", (runs+1), end_time - start_time);
    }

#if NOF_RUNS > 1
    total_end_time = read_timer();
    printf("The total execution time for all %d runs is %g sec\n", NOF_RUNS, total_end_time - total_start_time);
#endif

    return 0;
}
