#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>

#include "helpers.h"

void display_usage() {
    printf(
        "Usage: memview [OPTIONS]\n"
        "Options:\n"
        "  -p, --pid <pid>        Display memory details for a process\n"
        "  -m, --maps             Display memory map layout\n"
        "  -s, --system           Display system-wide memory stats\n"
        "  -S, --shared           Display shared memory segments\n"
        "  -t, --threads          Run operations in a separate thread\n"
        "  -h, --help             Display this help\n"
    );
}

int main(int argc, char *argv[]) {
    install_signal_handlers();

    int process_id = -1;
    int display_status = 0;
    int display_memory_map = 0;
    int display_system_info = 0;
    int display_shared_mem = 0;
    int run_threaded = 0;

    const struct option long_options[] = {
        {"pid", required_argument, NULL, 'p'},
        {"maps", no_argument, NULL, 'm'},
        {"system", no_argument, NULL, 's'},
        {"shared", no_argument, NULL, 'S'},
        {"threads", no_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    int option;
    while ((option = getopt_long(argc, argv, "p:msSth", long_options, NULL)) != -1) {
        switch (option) {
            case 'p':
                process_id = atoi(optarg);
                display_status = 1;
                break;
            case 'm':
                display_memory_map = 1;
                break;
            case 's':
                display_system_info = 1;
                break;
            case 'S':
                display_shared_mem = 1;
                break;
            case 't':
                run_threaded = 1;
                break;
            case 'h':
                display_usage();
                return 0;
            default:
                display_usage();
                return 1;
        }
    }

    if ((display_memory_map || display_status) && process_id < 0) {
        fprintf(stderr, "Hey, you need to specify a PID with -p when using -m\n");
        return 1;
    }

    TaskArgs arguments = {
        .pid = process_id,
        .show_status = display_status,
        .show_maps = display_memory_map,
        .show_shm = display_shared_mem,
        .show_sysinfo = display_system_info
    };

    if (run_threaded) {
        pthread_t worker_thread;
        if (pthread_create(&worker_thread, NULL, thread_worker, &arguments) != 0) {
            fprintf(stderr, "Couldn't create worker thread\n");
            return 1;
        }
        pthread_join(worker_thread, NULL);
        return 0;
    }

    if (display_system_info)
        read_system_meminfo();

    if (display_status)
        read_process_status(process_id);

    if (display_memory_map)
        read_process_maps(process_id);

    if (display_shared_mem)
        read_shared_memory();

    return 0;
}