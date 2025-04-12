#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <locale.h>
#define MAX_PATH 1024
#define MAX_FILENAME 256
#define LOG_FILE "activity.log"

typedef struct {
    pid_t pid;
    int running;
} DaemonProcess;

DaemonProcess decryption_daemon = {0, 0};

// Base64 decoding using OpenSSL
char* base64_decode(const char* input, size_t length, size_t* output_length) {
    BIO *bio, *b64;
    char* buffer = (char*)malloc(length);
    if (!buffer) return NULL;

    bio = BIO_new_mem_buf((void*)input, length);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    *output_length = BIO_read(bio, buffer, length);
    if (*output_length <= 0) {
        free(buffer);
        buffer = NULL;
    }

    BIO_free_all(bio);
    return buffer;
}

char* base64_decode_filename(const char *encoded) {
    size_t output_length;
    char *decoded = base64_decode(encoded, strlen(encoded), &output_length);
    if (!decoded) return NULL;
    
    decoded[output_length] = '\0';
    
    // Replace invalid filename characters
    for (size_t i = 0; i < output_length; i++) {
        if (strchr("/\\:*?\"<>|", decoded[i])) {
            decoded[i] = '_';
        }
    }
    
    return decoded;
}

void write_log(const char *action, const char *filename, pid_t pid) {
    setlocale(LC_ALL, "en_US.UTF-8");
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%d-%m-%Y][%H:%M:%S]", tm);

    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("Failed to open log file");
        return;
    }

    if (strcmp(action, "Decrypt") == 0 || strcmp(action, "Shutdown") == 0) {
        fprintf(log, "%s - %s decryption process with PID %d.\n", 
                timestamp, 
                (strcmp(action, "Decrypt") == 0 ? "Started" : "Stopped"), 
                pid);
    } 
    else if (filename) {
        fprintf(log, "%s - %s - %s\n", timestamp, filename,
                (strcmp(action, "Quarantine") == 0 ? "Moved to quarantine" :
                (strcmp(action, "Return") == 0 ? "Returned to starter kit" : "Deleted")));
    }
    fclose(log);
}
void download_and_extract() {
    printf("Downloading starter kit...\n");
    
    if (system("wget -q 'https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download' -O starter_kit.zip") != 0) {
        fprintf(stderr, "Error: Failed to download starter kit\n");
        exit(EXIT_FAILURE);
    }
    
    mkdir("starter_kit", 0755);
    
    if (system("unzip -q starter_kit.zip -d starter_kit") != 0) {
        fprintf(stderr, "Error: Failed to extract starter kit\n");
        exit(EXIT_FAILURE);
    }
    
    if (remove("starter_kit.zip") != 0) {
        perror("Warning: Failed to remove starter_kit.zip");
    }
    
    printf("Download and extraction completed.\n");
}

void decryption_daemon_process() {
    mkdir("quarantine", 0755);
    
    while (1) {
        DIR *dir = opendir("quarantine");
        if (!dir) {
            perror("Failed to open quarantine directory");
            sleep(5);
            continue;
        }
        
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                char *decoded = base64_decode_filename(ent->d_name);
                if (!decoded) continue;
                
                char old_path[MAX_PATH], new_path[MAX_PATH];
                snprintf(old_path, MAX_PATH, "quarantine/%s", ent->d_name);
                snprintf(new_path, MAX_PATH, "quarantine/%s", decoded);
                
                if (rename(old_path, new_path) != 0) {
                    perror("Failed to rename file");
                }
                
                free(decoded);
            }
        }
        closedir(dir);
        sleep(5);
    }
}

void start_decryption_daemon() {
    if (decryption_daemon.running) {
        printf("Decryption daemon is already running with PID %d\n", decryption_daemon.pid);
        return;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork decryption daemon");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        setsid();
        decryption_daemon_process();
        exit(EXIT_SUCCESS);
    }
    else {
        decryption_daemon.pid = pid;
        decryption_daemon.running = 1;
        printf("Decryption daemon started with PID %d\n", pid);
        write_log("Decrypt", "", pid);
    }
}
void stop_decryption_daemon() {
    // Cek apakah daemon sedang berjalan
    if (!decryption_daemon.running) {
        printf("Decryption daemon is not running.\n");
        return;
    }

    // Kirim sinyal SIGTERM ke proses daemon
    if (kill(decryption_daemon.pid, SIGTERM) == 0) {
        // Catat log SEBELUM mengubah state
        write_log("Shutdown", NULL, decryption_daemon.pid);
        
        // Update status
        decryption_daemon.running = 0;
        printf("Daemon (PID: %d) stopped successfully.\n", decryption_daemon.pid);
    } else {
        // Error handling
        perror("Failed to stop daemon");
        write_log("Shutdown", "FAILED", decryption_daemon.pid);  // Catat kegagalan
    }
}
void quarantine_files() {
    mkdir("quarantine", 0755);
    
    DIR *dir = opendir("starter_kit");
    if (!dir) {
        perror("Failed to open starter_kit directory");
        return;
    }
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            char old_path[MAX_PATH], new_path[MAX_PATH];
            snprintf(old_path, MAX_PATH, "starter_kit/%s", ent->d_name);
            snprintf(new_path, MAX_PATH, "quarantine/%s", ent->d_name);
            
            if (rename(old_path, new_path) == 0) {
                write_log("Quarantine", ent->d_name, 0);
            } else {
                perror("Failed to move file to quarantine");
            }
        }
    }
    closedir(dir);
}

void return_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        perror("Failed to open quarantine directory");
        return;
    }
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            char old_path[MAX_PATH], new_path[MAX_PATH];
            snprintf(old_path, MAX_PATH, "quarantine/%s", ent->d_name);
            snprintf(new_path, MAX_PATH, "starter_kit/%s", ent->d_name);
            
            if (rename(old_path, new_path) == 0) {
                write_log("Return", ent->d_name, 0);
            } else {
                perror("Failed to return file to starter kit");
            }
        }
    }
    closedir(dir);
}
void eradicate_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        perror("Eradicate failed: quarantine directory not found");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            write_log("Eradicate", ent->d_name, 0);  // Catat sebelum hapus
            remove(ent->d_name);
        }
    }
    closedir(dir);
}
void print_usage() {
    printf("Usage: ./starterkit [OPTION]\n");
    printf("Options:\n");
    printf("  --decrypt     Start decryption daemon\n");
    printf("  --quarantine  Move files to quarantine\n");
    printf("  --return      Return files from quarantine\n");
    printf("  --eradicate   Delete all files in quarantine\n");
    printf("  --shutdown    Stop decryption daemon\n");
    printf("  --help        Show this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage();
        return EXIT_FAILURE;
    }
    
    // Check if starter_kit exists, download if not
    struct stat st;
    if (stat("starter_kit", &st) == -1) {
        download_and_extract();
    }
    
    if (strcmp(argv[1], "--decrypt") == 0) {
        start_decryption_daemon();
    }
    else if (strcmp(argv[1], "--quarantine") == 0) {
        quarantine_files();
    }
    else if (strcmp(argv[1], "--return") == 0) {
        return_files();
    }
    else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    }
    else if (strcmp(argv[1], "--shutdown") == 0) {
        stop_decryption_daemon();
    }
    else if (strcmp(argv[1], "--help") == 0) {
        print_usage();
    }
    else {
        fprintf(stderr, "Error: Invalid option '%s'\n", argv[1]);
        print_usage();
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
