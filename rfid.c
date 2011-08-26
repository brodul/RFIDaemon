/*
From:
http://slackware.osuosl.org/slackware-3.3/docs/mini/Serial-Port-Programming

Better port reading program
v1.0
23-10-96

This test program uses quasi-POSIX compliant UNIX functions to
open the ABU port and read.

Uses termio functions to initialise the port to 9600 baud, at
8 data bits, no parity, no hardware flow control,
and features character buffering.
The 20ms delay after the port read indicates that characters are
buffered if a button is pressed many times.

This program was derived from instructions at
http://www.easysw.com/~mike/serial/
*/

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <math.h>    /*The math lib*/

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int open_port(void)
{
    int fd;                                   /* File descriptor for the port */

    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY); /*Temporaly change to USB0*/

    if (fd == -1)
    {                                              /* Could not open the port */
        fprintf(stderr, "open_port: Unable to open /dev/ttyUSB0 - %s\n",
                strerror(errno));
    }

    return (fd);
}


/*Checksum calculation*/
int checksum_calculation(unsigned char * ch);
int checksum_calculation(unsigned char * ch)
{
    unsigned short int i;
    short int state_ch;
    unsigned short int alfa; /*Temp var for calculating checksum*/
    unsigned short int tempsum; /*Temp var for calculating checksum*/

    ((char *) (&tempsum))[0] = ch[1];
    ((char *) (&tempsum))[1] = ch[2];

    for(i = 2;i < 6;i++)
    {
        ((char *) (&alfa))[0] = ch[i * 2 - 1];
        ((char *) (&alfa))[1] = ch[i * 2];
        tempsum = alfa ^ tempsum;
    }

    if ( ch[11] == ((char *) (&tempsum))[0] && ch[12] == (((char *) (&tempsum))[1]) )
    state_ch = 1;
    else
    state_ch = 0;

    printf("%x\n",tempsum);
    printf("%x\n",ch[11]);
    printf("%x\n",ch[12]);

    return state_ch;
}


void main()
{
    int mainfd=0;                                            /* File descriptor */
    int short i;
    int short state;
    unsigned char chout[13];
    struct termios options;

    mainfd = open_port();

    fcntl(mainfd, F_SETFL, FNDELAY);                  /* Configure port reading */
    /* Get the current options for the port */
    tcgetattr(mainfd, &options);
    cfsetispeed(&options, B9600);                 /* Set the baud rates to 9600 */
    cfsetospeed(&options, B9600);

    /* Enable the receiver and set local mode */
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |=  CS8;                              /* Select 8 data bits */
    options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */

    /* Enable data to be processed as raw input */
    options.c_lflag &= ~(ICANON | ECHO | ISIG);

    /* Set the new options for the port */
    tcsetattr(mainfd, TCSANOW, &options);

    int r=-1; /*The read object*/
    int counter=0; /*Byte count for reading */
    unsigned short int alfa; /*Temp var for calculating checksum*/
    unsigned short int beta; /*Temp var for calculating checksum*/
    unsigned short int tempsum; /*Temp var for calculating checksum*/
    unsigned long int result; /*Printed ID on RFID tag*/

    while (1)
    {
        result = 0;

        /*Loop reading 14 bytes*/
        while (1)
        {
            r = read(mainfd, &chout[counter], 1);  /* Read character from ABU */

            if (r > 0) /*If there is no error*/
            {
                counter++;
            }

            if (counter == 14)
                break;
            usleep(20000);
        }

        counter = 0;

        if (chout[0] == 0x02)
        {
            state = checksum_calculation(chout);
            printf("%d", state);

            /*if (state)*/
            if (1)
            {
                printf("\nChecksum OK\n");
                for(i=3;i<11;i++)
                {
                    /*Translation from ASCII to hex*/
                    if (47 < chout[i] && chout[i] < 58)
                        chout[i] = chout[i] - 48;
                    if (64 < chout[i] && chout[i] < 71)
                        chout[i] = chout[i] - 55;
                    /*Dirty*/
                    result += pow(16,10-i) * chout[i];

                }
                printf("%lu\n", result);
            }
            else printf("Checksum BAD :(");

            printf("\n-----\n");
        }
        else printf("Start byte is not 0x02");

    }
    close(mainfd);
}

