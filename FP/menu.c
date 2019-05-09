#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

//Read Directory
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//Player
#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8
//GETCH
#include <termios.h>

char fusepath[100] = "";

int Exit = 0;
int Interrupt = 0;
int Pause = 0;

int currentSong = 0; //Current Song Chosen
int nowplaying = 0; //No Song is playing

char SongList[100][100];
int ListSize = 0;
 
 void DisplayList();
 void ShowRecordMenu();
void* Play(void *arg)
{
    //Inisialisasi interrupt reset
    Interrupt = 0;
    nowplaying = 1;
    

    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    char redirSong[1000] = "";
    // Reconstruct directory
    sprintf(redirSong, "%s/%s", fusepath, SongList[currentSong]);
    /* open the file and get the decoding format */
    mpg123_open(mh, redirSong);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
    {
        //Pause
        while(Pause!=0 && Interrupt != 1)
        {
            printf("\r");
        }
        if(Interrupt == 1)
        {
            break;
        }
        ao_play(dev, buffer, done);
    }
    //Reset
    Pause = 0;
    Interrupt = 0;
    nowplaying = 0;
    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    return 0;
}

//Function Button Pressed Handler
char getch()
{
    char buf = 0;
    struct termios old = {0};
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

int Choices = 0;
int StateMenu = 0; // 0 StandBy, 1 Play, 2 Record, 3 Quit
pthread_t inputid;
void* input(void *arg)
{
    int character;
    while(character = getch())
    {
        switch(character)
        {
            case '1':
            Choices = 1;
            break;
            case '2':
            Choices = 2;
            break;
            case '3':
            Choices = 3;
            break;
            case '4':
            Choices = 4;
            break;
            case '5':
            Choices = 5;
            break;
            case '6':
            Choices = 6;
            break;
        }
    }
}

void ResetChoice()
{
    Choices = 0;
}

void StandbyMenu()
{
    int cek = system("clear");
    if(cek)return;
    printf ("*********** MP3 Player **********\n");
    //DisplayList();
      printf("\n1: Play");   
      printf("\n2: Show Record");
      printf("\n3: Quit");
       
      printf("\nEnter Choice From 1-3\n");

      switch(Choices)
      {
            case 1:
            //Menu Play
            StateMenu = 1;
            break;
            case 2:
            //Menu Record
            StateMenu = 2;
            break;
            case 3:
            //Quit
            StateMenu = 3;
            break;
      }
    ResetChoice();
}


pthread_t play_song;
void PlayMenu()
{
    
    int cek = system("clear");
    if(cek)return;
    printf("*********** Play ***********\n");
    printf("------List------\n");
    DisplayList();
    printf("----------------\n");
    printf("Current Song : %d. %s\n", currentSong+1, SongList[currentSong]);
    printf("\n1: Play");
    printf("\n2: Pause/Resume");
    printf("\n3: Next");
    printf("\n4: Previous");
    printf("\n5: Back to main menu\n\n");
    //printf("Command -> \n");
    switch(Choices)
    {
        case 1:
        //Play Current Song
        //pthread_create(&inputid, NULL, &input, NULL);
        
        if(nowplaying == 0)
        {
            Pause = 0;
            printf("Play : %s\n", SongList[currentSong]);
            pthread_create(&play_song, NULL, &Play, NULL);
        }
        else
        {
            //Interrupt activate
            printf("Press 1 Again to play the Song\n");
            Interrupt = 1;
        }
        
        break;
        case 2:
        //Pause or Resume Current Song
        if(Pause == 0)
        {
            printf("Paused\n");
            Pause = 1;
        }
        else
        {
            printf("Resume\n");
            Pause = 0;
        }
        break;
        case 3:
        //Next Song
        printf("Next Song\n");
        currentSong = (currentSong + 1) % (ListSize);
        Interrupt = 1;
        break;
        case 4:
        //Previous Song
        printf("Previous Song\n");
        currentSong--;
        if(currentSong < 0)
        {
            currentSong += ListSize;
        }
        Interrupt = 1;
        break;
        case 5:
        Interrupt = 1;
        StateMenu = 0;
        break;
    }
    ResetChoice();
}

void ShowRecordMenu()
{
    int cek = system("clear");
    if(cek)return;
    printf("*********** Record List ***********\n");
    DisplayList();
    printf("\nPress 1 to go Back\n");
    switch(Choices)
    {
        case 1:
        StateMenu = 0;
        break;
    }
    ResetChoice();
}

void Inisialiasi()
{

    char curpath[100] = "";
    

    printf("Input FUSE Path : ");
    scanf("%s", fusepath);
    getcwd(curpath, sizeof(curpath));
    sprintf(curpath, "%s/%s", curpath, fusepath);
    //if(cek)return;
    //printf("%s\n", curpath);

    DIR *dp;
    struct dirent *de;

    dp = opendir(curpath);
    if(dp == NULL)
        return;

    while((de = readdir(dp)) != NULL)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        char currentfile[100] = "";
        strcpy(currentfile, de->d_name);
        //printf("%s\n", currentfile);

        int same = 0;
        char extension[5] = ".mp3";
        int length = strlen(currentfile);
        int x;
        for(x = 0; x < 4; x++)
        {
            if(currentfile[length - 4 + x] == extension[x])
            {
                same++;
            }
        }

        if(same == 4)
        {
            strcpy(SongList[ListSize], currentfile);

            ListSize++;
        }
        
    }
}

void DisplayList()
{
    int x;
    for(x = 0; x < ListSize; x++)
    {
        printf("%d. %s\n", x+1, SongList[x]);
    }
}

int main (void)
{
        Inisialiasi();
        //printf("\n");
        //DisplayList();
        
      pthread_create(&inputid, NULL, &input, NULL);
      while(Exit != 1)
      {
        switch(StateMenu)
        {
            case 0:
            //Standby
                StandbyMenu();
            break;
            case 1:
                PlayMenu();
            // Play
            break;
            case 2:
                ShowRecordMenu();
            break;
            case 3:
            //Quit
                exit(EXIT_SUCCESS);
            break;
        }

        sleep(1);
      }
    
      
}
 
