// timedexec.c
// Run a command with a wall-clock time limit.
// Usage:
//   timedexec -t SECONDS -- command [args...]

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

static pid_t child_pid = -1;  // child process ID

/* Handle SIGALRM when time limit expires */
static void alarm_handler(int sig)
{
    (void)sig;                 // unused parameter
    if (child_pid > 0) {
        kill(child_pid, SIGKILL);  // forcefully kill child
    }
}

/* Handle Ctrl-C so parent also kills child */
static void sigint_handler(int sig)
{
    (void)sig;
    if (child_pid > 0) {
        kill(child_pid, SIGKILL);
    }
    write(STDERR_FILENO, "\ntimedexec: interrupted by user\n", 32);
    _exit(1);
}

/* Print usage help */
static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s -t SECONDS -- command [args...]\n"
        "  -t SECONDS : required time limit\n",
        prog);
}

int main(int argc, char *argv[])
{
    int opt;
    long time_limit = -1;  // time limit in seconds

    // --- command-line parsing and validation (like professor's example) ---
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            time_limit = strtol(optarg, NULL, 10);
            if (time_limit <= 0) {
                fprintf(stderr, "Invalid time limit: %s\n", optarg);
                return 1;
            }
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (time_limit <= 0) {
        fprintf(stderr, "Error: -t SECONDS is required.\n");
        usage(argv[0]);
        return 1;
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: no command specified.\n");
        usage(argv[0]);
        return 1;
    }

    char **cmd_argv = &argv[optind];

    // --- install signal handlers (professor-style, simple error checks) ---
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction(SIGALRM)");
        return 1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction(SIGINT)");
        return 1;
    }

    // --- create child (like open() in the snippet: check, perror, return 1) ---
    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return 1;
    }

    if (child_pid == 0) {
        // Child: replace with requested command
        execvp(cmd_argv[0], cmd_argv);
        // Only reached on error
        perror("execvp");
        _exit(127);
    }

    // Parent: start the timer
    alarm((unsigned int)time_limit);

    // Wait for child; handle EINTR if a signal arrives
    int status;
    for (;;) {
        int r = waitpid(child_pid, &status, 0);
        if (r == -1) {
            if (errno == EINTR) {
                continue;      // interrupted by signal, try again
            }
            perror("waitpid");
            return 1;
        }
        break;                 // child reaped successfully
    }

    // Disable any pending alarm (not strictly required but nice)
    alarm(0);

    // --- interpret child's termination status ---
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);

        if (sig == SIGKILL) {
            fprintf(stderr,
                "timedexec: process killed (time limit exceeded).\n");
            return 124;        // timeout-style exit code
        }

        printf("timedexec: process killed by signal %d\n", sig);
        return 128 + sig;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("timedexec: process exited with code %d\n", code);
        return code;
    }

    printf("timedexec: unknown termination.\n");
    return 1;
}