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
#soal_2

#soal_3

#soal_4
