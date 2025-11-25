#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

static void handle_interrupt(int signal_num) {
    const char *interrupt_msg = "Caught interrupt signal, cleaning up...\n";
    write(STDERR_FILENO, interrupt_msg, strlen(interrupt_msg));
}

void install_signal_handlers() {
    signal(SIGINT, handle_interrupt);
    signal(SIGTERM, handle_interrupt);
}

int read_process_status(int process_id) {
    char file_path[64];
    snprintf(file_path, sizeof(file_path), "/proc/%d/status", process_id);
    
    int file_desc = open(file_path, O_RDONLY);
    if (file_desc == -1) {
        fprintf(stderr, "Couldn't open %s: %s\n", file_path, strerror(errno));
        return -1;
    }
    
    char data_buffer[4096];
    int bytes_read = read(file_desc, data_buffer, sizeof(data_buffer) - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "Failed to read %s: %s\n", file_path, strerror(errno));
        close(file_desc);
        return -1;
    }
    data_buffer[bytes_read] = '\0';
    
    printf("\n---- Process Status (PID: %d) ----\n", process_id);
    printf("%s\n", data_buffer);
    
    close(file_desc);
    return 0;
}

int read_process_maps(int process_id) {
    char file_path[64];
    snprintf(file_path, sizeof(file_path), "/proc/%d/maps", process_id);
    
    int file_desc = open(file_path, O_RDONLY);
    if (file_desc == -1) {
        fprintf(stderr, "Couldn't open %s: %s\n", file_path, strerror(errno));
        return -1;
    }
    
    struct stat file_info;
    if (fstat(file_desc, &file_info) < 0) {
        fprintf(stderr, "Couldn't get file info for %s: %s\n", file_path, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    size_t file_size = file_info.st_size;
    char *mapped_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_desc, 0);
    
    if (mapped_data == MAP_FAILED) {
        fprintf(stderr, "Memory mapping failed for %s: %s\n", file_path, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    printf("\n---- Memory Map Layout (PID: %d) ----\n", process_id);
    write(STDOUT_FILENO, mapped_data, file_size);
    
    munmap(mapped_data, file_size);
    close(file_desc);
    return 0;
}

int read_system_meminfo() {
    const char *meminfo_path = "/proc/meminfo";
    
    int file_desc = open(meminfo_path, O_RDONLY);
    if (file_desc == -1) {
        fprintf(stderr, "Couldn't open %s: %s\n", meminfo_path, strerror(errno));
        return -1;
    }
    
    char data_buffer[4096];
    int bytes_read = read(file_desc, data_buffer, sizeof(data_buffer) - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "Failed to read %s: %s\n", meminfo_path, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    data_buffer[bytes_read] = '\0';
    
    printf("\n---- System-Wide Memory Stats ----\n");
    printf("%s\n", data_buffer);
    
    close(file_desc);
    return 0;
}

int read_shared_memory() {
    const char *shm_path = "/proc/sysvipc/shm";
    
    int file_desc = open(shm_path, O_RDONLY);
    if (file_desc == -1) {
        fprintf(stderr, "Couldn't open %s: %s\n", shm_path, strerror(errno));
        return -1;
    }
    
    char data_buffer[4096];
    int bytes_read = read(file_desc, data_buffer, sizeof(data_buffer) - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "Failed to read %s: %s\n", shm_path, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    data_buffer[bytes_read] = '\0';
    
    printf("\n---- Shared Memory Segments ----\n");
    printf("%s\n", data_buffer);
    
    close(file_desc);
    return 0;
}

void* thread_worker(void *args) {
    TaskArgs *job = (TaskArgs*)args;
    
    if (job->show_sysinfo)
        read_system_meminfo();
    
    if (job->show_status)
        read_process_status(job->pid);
    
    if (job->show_maps)
        read_process_maps(job->pid);
    
    if (job->show_shm)
        read_shared_memory();
    
    return NULL;
}