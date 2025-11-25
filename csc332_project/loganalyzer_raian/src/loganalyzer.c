#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

static volatile sig_atomic_t stop_requested = 0;

// Date filter modes
#define DATE_NONE   0
#define DATE_ON     1
#define DATE_BEFORE 2
#define DATE_AFTER  3

// Structure to store statistics about the log file
struct LogStats {
    long total_lines;
    long error_lines;
    long warning_lines;
    long info_lines;
    long pattern_matches;
};

static int date_mode = DATE_NONE;
static char date_filter[11]; // "YYYY-MM-DD" + '\0'

static int filter_error = 0;   // -E
static int filter_warning = 0; // -W
static int filter_info = 0;    // -I

static int print_matching_lines = 0; // -p: print each line that pass filters

// Function called when the user presses Ctrl+C
void handle_sigint(int sig) {
    (void)sig; 
    stop_requested = 1;
}

// Trim trailing newline 
static void trim_newline(char *line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

// Check if a line matches the current date filter
static int line_matches_date_filter(const char *line) {
    if (date_mode == DATE_NONE) {
        return 1; // no date filtering
    }

    if (strlen(line) < 10) {
        return 0;
    }

    char line_date[11];
    memcpy(line_date, line, 10);
    line_date[10] = '\0';

    int cmp = strcmp(line_date, date_filter);

    if (date_mode == DATE_ON) {
        return (cmp == 0);
    } else if (date_mode == DATE_BEFORE) {
        return (cmp < 0);
    } else if (date_mode == DATE_AFTER) {
        return (cmp > 0);
    }

    return 1;
}

static void process_line(const char *line, struct LogStats *stats, const char *pattern) {
    // Check date filter
    if (!line_matches_date_filter(line)) {
        return; 
    }

    // Detect log levels in this line
    int has_error = 0;
    int has_warning = 0;
    int has_info = 0;

    if (strstr(line, "ERROR") != NULL) {
        has_error = 1;
    }
    if (strstr(line, "WARNING") != NULL) {
        has_warning = 1;
    }
    if (strstr(line, "INFO") != NULL) {
        has_info = 1;
    }

    int any_level_filter = filter_error || filter_warning || filter_info;
    if (any_level_filter) {
        int matches_selected_level = 0;

        if (filter_error && has_error) {
            matches_selected_level = 1;
        }
        if (filter_warning && has_warning) {
            matches_selected_level = 1;
        }
        if (filter_info && has_info) {
            matches_selected_level = 1;
        }

        if (!matches_selected_level) {
            // Line does not match any of the levels
            return;
        }
    }

    // -p : print the line itself 
    if (print_matching_lines) {
        printf("%s\n", line);
    }

    stats->total_lines++;

    if (has_error) {
        stats->error_lines++;
    }
    if (has_warning) {
        stats->warning_lines++;
    }
    if (has_info) {
        stats->info_lines++;
    }

    if (pattern != NULL && strstr(line, pattern) != NULL) {
        stats->pattern_matches++;
    }
}

// Analyze the log file content stored in memory 
static void analyze_buffer(const char *buf, size_t size,
                           struct LogStats *stats, const char *pattern) {
    size_t line_start = 0;

    for (size_t i = 0; i <= size && !stop_requested; i++) {
        char c;
        if (i == size) {
            // newline at the end of file
            c = '\n';
        } else {
            c = buf[i];
        }

        if (c == '\n') {
            size_t line_len = i - line_start;
            if (line_len > 0) {
                char *line_copy = (char *)malloc(line_len + 1);
                if (line_copy == NULL) {
                    fprintf(stderr, "malloc failed while copying line\n");
                    return;
                }

                memcpy(line_copy, buf + line_start, line_len);
                line_copy[line_len] = '\0';

                trim_newline(line_copy);

                process_line(line_copy, stats, pattern);

                free(line_copy);
            } else {
                // Empty line 
            }

            line_start = i + 1;
        }
    }
}

static void print_usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s -f <logfile> [options]\n"
        "\n"
        "Required:\n"
        "  -f <logfile>      Path to the log file to analyze\n"
        "\n"
        "Optional:\n"
        "  -s <pattern>      Count lines containing this substring\n"
        "  -p                Print each matching log line\n"
        "  -d <YYYY-MM-DD>   Only include lines ON this date\n"
        "  -b <YYYY-MM-DD>   Only include lines BEFORE this date\n"
        "  -a <YYYY-MM-DD>   Only include lines AFTER this date\n"
        "  -E                Only include ERROR lines\n"
        "  -W                Only include WARNING lines\n"
        "  -I                Only include INFO lines\n"
        "\n"
        "Examples:\n"
        "  %s -f master_log.txt\n"
        "  %s -f master_log.txt -s User\n"
        "  %s -f master_log.txt -d 2025-03-01 -E\n"
        "  %s -f master_log.txt -E -p\n",
        progname, progname, progname, progname, progname);
}

int main(int argc, char *argv[]) {
    const char *filename = NULL;
    const char *pattern = NULL;

    // Initialize date_filter to empty string
    date_filter[0] = '\0';

    int opt;
    // Options: f, s, d, b, a, E, W, I, p
    while ((opt = getopt(argc, argv, "f:s:d:b:a:EWIp")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 's':
                pattern = optarg;
                break;
            case 'd':
                date_mode = DATE_ON;
                strncpy(date_filter, optarg, 10);
                date_filter[10] = '\0';
                break;
            case 'b':
                date_mode = DATE_BEFORE;
                strncpy(date_filter, optarg, 10);
                date_filter[10] = '\0';
                break;
            case 'a':
                date_mode = DATE_AFTER;
                strncpy(date_filter, optarg, 10);
                date_filter[10] = '\0';
                break;
            case 'E':
                filter_error = 1;
                break;
            case 'W':
                filter_warning = 1;
                break;
            case 'I':
                filter_info = 1;
                break;
            case 'p':
                print_matching_lines = 1;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Error: log file not specified.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Install signal handler for Ctrl+C
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: cannot open file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "Error: fstat failed on '%s': %s\n", filename, strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "Warning: file '%s' is empty.\n", filename);
        close(fd);
        return EXIT_SUCCESS;
    }

    size_t filesize = (size_t)st.st_size;

    // Map the file into memory
    char *mapped = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        fprintf(stderr, "Error: mmap failed on '%s': %s\n", filename, strerror(errno));
        close(fd);
        return EXIT_FAILURE;
    }

    struct LogStats stats;
    memset(&stats, 0, sizeof(stats));

    analyze_buffer(mapped, filesize, &stats, pattern);

    if (munmap(mapped, filesize) == -1) {
        fprintf(stderr, "Warning: munmap failed: %s\n", strerror(errno));
    }
    close(fd);

    printf("Total lines            : %ld\n", stats.total_lines);
    printf("Lines with 'ERROR'     : %ld\n", stats.error_lines);
    printf("Lines with 'WARNING'   : %ld\n", stats.warning_lines);
    printf("Lines with 'INFO'      : %ld\n", stats.info_lines);

    if (pattern != NULL) {
        printf("Lines with '%s' : %ld\n", pattern, stats.pattern_matches);
    }

    return EXIT_SUCCESS;
}
