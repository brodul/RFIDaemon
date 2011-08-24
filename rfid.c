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
#include <math.h>

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int open_port(void)
{
    int fd;                                   /* File descriptor for the port */

    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1)
    {                                              /* Could not open the port */
        fprintf(stderr, "open_port: Unable to open /dev/ttyS1 - %s\n",
                strerror(errno));
    }

    return (fd);
}

void main()
{
    int mainfd=0;                                            /* File descriptor */
    int i;
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

    int r=-1;
    int counter=0;
    int m=0;
    unsigned short int alfa;
    unsigned short int beta;
    unsigned short int tempsum;
    unsigned long int result;

    while (1)
    {
    result = 0;

        while (1)
        {
            r = read(mainfd, &chout[counter], 1);  /* Read character from ABU */

            if (r > 0)
            {
                /*printf("\nr = %d\n",r);*/
                counter++;
            }

            if (counter == 14)
                break;
            usleep(20000);
        }

        counter = 0;
        m = 0;
        for(i=3;i<11;i++)
        {
            /*value = (value | chout[i]) << 8;*/
            printf("%d\n", chout[i]);
            /*printf("%x\n", value);*/
        }

        /*printf("\n%llu\n", value);*/


        if (chout[0] == 0x02)
        {
            ((char *) (&alfa))[0] = chout[1];
            ((char *) (&alfa))[1] = chout[2];
            ((char *) (&beta))[0] = chout[3];
            ((char *) (&beta))[1] = chout[4];
            tempsum = alfa ^ beta;
            ((char *) (&alfa))[0] = chout[5];
            ((char *) (&alfa))[1] = chout[6];
            tempsum = alfa ^ tempsum;
            ((char *) (&alfa))[0] = chout[7];
            ((char *) (&alfa))[1] = chout[8];
            tempsum = alfa ^ tempsum;
            ((char *) (&alfa))[0] = chout[9];
            ((char *) (&alfa))[1] = chout[10];
            tempsum = alfa ^ tempsum;

            if ( chout[11] == ((char *) (&tempsum))[0] && chout[12] == ("%c\n",((char *) (&tempsum))[1]))
            {
                printf("\nChecksum OK\n");
                for(i=3;i<11;i++)
                {
                    /*value = (value | chout[i]) << 8;*/
                    if (47 < chout[i] && chout[i] < 58)
                        chout[i] = chout[i] - 48;
                    if (64 < chout[i] && chout[i] < 71)
                        chout[i] = chout[i] - 55;
                    result += pow(16,10-i) * chout[i];


                    /*printf("%x\n", value);*/
                }
                printf("%lu\n", result);
            }
            printf("\n-----\n");
        }



    }
    close(mainfd);
}

