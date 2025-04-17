#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

// Function to display the directory structure
void print_directory_structure() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Redirect stdout to /dev/null to not display the actual output
        int devnull = open("/dev/null", O_WRONLY);
        dup2(devnull, STDOUT_FILENO);
        close(devnull);
        
        // Execute tree command
        execl("/usr/bin/tree", "tree", "--noreport", "-C", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}

#define DOWNLOAD_URL "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download"
#define ZIP_FILE "Clues.zip"

// Function to check if a directory exists
int directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

// Function to download and extract Clues
void download_and_extract() {
    // Check if Clues directory already exists
    if (directory_exists("Clues")) {
        printf("Clues directory already exists. Skipping download.\n");
        return;
    }

    printf("Downloading Clues.zip...\n");
    
    // Download the file using curl
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execl("/usr/bin/curl", "curl", "-L", "-o", ZIP_FILE, DOWNLOAD_URL, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Download complete. Extracting...\n");
            
            // Extract the ZIP file
            pid_t unzip_pid = fork();
            if (unzip_pid == 0) {
                // Child process
                execl("/usr/bin/unzip", "unzip", ZIP_FILE, NULL);
                perror("execl");
                exit(EXIT_FAILURE);
            } else if (unzip_pid > 0) {
                // Parent process
                int unzip_status;
                waitpid(unzip_pid, &unzip_status, 0);
                if (WIFEXITED(unzip_status) && WEXITSTATUS(unzip_status) == 0) {
                    printf("Extraction complete. Removing ZIP file...\n");
                    
                    // Remove the ZIP file
                    if (remove(ZIP_FILE) == 0) {
                        printf("ZIP file removed successfully.\n");
                    } else {
                        perror("remove");
                    }
                    
                    // Make sure the Clues directory has the expected structure
                    printf("Ensuring correct directory structure...\n");
                } else {
                    printf("Extraction failed.\n");
                }
            } else {
                perror("fork");
            }
        } else {
            printf("Download failed.\n");
        }
    } else {
        perror("fork");
    }
}

// Function to check if a filename is valid (one letter or one digit)
int is_valid_filename(const char *filename) {
    // Get the filename without path
    const char *base = strrchr(filename, '/');
    if (base) {
        base++;
    } else {
        base = filename;
    }
    
    // Get the extension
    const char *ext = strrchr(base, '.');
    if (!ext) {
        return 0;
    }
    
    // Check if the filename (without extension) is exactly one character
    if (ext - base != 1) {
        return 0;
    }
    
    // Check if the character is a letter or a digit
    char c = base[0];
    return (isalpha(c) || isdigit(c));
}

// Function to filter files
void filter_files() {
    // Create Filtered directory if it doesn't exist
    if (!directory_exists("Filtered")) {
        if (mkdir("Filtered", 0755) != 0) {
            perror("mkdir");
            return;
        }
        printf("Created Filtered directory.\n");
    }

    // Go through each Clue directory
    const char *clue_dirs[] = {"Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"};
    for (int i = 0; i < 4; i++) {
        DIR *dir = opendir(clue_dirs[i]);
        if (!dir) {
            printf("Failed to open directory: %s\n", clue_dirs[i]);
            continue;
        }
        
        struct dirent *entry;
        // First, collect all filenames to process
        char filenames[100][256];
        int file_count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {  // Check if it's a regular file
                strcpy(filenames[file_count], entry->d_name);
                file_count++;
            }
        }
        closedir(dir);
        
        // Process all files
        for (int j = 0; j < file_count; j++) {
            char src_path[512];
            snprintf(src_path, sizeof(src_path), "%s/%s", clue_dirs[i], filenames[j]);
            
            if (is_valid_filename(filenames[j])) {
                // Move valid files to Filtered directory
                char dst_path[512];
                snprintf(dst_path, sizeof(dst_path), "Filtered/%s", filenames[j]);
                
                FILE *src = fopen(src_path, "r");
                if (!src) {
                    perror("fopen source");
                    continue;
                }
                
                FILE *dst = fopen(dst_path, "w");
                if (!dst) {
                    perror("fopen destination");
                    fclose(src);
                    continue;
                }
                
                char buffer[1024];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                    fwrite(buffer, 1, bytes, dst);
                }
                
                fclose(src);
                fclose(dst);
                
                // Delete the original file after copying it to Filtered
                if (remove(src_path) == 0) {
                    printf("Moved %s to Filtered directory and removed original.\n", filenames[j]);
                } else {
                    perror("remove");
                }
            } else {
                // Delete invalid files
                if (remove(src_path) == 0) {
                    printf("Removed invalid file: %s\n", filenames[j]);
                } else {
                    perror("remove");
                }
            }
        }
    }
}

// Function to compare filenames for sorting
int compare_filenames(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to combine file contents
void combine_files() {
    DIR *dir = opendir("Filtered");
    if (!dir) {
        printf("Failed to open Filtered directory.\n");
        return;
    }
    
    // Collect all digit filenames and letter filenames
    char *digit_files[100];
    char *letter_files[100];
    int digit_count = 0;
    int letter_count = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *filename = strdup(entry->d_name);
            if (isdigit(filename[0])) {
                digit_files[digit_count++] = filename;
            } else if (isalpha(filename[0])) {
                letter_files[letter_count++] = filename;
            }
        }
    }
    
    closedir(dir);
    
    // Sort the filenames
    qsort(digit_files, digit_count, sizeof(char *), compare_filenames);
    qsort(letter_files, letter_count, sizeof(char *), compare_filenames);
    
    // Open the combined file
    FILE *combined = fopen("Combined.txt", "w");
    if (!combined) {
        perror("fopen Combined.txt");
        return;
    }
    
    // Combine the files in the required order (alternating digit, letter)
    int digit_idx = 0;
    int letter_idx = 0;
    
    while (digit_idx < digit_count || letter_idx < letter_count) {
        // Process digit file
        if (digit_idx < digit_count) {
            char path[512];
            snprintf(path, sizeof(path), "Filtered/%s", digit_files[digit_idx]);
            
            FILE *file = fopen(path, "r");
            if (file) {
                char buffer[1024];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    fwrite(buffer, 1, bytes, combined);
                }
                fclose(file);
                printf("Combined content from %s\n", digit_files[digit_idx]);
                
                // Delete the file
                if (remove(path) == 0) {
                    printf("Removed %s after combining.\n", digit_files[digit_idx]);
                } else {
                    perror("remove");
                }
                
                digit_idx++;
            }
        }
        
        // Process letter file
        if (letter_idx < letter_count) {
            char path[512];
            snprintf(path, sizeof(path), "Filtered/%s", letter_files[letter_idx]);
            
            FILE *file = fopen(path, "r");
            if (file) {
                char buffer[1024];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    fwrite(buffer, 1, bytes, combined);
                }
                fclose(file);
                printf("Combined content from %s\n", letter_files[letter_idx]);
                
                // Delete the file
                if (remove(path) == 0) {
                    printf("Removed %s after combining.\n", letter_files[letter_idx]);
                } else {
                    perror("remove");
                }
                
                letter_idx++;
            }
        }
    }
    
    fclose(combined);
    printf("Combined all files into Combined.txt\n");
    
    // Free allocated memory
    for (int i = 0; i < digit_count; i++) {
        free(digit_files[i]);
    }
    for (int i = 0; i < letter_count; i++) {
        free(letter_files[i]);
    }
}

// Function to decode using ROT13
char rot13(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        char offset = (c >= 'a' && c <= 'z') ? 'a' : 'A';
        return ((c - offset + 13) % 26) + offset;
    }
    return c;
}

// Function to decode the file
void decode_file() {
    FILE *combined = fopen("Combined.txt", "r");
    if (!combined) {
        printf("Failed to open Combined.txt\n");
        return;
    }
    
    FILE *decoded = fopen("Decoded.txt", "w");
    if (!decoded) {
        perror("fopen Decoded.txt");
        fclose(combined);
        return;
    }
    
    int c;
    while ((c = fgetc(combined)) != EOF) {
        fputc(rot13((char)c), decoded);
    }
    
    fclose(combined);
    fclose(decoded);
    printf("Decoded Combined.txt to Decoded.txt using ROT13\n");
}

// Function to display usage
void display_usage() {
    printf("Usage: ./action [-m Mode]\n");
    printf("Modes:\n");
    printf("  Filter   - Filter files with single letter/digit names\n");
    printf("  Combine  - Combine filtered files into a single file\n");
    printf("  Decode   - Decode the combined file using ROT13\n");
    printf("  Check    - Check the password from the decoded file\n");
    printf("Without arguments, it will download and extract the clues.\n");
}

// Function to check the password
void check_password() {
    // Open the decoded file
    FILE *decoded = fopen("Decoded.txt", "r");
    if (!decoded) {
        printf("Failed to open Decoded.txt. Please decode the file first.\n");
        return;
    }
    
    // Read the password from the file
    char password[256] = {0};
    if (fgets(password, sizeof(password), decoded) != NULL) {
        // Remove newline character if present
        size_t len = strlen(password);
        if (len > 0 && password[len-1] == '\n') {
            password[len-1] = '\0';
        }
        
        printf("Password found: %s\n", password);
        printf("Please input this password at the 'lokasi' mentioned in the problem.\n");
    } else {
        printf("Could not read password from file.\n");
    }
    
    fclose(decoded);
}

int main(int argc, char *argv[]) {
    // Parse arguments
    if (argc == 1) {
        // No arguments, download and extract
        download_and_extract();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
            filter_files();
        } else if (strcmp(argv[2], "Combine") == 0) {
            combine_files();
        } else if (strcmp(argv[2], "Decode") == 0) {
            decode_file();
        } else if (strcmp(argv[2], "Check") == 0) {
            check_password();
        } else {
            printf("Invalid mode: %s\n", argv[2]);
            display_usage();
            return 1;
        }
    } else {
        display_usage();
        return 1;
    }
    
    return 0;
}
