#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_FILES 100
#define MAX_FILENAME 50
#define MAX_CONTENT 1000
#define MAX_PATH 1024  // Diperbesar untuk menghindari truncation
#define DOWNLOAD_URL "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download"
#define PASSWORD_CHECK_URL "https://dragon-pw-checker.vercel.app/"

void downloadAndUnzip();
void filterFiles();
void combineFiles();
void decodeFile();
void checkPassword(const char *password);
void printUsage(char *program_name);
void executeCommand(char *const args[]);
int isFileExists(const char *path);
int compare_filenames(const void *a, const void *b);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        downloadAndUnzip();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
            filterFiles();
        } else if (strcmp(argv[2], "Combine") == 0) {
            combineFiles();
        } else if (strcmp(argv[2], "Decode") == 0) {
            decodeFile();
        } else {
            printUsage(argv[0]);
        }
    } else if (argc == 4 && strcmp(argv[1], "-m") == 0 && strcmp(argv[2], "Check") == 0) {
        checkPassword(argv[3]);
    } else {
        printUsage(argv[0]);
    }
    return 0;
}

void printUsage(char *program_name) {
    printf("Usage:\n");
    printf("  %s                       - Download and unzip Clues.zip\n", program_name);
    printf("  %s -m Filter             - Filter files\n", program_name);
    printf("  %s -m Combine            - Combine filtered files\n", program_name);
    printf("  %s -m Decode             - Decode combined file\n", program_name);
    printf("  %s -m Check <password>   - Check password at %s\n", program_name, PASSWORD_CHECK_URL);
}

void executeCommand(char *const args[]) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed with exit code %d\n", WEXITSTATUS(status));
            exit(EXIT_FAILURE);
        }
    }
}

int isFileExists(const char *path) {
    return access(path, F_OK) == 0;
}

void downloadAndUnzip() {
    if (isFileExists("Clues")) {
        printf("Clues directory already exists. Skipping download.\n");
        return;
    }

    printf("Downloading Clues.zip...\n");
    char *wget_args[] = {"wget", "-q", DOWNLOAD_URL, "-O", "Clues.zip", NULL};
    executeCommand(wget_args);
    
    printf("Unzipping Clues.zip...\n");
    char *unzip_args[] = {"unzip", "-q", "Clues.zip", NULL};
    executeCommand(unzip_args);
    
    printf("Cleaning up...\n");
    if (unlink("Clues.zip") != 0) {
        perror("Failed to delete Clues.zip");
    }
    
    printf("Done! Clues extracted to Clues directory.\n");
}

void filterFiles() {
    struct dirent *entry, *sub_entry;
    DIR *dp, *sub_dp;
    char path[MAX_PATH];

    if (mkdir("Filtered", 0755) == -1 && errno != EEXIST) {
        perror("Failed to create Filtered directory");
        exit(EXIT_FAILURE);
    }

    if ((dp = opendir("Clues")) == NULL) {
        perror("Unable to open Clues directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(path, sizeof(path), "Clues/%s", entry->d_name);
            if ((sub_dp = opendir(path)) == NULL) {
                perror("Unable to open subdirectory");
                continue;
            }

            while ((sub_entry = readdir(sub_dp))) {
                if (sub_entry->d_type == DT_REG) {
                    int name_len = strlen(sub_entry->d_name);
                    if (name_len == 5 && strcmp(sub_entry->d_name + 1, ".txt") == 0 && isalnum(sub_entry->d_name[0])) {
                        char src_path[MAX_PATH], dest_path[MAX_PATH];
                        
                        // Pengecekan panjang path sebelum menggunakan snprintf
                        int needed_length = snprintf(NULL, 0, "Clues/%s/%s", entry->d_name, sub_entry->d_name);
                        if (needed_length < sizeof(src_path)) {
                            snprintf(src_path, sizeof(src_path), "Clues/%s/%s", entry->d_name, sub_entry->d_name);
                        } else {
                            fprintf(stderr, "Path too long: %s/%s\n", entry->d_name, sub_entry->d_name);
                            continue;  // Skip file if path is too long
                        }

                        snprintf(dest_path, sizeof(dest_path), "Filtered/%s", sub_entry->d_name);
                        
                        if (rename(src_path, dest_path) != 0) {
                            perror("Failed to move file");
                        }
                    }
                }
            }
            closedir(sub_dp);
        }
    }
    closedir(dp);
    printf("Filtering complete. Valid files moved to Filtered directory.\n");
}

int compare_filenames(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combineFiles() {
    DIR *dp;
    struct dirent *entry;
    char *filenames[MAX_FILES];
    int num_files = 0;

    if ((dp = opendir("Filtered")) == NULL) {
        perror("Unable to open Filtered directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dp)) && num_files < MAX_FILES) {
        if (entry->d_type == DT_REG) {
            if (strlen(entry->d_name) == 5 && 
                isalnum(entry->d_name[0]) && 
                strcmp(entry->d_name + 1, ".txt") == 0) {
                filenames[num_files] = strdup(entry->d_name);
                if (!filenames[num_files]) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                num_files++;
            }
        }
    }
    closedir(dp);

    // Separate numbers and letters
    int num_count = 0, alpha_count = 0;
    char *numbers[MAX_FILES/2], *letters[MAX_FILES/2];

    for (int i = 0; i < num_files; i++) {
        if (isdigit(filenames[i][0])) {
            numbers[num_count++] = filenames[i];
        } else {
            letters[alpha_count++] = filenames[i];
        }
    }

    // Sort each group
    qsort(numbers, num_count, sizeof(char*), compare_filenames);
    qsort(letters, alpha_count, sizeof(char*), compare_filenames);

    // Interleave the sorted groups
    int combined_index = 0;
    int max_count = num_count > alpha_count ? num_count : alpha_count;
    for (int i = 0; i < max_count; i++) {
        if (i < num_count) {
            filenames[combined_index++] = numbers[i];
        }
        if (i < alpha_count) {
            filenames[combined_index++] = letters[i];
        }
    }

    // Combine files
    FILE *combined = fopen("Combined.txt", "w");
    if (!combined) {
        perror("Failed to create Combined.txt");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_files; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "Filtered/%s", filenames[i]);

        FILE *fp = fopen(path, "r");
        if (fp) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), fp)) {
                fputs(buffer, combined);
            }
            fclose(fp);
            remove(path);
        }
        free(filenames[i]);
    }
    fclose(combined);
    printf("Files combined successfully into Combined.txt\n");
}

void decodeFile() {
    FILE *fp;
    char content[MAX_CONTENT];
    char decoded[MAX_CONTENT];

    if ((fp = fopen("Combined.txt", "r")) == NULL) {
        perror("Unable to open Combined.txt");
        exit(EXIT_FAILURE);
    }

    if (fgets(content, sizeof(content), fp) == NULL) {
        fclose(fp);
        printf("Combined.txt is empty\n");
        return;
    }
    fclose(fp);

    for (int i = 0; content[i] != '\0' && i < sizeof(content) - 1; i++) {
        char c = content[i];
        if (c >= 'a' && c <= 'z') {
            decoded[i] = 'a' + ((c - 'a' + 13) % 26);
        } else if (c >= 'A' && c <= 'Z') {
            decoded[i] = 'A' + ((c - 'A' + 13) % 26);
        } else {
            decoded[i] = c;
        }
    }
    decoded[strlen(content)] = '\0';

    if ((fp = fopen("Decoded.txt", "w")) == NULL) {
        perror("Unable to create Decoded.txt");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s", decoded);
    fclose(fp);

    printf("File decoded successfully into Decoded.txt\n");
    printf("The password is: %s\n", decoded);
    printf("You can check this password at: %s\n", PASSWORD_CHECK_URL);
}

void checkPassword(const char *password) {
    printf("Checking password at %s...\n", PASSWORD_CHECK_URL);
    printf("Please visit the website and enter this password:\n");
    printf("==> %s <==\n", password);
}
