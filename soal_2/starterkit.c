#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <linux/limits.h>

#define DOWNLOAD_URL "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download"
#define ZIP_FILE "starterkit.zip"
#define LOG_FILE "activity.log"
#define STARTER_KIT_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define DAEMON_PID_FILE "/tmp/decrypt_daemon.pid"

// Function to write log entry
void write_log(const char *format, ...) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Error opening log file");
        return;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    
    // Format timestamp explicitly to avoid locale issues
    fprintf(log_file, "[%02d-%02d-%04d][%02d:%02d:%02d] - ", 
            time_info->tm_mday, 
            time_info->tm_mon + 1, 
            time_info->tm_year + 1900,
            time_info->tm_hour,
            time_info->tm_min,
            time_info->tm_sec);

    // Handle variable arguments
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fclose(log_file);
}

// Function to check if directory exists
int directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

// Function to create directory if it doesn't exist
void create_directory_if_not_exists(const char *path) {
    if (!directory_exists(path)) {
        if (mkdir(path, 0755) != 0) {
            perror("Error creating directory");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to download and extract starter kit
void download_and_extract() {
    // Check if STARTER_KIT_DIR already exists
    if (directory_exists(STARTER_KIT_DIR)) {
        printf("Starter kit directory already exists. Skipping download.\n");
        return;
    }

    // Download the zip file using curl
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execl("/usr/bin/curl", "curl", "-L", "-o", ZIP_FILE, DOWNLOAD_URL, NULL);
        perror("execl curl failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error downloading file\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Create the starter kit directory
    create_directory_if_not_exists(STARTER_KIT_DIR);

    // Unzip the file
    pid = fork();
    if (pid == 0) {
        // Child process
        execl("/usr/bin/unzip", "unzip", ZIP_FILE, "-d", STARTER_KIT_DIR, NULL);
        perror("execl unzip failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error extracting file\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // Remove the zip file
    if (unlink(ZIP_FILE) != 0) {
        perror("Error removing zip file");
    }

    printf("Successfully downloaded and extracted starter kit\n");
}

// Base64 decoding functions
static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char base64_reverse_table[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// Function to decode Base64 string
char* base64_decode(const char *src, size_t *out_len) {
    size_t src_len = strlen(src);
    size_t target_len = src_len / 4 * 3;
    char *target = malloc(target_len + 1);
    if (!target)
        return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < src_len; i += 4, j += 3) {
        unsigned char a = base64_reverse_table[(unsigned char)src[i]];
        unsigned char b = (i + 1 < src_len) ? base64_reverse_table[(unsigned char)src[i + 1]] : 0;
        unsigned char c = (i + 2 < src_len) ? base64_reverse_table[(unsigned char)src[i + 2]] : 0;
        unsigned char d = (i + 3 < src_len) ? base64_reverse_table[(unsigned char)src[i + 3]] : 0;

        target[j] = (a << 2) | (b >> 4);
        if (i + 2 < src_len)
            target[j + 1] = (b << 4) | (c >> 2);
        if (i + 3 < src_len)
            target[j + 2] = (c << 6) | d;
    }

    *out_len = j;
    target[j] = '\0';
    return target;
}

// Function to check if a string is Base64 encoded
int is_base64_encoded(const char *str) {
    size_t len = strlen(str);
    
    // Basic check: Base64 strings have a length that's a multiple of 4
    if (len % 4 != 0) return 0;
    
    // Check each character is in the Base64 alphabet
    for (size_t i = 0; i < len; i++) {
        if (!((str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= 'a' && str[i] <= 'z') ||
              (str[i] >= '0' && str[i] <= '9') ||
              str[i] == '+' || str[i] == '/' ||
              (i >= len - 2 && str[i] == '='))) {
            return 0;
        }
    }
    
    return 1;
}

// Function to read daemon PID from file
pid_t read_daemon_pid() {
    FILE *pid_file = fopen(DAEMON_PID_FILE, "r");
    if (!pid_file) {
        return -1;  // PID file doesn't exist or can't be read
    }

    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1) {
        fclose(pid_file);
        return -1;  // Failed to read PID
    }

    fclose(pid_file);
    return pid;
}

// Function to write daemon PID to file
void write_daemon_pid(pid_t pid) {
    FILE *pid_file = fopen(DAEMON_PID_FILE, "w");
    if (!pid_file) {
        perror("Failed to create PID file");
        exit(EXIT_FAILURE);
    }

    fprintf(pid_file, "%d", pid);
    fclose(pid_file);
}

// Function to start daemon that decrypts Base64 filenames
void start_decrypt_daemon() {
    // Check if daemon is already running
    if (read_daemon_pid() > 0) {
        printf("Decrypt daemon is already running\n");
        return;
    }

    // Create quarantine directory if it doesn't exist
    create_directory_if_not_exists(QUARANTINE_DIR);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        printf("Started decryption daemon with PID %d\n", pid);
        write_daemon_pid(pid);
        write_log("Successfully started decryption process with PID %d.", pid);
        return;
    }

    // Child process becomes daemon
    // Create a new session to detach from terminal
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // Change working directory to root
    if (chdir("/") < 0) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Redirect to /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    // Main daemon loop
    while (1) {
        DIR *dir = opendir(QUARANTINE_DIR);
        if (!dir) {
            sleep(5);
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && is_base64_encoded(entry->d_name)) {
                // Decode the filename
                size_t decoded_len;
                char *decoded_name = base64_decode(entry->d_name, &decoded_len);
                if (!decoded_name)
                    continue;

                // Build old and new paths
                char old_path[PATH_MAX];
                char new_path[PATH_MAX];
                snprintf(old_path, PATH_MAX, "%s/%s", QUARANTINE_DIR, entry->d_name);
                snprintf(new_path, PATH_MAX, "%s/%s", QUARANTINE_DIR, decoded_name);

                // Rename the file
                if (rename(old_path, new_path) == 0) {
                    // Successfully renamed
                }

                free(decoded_name);
            }
        }

        closedir(dir);
        sleep(5);  // Sleep for 5 seconds before checking again
    }

    // Should never reach here
    exit(EXIT_SUCCESS);
}

// Function to stop daemon
void stop_decrypt_daemon() {
    pid_t daemon_pid = read_daemon_pid();
    if (daemon_pid <= 0) {
        printf("Decrypt daemon is not running\n");
        return;
    }

    // Send SIGTERM to the daemon
    if (kill(daemon_pid, SIGTERM) == 0) {
        printf("Sent shutdown signal to decrypt daemon (PID: %d)\n", daemon_pid);
        write_log("Successfully shut off decryption process with PID %d.", daemon_pid);
        
        // Remove PID file
        if (unlink(DAEMON_PID_FILE) != 0) {
            perror("Failed to remove PID file");
        }
    } else {
        perror("Failed to send signal to daemon");
    }
}

// Function to move file between directories
void move_file(const char *src_dir, const char *dst_dir, const char *filename) {
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];
    
    snprintf(src_path, PATH_MAX, "%s/%s", src_dir, filename);
    snprintf(dst_path, PATH_MAX, "%s/%s", dst_dir, filename);
    
    if (rename(src_path, dst_path) == 0) {
        printf("Moved %s from %s to %s\n", filename, src_dir, dst_dir);
        if (strcmp(dst_dir, QUARANTINE_DIR) == 0) {
            write_log("%s - Successfully moved to quarantine directory.", filename);
        } else if (strcmp(dst_dir, STARTER_KIT_DIR) == 0) {
            write_log("%s - Successfully returned to starter kit directory.", filename);
        }
    } else {
        perror("Error moving file");
    }
}

// Function to move files from starter kit to quarantine
void quarantine_files() {
    DIR *dir = opendir(STARTER_KIT_DIR);
    if (!dir) {
        perror("Error opening starter kit directory");
        return;
    }

    create_directory_if_not_exists(QUARANTINE_DIR);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            move_file(STARTER_KIT_DIR, QUARANTINE_DIR, entry->d_name);
        }
    }

    closedir(dir);
}

// Function to move files from quarantine to starter kit
void return_files() {
    DIR *dir = opendir(QUARANTINE_DIR);
    if (!dir) {
        perror("Error opening quarantine directory");
        return;
    }

    create_directory_if_not_exists(STARTER_KIT_DIR);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            move_file(QUARANTINE_DIR, STARTER_KIT_DIR, entry->d_name);
        }
    }

    closedir(dir);
}

// Function to eradicate (delete) files in quarantine
void eradicate_files() {
    DIR *dir = opendir(QUARANTINE_DIR);
    if (!dir) {
        perror("Error opening quarantine directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", QUARANTINE_DIR, entry->d_name);
            
            if (unlink(path) == 0) {
                printf("Deleted %s\n", entry->d_name);
                write_log("%s - Successfully deleted.", entry->d_name);
            } else {
                perror("Error deleting file");
            }
        }
    }

    closedir(dir);
}

// Function to display usage information
void display_usage() {
    printf("Usage: ./starterkit [OPTION]\n");
    printf("Options:\n");
    printf("  --decrypt     Start the decryption daemon\n");
    printf("  --quarantine  Move files from starter kit to quarantine\n");
    printf("  --return      Move files from quarantine to starter kit\n");
    printf("  --eradicate   Delete all files in quarantine\n");
    printf("  --shutdown    Stop the decryption daemon\n");
}

int main(int argc, char *argv[]) {
    // Create log file if it doesn't exist
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fclose(log_file);
    } else {
        perror("Error creating log file");
    }

    // If no arguments, download and extract starter kit
    if (argc == 1) {
        download_and_extract();
        return 0;
    }

    // Handle command line arguments
    if (argc == 2) {
        if (strcmp(argv[1], "--decrypt") == 0) {
            start_decrypt_daemon();
        } else if (strcmp(argv[1], "--quarantine") == 0) {
            quarantine_files();
        } else if (strcmp(argv[1], "--return") == 0) {
            return_files();
        } else if (strcmp(argv[1], "--eradicate") == 0) {
            eradicate_files();
        } else if (strcmp(argv[1], "--shutdown") == 0) {
            stop_decrypt_daemon();
        } else {
            display_usage();
            return 1;
        }
    } else {
        display_usage();
        return 1;
    }

    return 0;
}
