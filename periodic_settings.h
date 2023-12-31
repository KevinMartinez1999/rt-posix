#ifndef PERIODIC_SETTINGS_H
#define PERIODIC_SETTINGS_H

struct PeriodicThread
{
    struct timespec r;
    uint16_t id;
    uint32_t period;
    uint32_t offset;
    uint32_t wcet;
};

void current_time();
void timespec_add_us(struct timespec *t, uint32_t us);
int timespec_cmp(struct timespec *a, struct timespec *b);
void start_periodic_timer(struct PeriodicThread *pt);
void wait_next_activation(struct PeriodicThread *pt);
void log_message(char *message, char *filename);

#endif