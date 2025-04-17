===================[Kelompok IT27]======================

Khumaidi Kharis Az-zacky - 5027241049

Mohamad Arkan Zahir Asyafiq - 5027241120

Abiyyu Raihan Putra Wikanto - 5027241042

============[Laporan Resmi Penjelasan Soal]=============

#soal_1
Pada soal ini, kita diminta untuk membuat sebuah program C untuk memproses file-file clue yang diberikan oleh Cyrus (naga penjaga pintu) untuk mendapatkan password. Program ini melakukan beberapa tahap pengolahan file, mulai dari mengunduh, memfilter, menggabungkan, hingga mendekode konten file.

Penjelasan code:
          
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
            #define DOWNLOAD_URL "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download"
            #define ZIP_FILE "Clues.zip"

Bagian ini mengimpor berbagai library yang diperlukan:

stdio.h untuk fungsi-fungsi I/O standar seperti printf, fopen, dsb.
stdlib.h untuk fungsi-fungsi umum seperti malloc, free, dsb.
string.h untuk manipulasi string seperti strcmp, strrchr, dsb.
unistd.h untuk fungsi sistem operasi seperti fork, execl, dsb.
sys/stat.h untuk fungsi yang berhubungan dengan status file dan direktori
dirent.h untuk operasi direktori seperti opendir, readdir, dsb.
ctype.h untuk fungsi klasifikasi karakter seperti isalpha, isdigit, dsb.
sys/wait.h untuk fungsi yang berhubungan dengan proses seperti waitpid
fcntl.h untuk kontrol file seperti open
time.h untuk fungsi yang berhubungan dengan waktu
DOWNLOAD_URL adalah URL untuk mengunduh file Clues.zip
ZIP_FILE adalah nama file ZIP yang akan diunduh

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
Fungsi ini mencoba menampilkan struktur direktori menggunakan perintah tree:

fork() membuat proses child
Dalam proses child, output diarahkan ke /dev/null menggunakan dup2
Kemudian perintah tree dijalankan dengan opsi --noreport dan -C (warna)
Proses parent menunggu proses child selesai dengan waitpid

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

Dua fungsi untuk memeriksa keberadaan direktori dan file:

directory_exists menggunakan stat untuk mendapatkan informasi tentang path dan memeriksa jika itu adalah direktori
file_exists mirip, tapi memeriksa jika path adalah file reguler

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

Fungsi untuk mengunduh dan mengekstrak file Clues.zip:

Pertama memeriksa apakah direktori Clues sudah ada, jika ada, proses dilewati
Menggunakan fork() untuk membuat proses child yang menjalankan curl untuk mengunduh file
Parent process menunggu download selesai
Jika download berhasil, membuat proses child lain untuk menjalankan unzip
Setelah ekstraksi berhasil, file ZIP dihapus menggunakan remove()

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

Fungsi untuk memeriksa jika sebuah nama file valid (satu huruf atau satu angka):

Pertama, mendapatkan nama file tanpa path menggunakan strrchr()
Kemudian, mendapatkan ekstensi file (setelah titik terakhir)
Memeriksa jika panjang nama file (tanpa ekstensi) tepat satu karakter
Terakhir, memeriksa jika karakter tersebut adalah huruf atau angka

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

Fungsi untuk memfilter file-file:

Membuat direktori Filtered jika belum ada
Menelusuri setiap direktori ClueA, ClueB, ClueC, dan ClueD
Mengumpulkan semua nama file dalam array
Untuk setiap file, memeriksa jika namanya valid (satu huruf/angka)
File yang valid disalin ke direktori Filtered dan file aslinya dihapus
File yang tidak valid langsung dihapus

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

Fungsi untuk menggabungkan konten file:

Membuka direktori Filtered dan mengumpulkan semua file
Memisahkan file menjadi dua kelompok: yang dimulai dengan angka dan yang dimulai dengan huruf
Mengurutkan kedua kelompok file menggunakan qsort()
Membuka file Combined.txt untuk penulisan
Menggabungkan file-file dengan urutan bergantian: angka, huruf, angka, huruf, dst.
Setiap file yang sudah digabungkan dihapus
Membebaskan memori yang dialokasikan untuk nama file

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

Fungsi untuk mendekode file menggunakan ROT13:

rot13() mengimplementasikan algoritma ROT13 yang menggeser huruf 13 posisi
decode_file() membuka file Combined.txt dan Decoded.txt
Membaca setiap karakter dari Combined.txt, mendekodenya dengan ROT13, dan menuliskannya ke Decoded.txt

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

Fungsi untuk memeriksa password:

Membuka file Decoded.txt
Membaca baris pertama file sebagai password
Menghapus karakter newline dari akhir password jika ada
Menampilkan password ke pengguna

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

Fungsi untuk menampilkan cara penggunaan program.

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

Fungsi main:

Memeriksa argumen command line
Jika tidak ada argumen, menjalankan download_and_extract()
Jika argumennya -m, menjalankan fungsi yang sesuai dengan mode yang diberikan
Jika argumen tidak valid, menampilkan pesan kesalahan dan bantuan penggunaan

#soal_2
Program ini dirancang untuk membantu Kanade mengidentifikasi dan mengisolasi malware dengan menyediakan alat pengelolaan file yang dapat memindahkan, mengisolasi, dan mengembalikan file-file yang mencurigakan, sambil secara otomatis mendekripsi nama file yang dienkode dengan Base64.

Penjelasan Code:

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
          
Ini adalah daftar header yang dibutuhkan oleh program
Termasuk fungsi untuk manipulasi file, proses, sinyal, waktu, dan variabel argumen
linux/limits.h digunakan untuk konstanta PATH_MAX yang menentukan panjang maksimum path file

          #define DOWNLOAD_URL "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download"
          #define ZIP_FILE "starterkit.zip"
          #define LOG_FILE "activity.log"
          #define STARTER_KIT_DIR "starter_kit"
          #define QUARANTINE_DIR "quarantine"
          #define DAEMON_PID_FILE "/tmp/decrypt_daemon.pid"

Mendefinisikan konstanta yang digunakan dalam program
URL untuk mengunduh starter kit
Nama file zip, file log, dan direktori
File untuk menyimpan PID (Process ID) dari daemon

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

Fungsi untuk menulis entri log ke file activity.log
Membuka file log dalam mode append ('a')
Mendapatkan waktu saat ini dan memformatnya sesuai format yang diminta: [dd-mm-YYYY][HH:MM]
Menggunakan fungsi variadic untuk memungkinkan format string seperti printf
Menutup file setelah selesai menulis

          int directory_exists(const char *path) {
              struct stat st;
              return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
          }
          
          void create_directory_if_not_exists(const char *path) {
              if (!directory_exists(path)) {
                  if (mkdir(path, 0755) != 0) {
                      perror("Error creating directory");
                      exit(EXIT_FAILURE);
                  }
              }
          }
          
directory_exists: Memeriksa apakah direktori sudah ada dengan menggunakan fungsi stat
create_directory_if_not_exists: Membuat direktori jika belum ada dengan izin 0755 (dapat dibaca, ditulis pemilik, dibaca dan dieksekusi oleh semua)

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

Mengecek apakah direktori starter kit sudah ada; jika sudah, lewati proses download
Menggunakan fork() dan execl() untuk menjalankan perintah curl dan mendownload file zip
Menunggu proses download selesai dengan waitpid() dan memeriksa status error
Membuat direktori starter kit jika belum ada
Menggunakan proses yang sama untuk menjalankan unzip
Menghapus file zip setelah ekstraksi selesai dengan unlink()
Perhatikan bahwa ini tidak menggunakan system() sesuai persyaratan

          static const unsigned char base64_table[65] =
              "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
          
          static const unsigned char base64_reverse_table[256] = {
              // ... (tabel inisialisasi panjang) ...
          };
          
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

Mendeklarasikan tabel karakter Base64 dan tabel terbalik untuk dekoding
base64_decode: Fungsi utama untuk mendekode string Base64 menjadi data asli

Mengalokasikan memori untuk hasil dekode
Melakukan proses dekode dengan operasi bitwise
Mengembalikan string hasil dekode dan panjangnya


is_base64_encoded: Memeriksa apakah string adalah Base64 valid

Memeriksa panjangnya (harus kelipatan 4)
Memeriksa setiap karakter apakah termasuk dalam alfabet Base64

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
          
          void write_daemon_pid(pid_t pid) {
              FILE *pid_file = fopen(DAEMON_PID_FILE, "w");
              if (!pid_file) {
                  perror("Failed to create PID file");
                  exit(EXIT_FAILURE);
              }
          
              fprintf(pid_file, "%d", pid);
              fclose(pid_file);
          }

read_daemon_pid: Membaca PID daemon dari file

Jika file tidak ada atau tidak dapat dibaca, mengembalikan -1


write_daemon_pid: Menulis PID daemon ke file

Menciptakan atau menimpa file PID yang ada

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

Memeriksa apakah daemon sudah berjalan dengan membaca file PID
Memastikan direktori karantina ada
Menggunakan fork() untuk membuat proses child
Proses parent:

Mencatat PID daemon dan menulis log
Kembali ke main


Proses child:

Menjadi daemon dengan setsid() untuk memisahkan dari terminal
Mengubah direktori kerja ke root
Menutup file descriptor standar dan mengalihkannya ke /dev/null
Menjalankan loop utama daemon:

Membuka direktori karantina
Mencari file dengan nama yang dikodekan Base64
Mendekode nama file
Mengganti nama file dengan nama yang sudah didekode
Tidur 5 detik sebelum memeriksa lagi

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

Membaca PID daemon, jika tidak ditemukan, tampilkan pesan dan kembali
Mengirim sinyal SIGTERM ke daemon untuk menghentikannya
Jika berhasil, menulis log dan menghapus file PID
Jika gagal, menampilkan pesan error

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

Membuat path lengkap untuk file sumber dan tujuan
Memindahkan file dengan rename()
Jika berhasil, mencetak pesan dan menulis log sesuai arah pemindahan
Jika gagal, menampilkan pesan error

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

quarantine_files: Memindahkan semua file dari starter kit ke karantina

Membuka direktori starter kit dan memastikan direktori karantina ada
Memindahkan setiap file reguler (bukan direktori)


return_files: Memindahkan semua file dari karantina kembali ke starter kit

Mirip dengan quarantine_files tetapi arah pemindahan berlawanan

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

Membuka direktori karantina
Untuk setiap file reguler dalam direktori:

Membuat path lengkap file
Menghapus file dengan unlink()
Jika berhasil, mencetak pesan dan menulis log
Jika gagal, menampilkan pesan error

          void display_usage() {
              printf("Usage: ./starterkit [OPTION]\n");
              printf("Options:\n");
              printf("  --decrypt     Start the decryption daemon\n");
              printf("  --quarantine  Move files from starter kit to quarantine\n");
              printf("  --return      Move files from quarantine to starter kit\n");
              printf("  --eradicate   Delete all files in quarantine\n");
              printf("  --shutdown    Stop the decryption daemon\n");
          }

Menampilkan informasi cara penggunaan program
Daftar semua opsi yang tersedia dengan deskripsi singkat

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

Memastikan file log ada dengan mencoba membukanya
Jika tidak ada argumen (argc == 1), program mendownload dan mengekstrak starter kit
Jika terdapat satu argumen (argc == 2):

--decrypt: Menjalankan daemon dekripsi
--quarantine: Memindahkan file ke karantina
--return: Mengembalikan file dari karantina
--eradicate: Menghapus file karantina
--shutdown: Menghentikan daemon
Argumen lain: Menampilkan cara penggunaan

Jika jumlah argumen tidak sesuai, menampilkan cara penggunaan


#soal_3

#soal_4
