

/*
 * #################### PLEASE READ #########################
 *
 * While the assignment was an interesting problem to parallelize, any attempts to demonstrate the generated
 * solutions causes the parallelism to be redundant since printing can't really be done any faster and in the end
 * any measure of performance simply demonstrates how long it takes to print X amount of arrays.
 *
 * In fact in the given solution you have provided the program actually executes slower the more threads you add,
 * performing the best if only one thread is created.
 *
 * If actual speedup is to be measured, please define DISABLE_PRINTING which disables printing.
 * Increasing NOF_RUNS is also recommended since the problem size is quite small per default, making measurements
 * unreliable with only a single run performed.
 *
 * Please note that the program has only been extensively tested for N=8, other cases return the correct number of solutions
 * but they might be incorrect or the program might have undetermined behavior for those cases.
 *
 * #################### PLEASE READ #########################
 */


#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

//#define DISABLE_PRINTING
#define N 8                                 /* size of problem */
#define MAXWORKERS 10
#define BUF_LEN 4                           /* length of print buffer */
#define NOF_RUNS 1                             /* how many times to run problem */

int nof_solutions = 1;                      /* number of submitted solutions */
int task = 0;                               /* bag of tasks */
int print_buf[BUF_LEN][N];                  /* buffer of solutions to be printed */
int buf_pos = 0;                            /* position in print buffer */

bool empty = true;                          /* print buffer empty flag */
bool full = false;                          /* print buffer full flag */
bool producers_done = false;                /* producers done flag */

pthread_cond_t buf_empty;                   /* print buffer empty condition */
pthread_cond_t buf_full;                    /* print buffer full condition */

pthread_mutex_t work_lock;                  /* bag of tasks */
pthread_mutex_t buf_lock;                   /* prevents threads from submitting solution to buffer at the same time */


/* submits board state for printing */
void submit_board_state(int board_state[N]) {
    int i;
    int temp;

    pthread_mutex_lock(&buf_lock);

    while (full) {
        pthread_cond_wait(&buf_empty, &buf_lock);
    }

    empty = false;

    for (i=0; i < N; i++) {
        temp = board_state[i] + 48; /*offloads a little work from printer thread */
        print_buf[buf_pos][i] = temp;
    }

    buf_pos++;

    if (buf_pos>=BUF_LEN){
        full = true;
        pthread_cond_broadcast(&buf_full);
    }

    pthread_mutex_unlock(&buf_lock);
}

/* checks if given position is safe queen placement */
bool safe_pos(int board_state[N], int row, int column) {
    int i, j;

    /* left side of row */
    for (i = 0; i < column; i++) {
        if (board_state[i] == row) {
            return false;
        }
    }

    /* upper left diagonal */
    for (i=row, j=column; i>=0 && j>=0; i--, j--) {
        if (board_state[j] == i) {
            return false;
        }
    }

    /* lower left diagonal */
    for (i=row, j=column; j>=0 && i<N; i++, j--) {
        if (board_state[j] == i) {
            return false;
        }
    }

    return true;
}

/* modifies row to safe position, return value signifies if safe position exists for given board state */
bool get_pos(int board_state[N], int* row, int column) {
    int i = *row;

    for (i; i<N; i++) {
        if (safe_pos(board_state, i, column)) {
            *row = i;
            return true;
        }
    }
    return false;
}

/* main work function, recursively solves N queens for given board state */
void solve_N_queens(int board_state[N], int column) {
    /* checks if all queens has been placed then prints */
    if (column == N) {
#ifndef DISABLE_PRINTING
        submit_board_state(board_state);
#endif
        return;
    }

    int row = 0;

    bool done = !get_pos(board_state, &row, column);

    while (!done) {
        board_state[column] = row;

        solve_N_queens(board_state, column + 1);

        done = !get_pos(board_state, &row, column);

        if (done && column > 2) { /* no need to backtrack if on first column since all threads are given first queen pos */
            board_state[column] = -1; /* backtrack */
        }
    }

    /*if thread reaches end of function, no queen can be placed, or if column==0 no more solutions exist*/
}
static int steps[N];
/* creates a board state with given first queen pos */
void init_board_state(int board_state[N], int curr_task){
    int i;

    for (i = 1; i < N; i++) {
        board_state[i] = -1;
    }
    board_state[0] = curr_task;
}

/* bag of tasks handler function, assigns work to producer threads */
void* Producer(void* null) {
    int i;
    int board[N]; /* create and init board state */

    while (1) {
        pthread_mutex_lock(&work_lock);
        i = task;
        task++;

        if (i == N) {
            pthread_mutex_unlock(&work_lock);
            break;
        }
        pthread_mutex_unlock(&work_lock);

        init_board_state(board, i);
        solve_N_queens(board, 2);
    }

    return NULL; /* thread finished, all possible solutions printed */
}
void* Child(void* arg) {

}
void* Parent(void* null){
    struct { int board; bool slice; } args[2];
    args[0].slice = true;
    args[1].slice = false;

    int board[N];

    int i;
    pthread_t children[2];

    while (1) {
        pthread_mutex_lock(&work_lock);
        i = task;
        task++;

        if (i == N) {
            pthread_mutex_unlock(&work_lock);
            break;
        }
        pthread_mutex_unlock(&work_lock);

        init_board_state(board, i);
        args[0].board = board;
        args[1].board = board;
        for (i = 0; i < 2; i++) {
            pthread_create(&children[i], NULL, Child, (void *) args[i]);
        }

        for (i = 0; i < 2; i++) {
            pthread_join(children[i], NULL);
        }
    }
}
/* prints given board state */
void print_board_state(int board_state[N]) {
    printf("\n\nSolution no: %d\n",nof_solutions++);

#if N==8    /* optimized printing, unsafe but works */
    static char out[41] = "[0,0][1,0][2,0][3,0][4,0][5,0][6,0][7,0]";

    out[3] = board_state[0];
    out[8] = board_state[1];
    out[13] = board_state[2];
    out[18] = board_state[3];
    out[23] = board_state[4];
    out[28] = board_state[5];
    out[33] = board_state[6];
    out[38] = board_state[7];

    puts(out);
#else
    int i;
    for (i = 0; i < N; i++) {
        printf("[%d][%c] ", i, board_state[i]);
    }
#endif
}

/* printer/consumer handler function */
void* Printer(void* null) {
    while (1) {
        pthread_mutex_lock(&buf_lock);
        while (empty) {
            if (producers_done) { /* work done, return to main */
                pthread_mutex_unlock(&buf_lock);
                return NULL;
            }
            pthread_cond_wait(&buf_full,&buf_lock); /* waits for buffer to be full */
        }
        if (!empty) {
            while (1) {
                buf_pos--;
                print_board_state(print_buf[buf_pos]);

                if (buf_pos == 0) {
                    empty = true;
                    full = false;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&buf_lock);
        pthread_cond_broadcast(&buf_empty);
    }
    return NULL;
}
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

/* resets state of global flags and counters in between runs */
void reset_state(){
#ifndef DISABLE_PRINTING
    nof_solutions = 1;
    producers_done = false;
    empty = true;
    full = false;
#endif
    task = 0;
}
int main(){
    double start_time, end_time;                /* start and end times for individual runs */
    double total_start_time, total_end_time;    /* total start and end times */

    long l;
    int i;
    int runs;
    int nof_producers;

    if (N >= MAXWORKERS){           /* assures proper amount of producers are created */
                                    /* with current implementation more than N amount of producers are redundant */
#ifndef DISABLE_PRINTING
        nof_producers = MAXWORKERS-1;
#else
        nof_producers = MAXWORKERS;
#endif

    } else {
        nof_producers = 1;
    }
    pthread_attr_t attr;
    pthread_t producers[nof_producers];


#ifndef DISABLE_PRINTING
    pthread_t printer;

    pthread_cond_init (&buf_empty, NULL);
    pthread_cond_init (&buf_full, NULL);
#endif


    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&work_lock, NULL);
    pthread_mutex_init(&buf_lock, NULL);


#if NOF_RUNS > 1
    total_start_time = read_timer();
#endif

    for (runs = 0; runs < NOF_RUNS; runs++) {

        start_time = read_timer();

        /* create producer threads */
        for (l = 0; l < nof_producers; l++)
            pthread_create(&producers[l], &attr, Producer, NULL);

#ifndef DISABLE_PRINTING
        pthread_create(&printer, &attr, Printer, NULL); /* no real point in creating several printer/consumer threads */
#endif                                                  /* since only one thread can print at once */

        for (i = 0; i < nof_producers; i++)
            pthread_join(producers[i], NULL);


#ifndef DISABLE_PRINTING
        producers_done = true;              /* tells printer thread to wrap up work */
        pthread_cond_broadcast(&buf_full);  /* prevents deadlocking */
        pthread_join(printer, NULL);
#endif
        end_time = read_timer();

        printf("\nThe execution time for run #%d is %g sec\n", (runs+1), end_time - start_time);

        reset_state();
    }

#if NOF_RUNS > 1
    total_end_time = read_timer();
    printf("\n\nThe total execution time for all %d runs is %g sec\n", NOF_RUNS, total_end_time - total_start_time);
#endif

    return 0;
}


