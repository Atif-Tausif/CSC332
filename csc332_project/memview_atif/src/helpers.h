#ifndef HELPERS_H
#define HELPERS_H

#include <pthread.h>

typedef struct {
    int pid;
    int show_status;
    int show_maps;
    int show_shm;
    int show_sysinfo;
} TaskArgs;

void install_signal_handlers();

int read_process_status(int process_id);
int read_process_maps(int process_id);
int read_system_meminfo();
int read_shared_memory();

void* thread_worker(void *args);

#endif