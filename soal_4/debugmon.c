#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/stat.h>

int isnum(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

uid_t uid_username(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (!pw) {
        fprintf(stderr, "User not found");
        return -1;
    }
    return pw->pw_uid;
}

uid_t uid_process(const char *pid) {
    char path[100];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *fp = fopen(path, "r");
    if (!fp){
        return -1;
    }

    uid_t uid = -1;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line + 5, "%d", &uid);
            break;
        }
    }
    fclose(fp);
    return uid;
}

void get_command(char *pid, char *command) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/comm", pid);

    FILE *fp = fopen(path, "r");
    if (fp) {
        if (fgets(command, sizeof(command), fp) == NULL) {
            strcpy(command, " ");
        }
        fclose(fp);
    } else {
        strcpy(command, " ");
    }

    command[strcspn(command, "\n")] = '\0';
}

float get_cpu(const char *pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE *fp = fopen(path, "r");
    float cpu = 0;

    if (fp) {
        unsigned long usert, systemt;
        for (int i = 1; i <= 13; i++) fscanf(fp, "%*s");
        fscanf(fp, "%lu %lu", &usert, &systemt);
        fclose(fp);
        cpu = (usert + systemt) / (float)sysconf(_SC_CLK_TCK);
    }
    return cpu;
}

float get_memory(const char *pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *fp = fopen(path, "r");
    float memory = 0;

    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line + 6, "%f", &memory);
                break;
            }
        }
        fclose(fp);
    }
    return memory;
}

void file_log(const char *username, const char *process, const char *status) {
    FILE *fp = fopen("/tmp/debugmon.log", "a");
    if (!fp) {
        perror("Failed open file");
        return;
    }
    
    time_t now = time(NULL);
    struct tm *waktu = localtime(&now);
    fprintf(fp, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s_%s\n", waktu->tm_mday, waktu->tm_mon + 1, waktu->tm_year + 1900, waktu->tm_hour, waktu->tm_min, waktu->tm_sec, username, process, status);
    fclose(fp);
}

void list(const char *username) {
    uid_t uid_target = uid_username(username);
    if (uid_target == -1) return;
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Failed open directory");
        return;
    }

    printf("PID\tCOMMAND\t\tCPU Usage\tMEMORY Usage\n");

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (isnum(dp->d_name)) {
            uid_t process_uid = uid_process(dp->d_name);
            if (process_uid == uid_target) {
                char command[256];
                get_command(dp->d_name, command);

                float cpu = get_cpu(dp->d_name);
                float memory = get_memory(dp->d_name);

                printf("%s\t%s\t\t%.2f\t\t%.2f kb\n", dp->d_name, command, cpu, memory);
            }
        }
    }
    closedir(dir);
}

void Daemon(const char *username) {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    umask(0);

    pid_t sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    FILE *fp = fopen("/tmp/daemon.pid", "w");
    if (fp) {
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }

    while (1) {
        file_log(username, "DAEMON", "RUNNING");
        sleep(30); 
    }
}

void stop(const char *username) {
    FILE *fp = fopen("/tmp/daemon.pid", "r");
    if (!fp) {
        perror("Failed open file");
        return;
    }

    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fprintf(stderr, "Failed to read PID\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        file_log(username, "STOP", "RUNNING");
        remove("/tmp/debugmon_daemon.pid");
    } else {
        perror("Failed stop daemon");
    }
}

void fail(const char *username) {
    uid_t uid_target = uid_username(username);
    if (uid_target == -1){
        return;
    }

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Failed open directory");
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (isnum(dp->d_name)) {
            uid_t process_uid = uid_process(dp->d_name);
            if (process_uid == uid_target) {
                char proc_name[256];
                get_command(dp->d_name, proc_name);
                if (strcmp(proc_name, "debugmon") != 0) {
                    pid_t pid = atoi(dp->d_name);
                    if (kill(pid, SIGSTOP) == 0) {
                        file_log(username, proc_name, "FAILED");
                    }
                }
            }
        }
    }
    closedir(dir);
}

void revert(const char *username) {
    uid_t uid_target = uid_username(username);
    if (uid_target == -1){
        return;
    }

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Failed open directory");
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (isnum(dp->d_name)) {
            uid_t process_uid = uid_process(dp->d_name);
            if (process_uid == uid_target) {
                char proc_name[256];
                get_command(dp->d_name, proc_name);
                if (strcmp(proc_name, "debugmon") != 0) {
                    pid_t pid = atoi(dp->d_name);
                    if (kill(pid, SIGCONT) == 0) {
                        file_log(username, proc_name, "RUNNING");
                    }
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Command's unavailable:\n");
        printf("Usage: ./nama_file <command> <user>\n");
        printf("Available command:\n");
        printf("  list\n");
        printf("  Daemon\n");
        printf("  stop\n");
        printf("  fail\n");
        printf("  revert\n");
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list(argv[2]);
    }
    else if (strcmp(argv[1], "daemon") == 0) {
        Daemon(argv[2]);
    }
    else if (strcmp(argv[1], "stop") == 0) {
        stop(argv[2]);
    }
    else if (strcmp(argv[1], "fail") == 0) {
        fail(argv[2]);
    }
    else if (strcmp(argv[1], "revert") == 0) {
        revert(argv[2]);
    }
    else {
        printf("Unvalid Command\n");
        return 1;
    }

    return 0;
}
