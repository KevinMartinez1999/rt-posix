/**
 * @file mars_pahtfinder_sched.c
 * @brief Ejemplo de planificación de tareas con prioridades fijas
 * @details Este programa es un ejemplo de planificación de tareas con prioridades fijas.
 * Se crean 5 hilos (threads) con prioridades fijas y se les asignan tareas con periodos y
 * tiempos de ejecución fijos. Se utiliza la política de planificación FIFO (First In First Out)
 * para la asignación de los hilos a los procesadores.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

#include "periodic_settings.h"
#include "task_definition.h"

#define NUM_TAREAS 5

int resource = 0; // Recurso compartido

pthread_mutexattr_t mymutexattr;
pthread_mutex_t mymutex1, mymutex2;

void *thread1(void *pt);
void *thread2(void *pt);
void *thread3(void *pt);
void *thread4(void *pt);
void *thread5(void *pt);

int main(int argc, char *argv[])
{
    pthread_t tareas[NUM_TAREAS];
    pthread_attr_t my_attr;

    struct sched_param sched_bus_priority, data_priority;
    struct sched_param control_priority, radio_priority, video_priority;

    // Inicializar prioridades de los hilos (threads)
    sched_bus_priority.sched_priority = 90;
    data_priority.sched_priority = 89;
    control_priority.sched_priority = 88;
    radio_priority.sched_priority = 87;
    video_priority.sched_priority = 86;

    // Inicializar atributos de planeación de los hilos (threads)
    pthread_attr_init(&my_attr);
    pthread_mutexattr_init(&mymutexattr);
    pthread_mutexattr_setprotocol(&mymutexattr, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&mymutex1, &mymutexattr);
    pthread_mutex_init(&mymutex2, &mymutexattr);

    // Setear prioridades de los hilos (threads)
    int rc = pthread_attr_setschedpolicy(&my_attr, SCHED_FIFO);
    if (rc != 0)
    {
        perror("Error al setear la política de planificación");
        return -1;
    }

    struct PeriodicThread *sched_bus = malloc(sizeof(struct PeriodicThread));
    struct PeriodicThread *data = malloc(sizeof(struct PeriodicThread));
    struct PeriodicThread *control = malloc(sizeof(struct PeriodicThread));
    struct PeriodicThread *radio = malloc(sizeof(struct PeriodicThread));
    struct PeriodicThread *video = malloc(sizeof(struct PeriodicThread));

    sched_bus->thread_id = 1;
    data->thread_id = 2;
    control->thread_id = 3;
    radio->thread_id = 4;
    video->thread_id = 5;

    pthread_attr_setinheritsched(&my_attr, PTHREAD_EXPLICIT_SCHED);

    pthread_attr_setschedparam(&my_attr, &sched_bus_priority);
    int rc1 = pthread_create(&tareas[0], &my_attr, thread1, (void *)sched_bus);

    pthread_attr_setschedparam(&my_attr, &data_priority);
    int rc2 = pthread_create(&tareas[1], &my_attr, thread2, (void *)data);

    pthread_attr_setschedparam(&my_attr, &control_priority);
    int rc3 = pthread_create(&tareas[2], &my_attr, thread3, (void *)control);

    pthread_attr_setschedparam(&my_attr, &radio_priority);
    int rc4 = pthread_create(&tareas[3], &my_attr, thread4, (void *)radio);

    pthread_attr_setschedparam(&my_attr, &video_priority);
    int rc5 = pthread_create(&tareas[4], &my_attr, thread5, (void *)video);

    if (rc1 || rc2 || rc3 || rc4 || rc5)
    {
        printf("ERROR; There was a problem creating a thread.\n");
        exit(-1);
    }

    // Esperar a que los hilos terminen
    for (int i = 0; i < NUM_TAREAS; i++)
    {
        pthread_join(tareas[i], NULL);
    }

    pthread_mutexattr_destroy(&mymutexattr);
    pthread_mutex_destroy(&mymutex1);
    pthread_mutex_destroy(&mymutex2);
    pthread_attr_destroy(&my_attr);

    pthread_exit(NULL);

    return 0;
}

void *thread1(void *pt)
{
    char message[256];
    struct timespec now;
    struct PeriodicThread *temp = (struct PeriodicThread *)pt;

    temp->period = 25000;
    temp->offset = 0;
    temp->wcet = 5000;

    start_periodic_timer(temp);

    while (1)
    {
        wait_next_activation(temp);
        sched_bus_task(temp->wcet);

        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_cmp(&now, &temp->r) > 0)
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline missed: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
        else
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline    met: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
    }
}

void *thread2(void *pt)
{
    char message[256];
    struct timespec now;
    struct PeriodicThread *temp = (struct PeriodicThread *)pt;

    temp->period = 25000;
    temp->offset = 0;
    temp->wcet = 5000;

    start_periodic_timer(temp);

    while (1)
    {
        wait_next_activation(temp);
        data_task(temp->wcet);

        clock_gettime(CLOCK_REALTIME, &now);
        snprintf(message, sizeof(message), "Thread %d: Accessing resource: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
        log_message(message, "log_file.log");

        pthread_mutex_lock(&mymutex1);
        int i;
        while (i < 100000)
        {
            i++;
        }
        resource++;
        pthread_mutex_unlock(&mymutex1);

        clock_gettime(CLOCK_REALTIME, &now);
        snprintf(message, sizeof(message), "Thread %d: Releasing resource: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
        log_message(message, "log_file.log");

        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_cmp(&now, &temp->r) > 0)
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline missed: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
        else
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline    met: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
    }
}

void *thread3(void *pt)
{
    char message[256];
    struct timespec now;
    struct PeriodicThread *temp = (struct PeriodicThread *)pt;

    temp->period = 50000;
    temp->offset = 0;
    temp->wcet = 5000;

    start_periodic_timer(temp);

    while (1)
    {
        wait_next_activation(temp);
        control_task(temp->wcet);

        clock_gettime(CLOCK_REALTIME, &now);
        snprintf(message, sizeof(message), "Thread %d: Accessing resource: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
        log_message(message, "log_file.log");

        pthread_mutex_lock(&mymutex2);
        int i;
        while (i < 100000)
        {
            i++;
        }
        resource++;
        pthread_mutex_unlock(&mymutex2);

        clock_gettime(CLOCK_REALTIME, &now);
        snprintf(message, sizeof(message), "Thread %d: Releasing resource: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
        log_message(message, "log_file.log");

        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_cmp(&now, &temp->r) > 0)
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline missed: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
        else
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline    met: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
    }
}

void *thread4(void *pt)
{
    char message[256];
    struct timespec now;
    struct PeriodicThread *temp = (struct PeriodicThread *)pt;

    temp->period = 50000;
    temp->offset = 0;
    temp->wcet = 5000;

    start_periodic_timer(temp);

    while (1)
    {
        wait_next_activation(temp);
        radio_task(temp->wcet);

        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_cmp(&now, &temp->r) > 0)
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline missed: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
        else
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline    met: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
    }
}

void *thread5(void *pt)
{
    char message[256];
    struct timespec now;
    struct PeriodicThread *temp = (struct PeriodicThread *)pt;

    temp->period = 50000;
    temp->offset = 0;
    temp->wcet = 5000;

    start_periodic_timer(temp);

    while (1)
    {
        wait_next_activation(temp);
        video_task(temp->wcet);

        clock_gettime(CLOCK_REALTIME, &now);
        if (timespec_cmp(&now, &temp->r) > 0)
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline missed: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
        else
        {
            snprintf(message, sizeof(message), "Thread %d: Deadline    met: now: %ld sec %ld nsec    next: %ld sec %ld nsec", temp->thread_id, now.tv_sec, now.tv_nsec, temp->r.tv_sec, temp->r.tv_nsec);
            log_message(message, "log_file.log");
        }
    }
}
