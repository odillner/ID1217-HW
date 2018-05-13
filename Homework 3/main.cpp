#include <iostream>
#include <semaphore.h>
#include <unistd.h>

#define NOF_FEMALES 3
#define NOF_MALES 3

sem_t print_lock;       /* prevents threads from printing at the same time */
sem_t queue_lock[2];    /* prevents threads from entering queue before it's empty */
sem_t buf_lock[2];      /* prevents threads from entering queue before it's empty */
sem_t room_lock;        /* prevents other gender from entering bathroom */

int queue_count[2];     /* amount of workers in queue */
int bathroom_count;     /* amount of workers in bathroom */


void enter_bathroom(bool gender) {

    sleep(rand() % 1 + 1);

    sem_wait(&print_lock);
    if (gender) {
        std::cout << "Thread with female gender has entered the bathroom" <<  std::endl;
    } else {
        std::cout << "Thread with male gender has entered the bathroom" <<  std::endl;
    }
    sem_post(&print_lock);


}

void* worker(void* arg) {

    int gender = (int) arg;

    while (true) {
        sleep(rand() % 2 + 1);

        queue_count[gender]++;

        sem_wait(&queue_lock[gender]);
        sem_post(&queue_lock[gender]);


        enter_bathroom(gender);

        bathroom_count++;

        if (bathroom_count == queue_count[gender]) { /* last person in queue has exited bathroom */
            bathroom_count = 0;
            queue_count[gender] = 0;

            sem_wait(&queue_lock[gender]);    /* lock queue exit */
            sem_post(&buf_lock[gender]);      /* unlock queue enter */

            sem_post(&queue_lock[!gender]); /* tell other gender to enter bathroom */
        }

        sem_wait(&buf_lock[gender]);
        sem_post(&buf_lock[gender]);

    }

}

int main() {
    int i;

    pthread_t males[NOF_MALES];
    pthread_t females[NOF_FEMALES];


    /* semaphore init */
    sem_init(&print_lock, 0, 1);
    sem_init(&room_lock, 0, 1);

    sem_init(&buf_lock[0], 0, 0);
    sem_init(&buf_lock[1], 0, 0);

    sem_init(&queue_lock[0], 0, 0);
    sem_init(&queue_lock[1], 0, 1);

    for (i = 0; i < NOF_MALES; i++) {
        pthread_create(&males[i], nullptr, worker, (void *) 0);
    }
    for (i = 0; i < NOF_FEMALES; i++) {
        pthread_create(&females[i], nullptr, worker, (void *) 1);
    }

    while(true);

    return 0;

}