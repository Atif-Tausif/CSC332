// filediffadvanced.c
// Advanced diff tool: binary-safe compare with statistics using mmap().
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <ctype.h>

typedef struct {
    int brief;
    int summary;
    int text_mode;     
    size_t max_report; 
} diff_options;

typedef struct {
    off_t offset;
    unsigned char b1;
    unsigned char b2;
    int line;  
    int col;  
} diff_entry;

//Simple SIGINT handler

static void handle_sigint(int sig) {
    (void)sig;
    const char msg[] = "\nfilediffadvanced: interrupted by SIGINT\n";
    ssize_t ignored = write(STDERR_FILENO, msg, sizeof(msg) - 1);
    (void)ignored; 
    _exit(130);
}



static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [OPTIONS] file1 file2\n"
        "Compare two files using mmap() and show binary differences.\n\n"
        "Options:\n"
        "  -b, --brief        Only report whether files differ (no details)\n"
        "  -s, --summary      Print summary statistics (default)\n"
        "  -t, --text         Show textual info (line/column and characters) for differences\n"
        "  -o, --offset N     Show at most N differing positions (default 10)\n"
        "  -h, --help         Show this help message\n",
        prog);
}

static int parse_options(int argc, char *argv[], diff_options *opt,
                         int *file_index) {
    static struct option long_opts[] = {
        {"brief",   no_argument,       0, 'b'},
        {"summary", no_argument,       0, 's'},
        {"text",    no_argument,       0, 't'},
        {"offset",  required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    opt->brief = 0;
    opt->summary = 0;
    opt->text_mode = 0;
    opt->max_report = 10; // default

    int c;
    while ((c = getopt_long(argc, argv, "bsto:h", long_opts, NULL)) != -1) {
        switch (c) {
        case 'b':
            opt->brief = 1;
            break;
        case 's':
            opt->summary = 1;
            break;
        case 't':
            opt->text_mode = 1;
            break;
        case 'o': {
            long v = strtol(optarg, NULL, 10);
            if (v <= 0) {
                fprintf(stderr, "Invalid value for --offset: %s\n", optarg);
                return -1;
            }
            opt->max_report = (size_t)v;
            break;
        }
        case 'h':
            print_usage(argv[0]);
            exit(0);
        default:
            print_usage(argv[0]);
            return -1;
        }
    }

    // default to summary
    if (!opt->brief && !opt->summary) {
        opt->summary = 1;
    }

    *file_index = optind;
    return 0;
}

//comparison using mmap()
static int diff_files(const char *path1, const char *path2,
                      const diff_options *opt) {
    int fd1 = -1, fd2 = -1;
    struct stat st1, st2;
    unsigned char *map1 = NULL;
    unsigned char *map2 = NULL;
    diff_entry *entries = NULL;
    off_t size1 = 0, size2 = 0;

    fd1 = open(path1, O_RDONLY);
    if (fd1 == -1) {
        perror("open file1");
        goto error;
    }

    fd2 = open(path2, O_RDONLY);
    if (fd2 == -1) {
        perror("open file2");
        goto error;
    }

    if (fstat(fd1, &st1) == -1) {
        perror("fstat file1");
        goto error;
    }
    if (fstat(fd2, &st2) == -1) {
        perror("fstat file2");
        goto error;
    }

    size1 = st1.st_size;
    size2 = st2.st_size;

    if (size1 > 0) {
        map1 = mmap(NULL, size1, PROT_READ, MAP_PRIVATE, fd1, 0);
        if (map1 == MAP_FAILED) {
            perror("mmap file1");
            map1 = NULL;
            goto error;
        }
    }
    if (size2 > 0) {
        map2 = mmap(NULL, size2, PROT_READ, MAP_PRIVATE, fd2, 0);
        if (map2 == MAP_FAILED) {
            perror("mmap file2");
            map2 = NULL;
            goto error;
        }
    }

    off_t min_size = (size1 < size2) ? size1 : size2;
    off_t max_size = (size1 > size2) ? size1 : size2;

    if (opt->max_report > 0) {
        entries = malloc(opt->max_report * sizeof(diff_entry));
        if (!entries) {
            perror("malloc");
            goto error;
        }
    }

    off_t diff_bytes = 0;
    size_t stored = 0;

    // Performance timing
    struct timespec start_ts, end_ts;
    if (clock_gettime(CLOCK_MONOTONIC, &start_ts) != 0) {
        perror("clock_gettime start");
        goto error;
    }

    // For text mode: track line/column in file1
    int line = 1;
    int col  = 1;

    // Compare common entries
    for (off_t i = 0; i < min_size; ++i) {
        if (map1 && map2 && map1[i] != map2[i]) {
            diff_bytes++;
            if (stored < opt->max_report && entries) {
                entries[stored].offset = i;
                entries[stored].b1 = map1[i];
                entries[stored].b2 = map2[i];
                entries[stored].line = line;
                entries[stored].col = col;
                stored++;
            }
        }

        // Next line/column based on file1 (for text mode)
        if (map1) {
            if (map1[i] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
        }
    }

    // Extra bytes in longer file are also differences
    diff_bytes += (max_size - min_size);

    if (clock_gettime(CLOCK_MONOTONIC, &end_ts) != 0) {
        perror("clock_gettime end");
        goto error;
    }

    // Compute elapsed time in seconds
    double elapsed_sec =
        (double)(end_ts.tv_sec - start_ts.tv_sec) +
        (double)(end_ts.tv_nsec - start_ts.tv_nsec) / 1e9;
    if (elapsed_sec < 0.0) {
        elapsed_sec = 0.0;
    }

    int identical = (diff_bytes == 0);

    if (!opt->brief) {
        if (opt->summary) {
            printf("file1: %s (%lld bytes)\n",
                   path1, (long long)size1);
            printf("file2: %s (%lld bytes)\n",
                   path2, (long long)size2);
            if (identical) {
                printf("Result: files are identical.\n");
            } else {
                double total = (double)max_size;
                double percent = (total > 0.0)
                                 ? (100.0 * (double)diff_bytes / total)
                                 : 0.0;
                printf("Differing bytes: %lld (%.2f%% of larger file)\n",
                       (long long)diff_bytes, percent);
            }

            // Performance statistics
            double compared_bytes = (double)max_size;
            double mb = compared_bytes / (1024.0 * 1024.0);
            double throughput = 0.0;
            if (elapsed_sec > 0.0) {
                throughput = mb / elapsed_sec;
            }

            printf("Comparison time: %.3f ms\n", elapsed_sec * 1000.0);
            printf("Bytes compared:  %lld (%.3f MB)\n",
                   (long long)max_size, mb);
            printf("Throughput:       %.3f MB/s\n", throughput);
            printf("\n");
        }

        if (!identical && stored > 0) {
            printf("First %zu differing positions:\n", stored);
            for (size_t i = 0; i < stored; ++i) {
                const diff_entry *e = &entries[i];
                if (opt->text_mode) {
                    char c1 = isprint(e->b1) ? (char)e->b1 : '.';
                    char c2 = isprint(e->b2) ? (char)e->b2 : '.';
                    printf("  offset %lld, line %d, col %d: "
                           "0x%02X ('%c') != 0x%02X ('%c')\n",
                           (long long)e->offset,
                           e->line, e->col,
                           e->b1, c1,
                           e->b2, c2);
                } else {
                    printf("  %lld: 0x%02X != 0x%02X\n",
                           (long long)e->offset,
                           e->b1, e->b2);
                }
            }
        }
    } else {
        if (identical) {
            printf("Files are identical.\n");
        } else {
            printf("Files differ.\n");
        }
    }

    // Cleanup
    free(entries);
    if (map1 && size1 > 0) munmap(map1, size1);
    if (map2 && size2 > 0) munmap(map2, size2);
    if (fd1 != -1) close(fd1);
    if (fd2 != -1) close(fd2);

    return identical ? 0 : 1;

error:
    if (entries) free(entries);
    if (map1 && size1 > 0) munmap(map1, size1);
    if (map2 && size2 > 0) munmap(map2, size2);
    if (fd1 != -1) close(fd1);
    if (fd2 != -1) close(fd2);
    return 2;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    diff_options opt;
    int file_index;

    if (parse_options(argc, argv, &opt, &file_index) != 0) {
        return 2;
    }

    if (argc - file_index != 2) {
        fprintf(stderr, "Error: exactly two file names are required.\n\n");
        print_usage(argv[0]);
        return 2;
    }

    const char *file1 = argv[file_index];
    const char *file2 = argv[file_index + 1];

    return diff_files(file1, file2, &opt);
}