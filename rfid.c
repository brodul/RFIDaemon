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

#include <stdlib.h>
#include <curl/curl.h> /* The network library */




int send_json(unsigned int result)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	char json[128];

	// Result in string
	char sresult[30];
	// Mac in string
	char smac[30];
	int f;
	int status;

	sprintf(sresult,"%d",result);

	f = open( "/sys/class/net/eth0/address", O_RDONLY );
	printf("%d\n", f);
	status = read(f, smac, 18);
	printf("%d\n", status);
	close(f);

	smac[17] = 0;

	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L );
		/*curl_easy_setopt(curl, CURLOPT_URL, "https://openvz.brodul.org");*/
		curl_easy_setopt(curl, CURLOPT_URL, "localhost/listener.php");
		/*curl_easy_setopt(curl, CURLOPT_URL, "localhost");*/
		/*curl_easy_setopt(curl, CURLOPT_URL, "http://www.postbin.org/qpss4f");*/
		curl_easy_setopt(curl, CURLOPT_PORT, 8000 );
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		strcpy (json,"{ ");
		strcat (json,"\"mac\" : \"");
		strcat (json, smac);
		strcat (json,"\", \"rfid\" : \"");
		strcat (json, sresult);
		strcat (json,"\"}");



		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json );

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		printf("%d", res);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;
}

int open_port(void)
{
	int fd;                                   /* File descriptor for the port */

	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY); /*Temporaly change to USB0*/

	if (fd == -1) {                                              /* Could not open the port */
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
	unsigned short int tempsum; /*Temp var for calculating checksum*/


	unsigned char tmp=0;
	for (i=0;i<5;i++)
	{
		tmp ^= ch[i];
	}

	tempsum = tmp;

	if (ch[5] == tmp)
		state_ch = 1;
	else
		state_ch = 0;

#ifdef DEBUG
	printf("%x\n",tempsum);
	printf("%x\n",ch[5]);
#endif

	return state_ch;
}


void main()
{
	int mainfd=0;                                            /* File descriptor */
	int short i;
	int short state;
	unsigned char chout[15];
	unsigned char hextable[7];
	chout[14] = 0 ;
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
	/*unsigned short int alfa; [>Temp var for calculating checksum<]*/
	/*unsigned short int beta; [>Temp var for calculating checksum<]*/
	/*unsigned short int tempsum; [>Temp var for calculating checksum<]*/
	unsigned int result; /*Printed ID on RFID tag*/
	unsigned char tmpa=0;
	unsigned char tmpb=0;


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

		for(i=1;i<14;i=i+2)
		{
			/*Translation from ASCII to hex*/
			if (47 < chout[i] && chout[i] < 58)
				tmpa = chout[i] - 48;
			if (64 < chout[i] && chout[i] < 71)
				tmpa = chout[i] - 55;

			if (47 < chout[i+1] && chout[i+1] < 58)
				tmpb = chout[i+1] - 48;
			if (64 < chout[i+1] && chout[i+1] < 71)
				tmpb = chout[i+1] - 55;

			hextable[i/2] = (tmpa << 4) + tmpb;
#ifdef DEBUG
			printf("H:%i %x\n", i/2, hextable[i/2]);
#endif

		}

		counter = 0;

		if (chout[0] == 0x02)
		{

			state = checksum_calculation(hextable);
			if (state)
			{
#ifdef DEBUG
				printf("\nChecksum OK\n");

				printf("%s\n", chout);
				for(i=0;i<14;i++)
				{
					printf("%x-", chout[i]);
					printf("%c:", chout[i]);
				}
				printf("\n");
#endif

				result = ((  hextable[1] << 24) | ((  hextable[2] << 16) ) | ((  hextable[3]  << 8) ) | (hextable[4] ));

				printf("%i\n", result);
				send_json(result);

			}
			else printf("Checksum BAD :(");

		}
		else printf("Start byte is not 0x02");

	}
	close(mainfd);
}

