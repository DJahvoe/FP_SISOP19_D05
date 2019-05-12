# FinalProject_D05

- Problem : <br>
Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.
Note: playlist bisa banyak

- Task : <br>
  - Pembuatan FUSE untuk mem-filter file dengan ekstensi .mp3 pada root dari folder dan file serta subfolder-subfoldernya.
  - Pembuatan MP3 Player yang dapat memainkan file ekstensi .mp3 dari hasil FUSE sebelumnya.

<br>

- Pembuatan FUSE :
  - Tools yang dibutuhkan <br>
    - header penting<br>
      (*template)
      - stdlib.h
      - fuse.h
      - stdio.h
      - string.h
      - unistd.h
      - fcntl.h
      - dirent.h
      - errno.h
      - sys/time.h 
      - sys/xattr.h <br>
      (*tambahan selain template)
      - pwd.h (checking file or folder)
      - grp.h (checking file or folder) 
     - template diambil dari <a href="https://github.com/asayler/CU-CS3753-PA5/blob/master/fusexmp.c">sini</a><br>
   - Bagian template FUSE yang dimodifikasi <br>
      - Dilakukan penambahan `dirpath` pada tiap fungsi template
      - Penambahan fungsi `xmp_init` dan `xmp_destroy`
      - Modifikasi pada `xmp_destroy`
      - Modifikasi pada 'xmp_readdir` <br>
   - Approach (sub-task yang harus diselesaikan) :
      1. Memastikan directory yang berada pada `root` (home/[user]) dapat dicek hingga subfolder-subfoldernya.
      2. Untuk menyelesaikan sub-task 1 digunakan `Queue` yang menampung directory dan sub-directory `root`
      3. Membuat fungsi - fungsi pendukung (`support`) dari `Queue` seperti `INSERT`, `POP`, dan `DISPLAY`.
      4. Mendeteksi bagian dalam dari sebuah `directory`, berupa `FILE` atau `FOLDER` (dalam hal ini digunakan `S_ISREG` untuk `FILE` dan `S_ISDIR` untuk `FOLDER`)
      5. Memastikan `FILE` atau `FOLDER` yang bersifat `hidden` tidak di-cek untuk mencegah `overflow` pada program (di mana pada Linux terdapat karakteristik bahwa `FILE` atau `FOLDER` diawali dengan karakter `.` atau diakhiri dengan `~`
      6. Tiap ditemukan `FOLDER` pada `Directory` tertentu akan di-insert ke dalam `Queue`
      7. Tiap ditemukan `FILE` dengan extension `.mp3` akan dimasukkan ke dalam fungsi `filler`, setelah itu file .mp3 tersebut akan di-move ke root
      8. `readdir` akan selesai pada saat `Queue` kosong (seluruh `Directory` dan `Sub-Directory` dari `root` berhasil di-trace)
      9. Di bagian akhir saat `FUSE` dimatikan, fungsi `destroy` akan trigger yang berfungsi untuk mengembalikan file `.mp3` kembali ke directory asalnya <br> <br>
      
- Pembuatan MP3Player :
  - Tools yang dibutuhkan <br>
    - header penting<br>
      - stdio.h
      - string.h
      - pthread.h
      - stdlib.h
      - unistd.h
      - sys/ipc.h
      - sys/shm.h
      - sys/types.h
      - sys/wait.h
      - dirent.h (melacak file .mp3)
      - sys/types.h
      - sys/stat.h
      - unistd.h <br>
      (*header MP3Player) <br>
      - ao/ao.h
      - mpg123.h <br>
      Untuk bisa menjalankan header MP3Player diperlukan instalasi dengan command sebagai berikut <br>
      - `sudo apt-get install libao-dev`
      - `sudo apt-get install mpg123`
      - `sudo apt-get install libmpg123-dev`
     - template diambil dari <a href="http://hzqtc.github.io/2012/05/play-mp3-with-libmpg123-and-libao.html">sini</a><br>
   - Bagian template MP3Player yang dimodifikasi <br>
      - Dilakukan penambahan sistem `Menu`
      - Penambahan fungsi `Inisialisasi`, `getch`, dan fungsi-fungsi pada sistem `Menu`
      - Modifikasi pada fungsi `Play` <br>
   - Approach (sub-task yang harus diselesaikan) :
      1. Menggunakan `thread` untuk menghandle `supportive function` seperti Choices
      2. Memastikan input `directory` mengarah pada `directory` yang sama dengan `FUSE`
      3. Setelah `directory FUSE` terdeteksi, menyimpan seluruh nama file .mp3 pada `local array`
      4. Terdapat variable `counter` sebagai pointer lagu yang akan dimainkan saat tombol `play` ditekan
      5. Untuk melakukan fitur `play`, hanya perlu menjalankan fungsi `Play` dan mengambil nama dari `local array`
      6. Untuk melakukan fitur `pause`, ditambahkan variable `int Pause` dengan conditional di dalam loop untuk menghentikan `mpg123_read` untuk sementara (apabila `Pause = 1`) dan akan lanjut melakukan `read` (apabila `Pause = 0`)
      7. Untuk melakukan fitur `next`, hanya perlu menggeser `counter` ke kanan
      8. Untuk melakukan fitur `prev`, hanya perlu menggeser `counter` ke kiri
      9. Untuk melakukan fitur `list lagu`, hanya perlu melakukan `display` seluruh isi `local array`
      10. Apabila User menekan tombol selain `pause` pada saat lagu sedang di-play, maka sistem akan menganggap user melakukan `Interrupt` dan menghentikan lagu (bukan `pause`) serta menjalankan perintah sesuai fitur yang dipilih oleh user (tombol yang ditekan user)

- Tutorial play <br>
  <ol type="1">
    <li>Download Repo</li>
    <li>Ubah file FUSEMP3_3.c sesuaikan user dengan user laptop anda</li> 
  <p align="center">
    <img src="/tutorial/1.JPG" width="70%" alt="step1" title="Ubah FUSE">
  </p>
    <li>Compile FUSE yang sudah diedit " gcc -Wall `pkg-config fuse --cflags` FUSEMP3_3.c -o FUSEMP3 `pkg-config fuse --libs` "</li>
  <p align="center">
    <img src="/tutorial/2.JPG" width="70%" alt="step2" title="Compile FUSE">
  </p>
    <li>Compile ulang menu.c " gcc -O2 -o DejahvoePlayer menu.c -pthread -lao -lmpg123 "</li>
  <p align="center">
    <img src="/tutorial/3.JPG" width="70%" alt="step3" title="Compile Music Player">
  </p>
    <li>Run FUSE ke direktori yang berisi file-file mp3 yang akan diplay </li>
  <p align="center">
    <img src="/tutorial/4.JPG" width="70%" alt="step4" title="Run FUSE">
  </p>
     <li>Jalankan DejahvoePlayer inputkan path folder FUSE sebelumnya </li>
  <p align="center">
    <img src="/tutorial/5.JPG" width="70%" alt="step5" title="Run Player">
  </p>
    <li>Enjoy your musics </li>
  </ol>
