
/*
 *
 * CSEE 4840 Lab 2 for 2019
 *
 * Name/UNI: Prathmesh Patel(pp2870), Tharun Kumar Jayaprakash(tj2557), Rishit Thakkar(rht2122)
 */
#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>

/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* arthur.cs.columbia.edu */
#define SERVER_HOST "128.59.19.114"
#define SERVER_PORT 42000
#define RLEN 64
#define CLEN 24
#define ROW_CLIENT_START 21
#define COL_LIMIT 63
#define BUFFER_SIZE 128
#define CHAT_LIMIT 18
#define CLIENT_LIMIT 22
/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */
//char displayChar;

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;
void *network_thread_f(void *);

void clearScreen(){
for(int row = 0; row < 24; row++){
	for(int col = 0; col < 64; col++){
	fbputchar(' ', row, col);
}
}
}


//check if colPointer is oging past 63, if so, make it 0
int colCheck(int incoming){
	if(incoming == 64)
	return 0;
	else
	return incoming;
}

void clearRow(int row){
	for(int i = 0; i < 64; i++){
		fbputchar(' ', row, i);
	}
}

void clear_client_side(){
for (int i = 21; i < 23; i++){
	for(int j = 0; j < 64; j++){
		fbputchar(' ', i, j);
	}
} 
}

/*void updateDisplay(int r, char *buff, int chars){
	int c = 0;
	if(chars>=64){
		for(int i = 0; i < chars; i++){
			fbputchar(*(buff+i), r, c);
		}
		if(i == 64)
		r+1; c = 0;
	}
	else
		fbputs(buff, r, 0);
}*/

int main()
{
  
  int colPointer = 0;
  int rowPointer = 21;
  int err, col;
  int count=0;
  int lastString = 21; //  rmembers where the last printed string ended
  int upDownTrack = 0;
  int col_word_tracker = 0;
  int numChars = 0;
  //char displayChar;
  char writeBuffer[134];
  struct sockaddr_in serv_addr;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[16];
    

  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }

  clearScreen();
  /* Draw rows of asterisks across the top and bottom of the screen */
  for (col = 0 ; col < 64 ; col++) {
    fbputchar('*',0,col);
    fbputchar('*',23,col);
    fbputchar('/',20,col);
  }

  fbputs("Messages Sent by You..", 19, 10);

  /* Open the keyboard */
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find a keyboard\n");
    exit(1);
  }
    
  /* Create a TCP communications socket */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    fprintf(stderr, "Error: Could not create socket\n");
    exit(1);
  }

  /* Get the server address */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
    exit(1);
  }

  /* Connect the socket to the server */
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
    exit(1);
  }

  /* Start the network thread */
  pthread_create(&network_thread, NULL, network_thread_f, NULL);


  /* Look for and handle keypresses */
  for (;;) {
    libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);
    if (transferred == sizeof(packet)) {
      /*sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
	      packet.keycode[1]);*/
      printf("%s\n", keystate);
      //fbputs("Hello", 6, 0);
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
	break;
      }

      else if(packet.keycode[0] == KEY_ENTER){   //enter key
        write(sockfd, writeBuffer, numChars);
	//fbputchar(' ', rowPointer+upDownTrack, numChars);
	//rowPointer += upDownTrack;
	//upDownTrack = 0;
	colPointer = numChars;
	if(rowPointer >= CLIENT_LIMIT){
                rowPointer = ROW_CLIENT_START;
		lastString = ROW_CLIENT_START;
		clear_client_side();
        }
	/////////////////////////////////////////////////////////////////////CHECKTHIS*************************************
	//if(lastString != ROW_CLIENT_START)
	//rowPointer = lastString+1;
	

	if(numChars >= 64){
		//int r = rowPointer;
		/*if(rowPointer-lastString >=2)
		r = rowPointer;
		else
		r = rowPointer-1;*/
		for(int j = 0; j < numChars; j++){
			fbputchar(writeBuffer[j], rowPointer, j);

			if(j == 63)
			  rowPointer++;
		}

	}
	else 
		fbputs(writeBuffer, rowPointer, 0);
		clear_client_side();
		rowPointer=ROW_CLIENT_START;
		lastString=ROW_CLIENT_START;

	for(int i = 0; i < sizeof(writeBuffer); i++){
		writeBuffer[i] = '\0';
	}

	count = 0;
	if(numChars < 127)
		fbputchar(' ', rowPointer,colPointer);
	
	lastString = rowPointer;
	//rowPointer++;
	numChars = 0;
	colPointer = 0;///////CHANGE THIS LATER WHEN YOU ADD STUFF TO UI TO MAKE IT BETTER
 	
	/*if(rowPointer == CLIENT_LIMIT){
		clear_client_side();
		rowPointer = ROW_CLIENT_START;
	}*/
	fbputcursor(' ', rowPointer, colPointer);
	
      }

      //else if(packet.keycode[0] == KEY_UP){
        /*if(writeBuffer[count] != '\0')
                fbputchar(writeBuffer[count], rowPointer, colPointer);
        else
                fbputchar(' ', rowPointer, colPointer);
	*/
        //count -= 63;
       // rowPointer-=1;
	//upDownTrack++;
        
      //  fbputcursor(' ', rowPointer, colPointer);
      //}


      else if(packet.keycode[0] == KEY_RIGHT){
        if(writeBuffer[count] != '\0'){
                fbputchar(writeBuffer[count], rowPointer, colPointer);
        //else
                //fbputchar(' ', rowPointer, colPointer);

	if(colPointer == 63 && numChars >= 64){
                colPointer = 0;
                rowPointer++;

                fbputcursor(' ', rowPointer, colPointer);
                count++;

        }

	else{
        count++;
        colPointer++;
	colPointer = colCheck(colPointer);
        fbputcursor(' ', rowPointer, colPointer);
	}
	}
      }


      else if(packet.keycode[0] == KEY_LEFT){

	/*if(colPointer == 0 && numChars >= 64){
		colPointer = 63;
		rowPointer--;
		
	}*/
	
	if(writeBuffer[count] != '\0')
		fbputchar(writeBuffer[count], rowPointer, colPointer);
	else
		fbputchar(' ', rowPointer, colPointer);
	
	if(colPointer == 0 && numChars >= 64){
                colPointer = 63;
                rowPointer--;
		
		fbputcursor(' ', rowPointer, colPointer);
		count--;

        }
	else{
	count--;
	colPointer--;
	fbputcursor(' ', rowPointer, colPointer);
	}
      }
      else if(packet.keycode[0] == KEY_BACKSPACE){//backspace
		if(count !=0){
			//numChars--;
			//writeBuffer[count] = '\0';
			//fbputchar(' ', rowPointer,colPointer);
			//writeBuffer[numChar-1] = '\0';
			//fbputchar(' ', rowPointer, colPointer);
			
			if(colPointer ==0){
				colPointer = 63;
				rowPointer = 21;
				fbputchar(' ', 22, 0);

			}
			else{
				fbputchar(' ', rowPointer, colPointer);
				colPointer--;
				
			}
			/*fbputchar(' ', rowPointer,colPointer);
			fbputcursor(' ',rowPointer, colPointer);
			count--;*/
			if(writeBuffer[count+1] != '\0'){
				for(int i = count-1; i < numChars; i++){
					writeBuffer[i] = writeBuffer[i+1];
				} 
			}
			writeBuffer[numChars-1] = '\0';

			numChars--;
			//clearRow(rowPointer);

			if(numChars > 64){

				int r = 21;
				//if(lastString != ROW_CLIENT_START) 
				//	r = lastString+1;
				int c = 0;
				for(int i = 0; i < numChars; i++){
					fbputchar(writeBuffer[i], r, c);
					c++;
					if(c == 64){
					 r++;c=0;}
				}
				fbputcursor(' ', rowPointer, colPointer);
				fbputchar(' ', r, c);
			}
	 
			else
				fbputs(writeBuffer, 21, 0);

			if(numChars == 64){
                                fbputchar(' ', 22, 0); fbputchar(' ', 22, 1);}
			
			else if(numChars != 63) 
				fbputchar(' ', rowPointer, colPointer+1);

			count--;
			fbputcursor(' ', rowPointer, colPointer);
		}
	
	}

	else if(packet.keycode[0] != 0x00){
	if(packet.keycode[1]==packet.keycode[2])
	{
	if(numChars < 128){
	//writeBuffer[count] = keyValue(packet.modifiers, packet.keycode[0]);
	numChars++;

	if(count < numChars-1){
		//writeBuffer[numChars] = ' ';
		for(int i = numChars; i > count; i--){
			writeBuffer[i] = writeBuffer[i-1];
		}
		writeBuffer[count] = keyValue(packet.modifiers, packet.keycode[0]);
		//updateDisplay(rowPointer, writeBuffer, numChars);
		int c = 0;
		int r = 21;
		for(int i = 0; i < numChars; i++){
			fbputchar(writeBuffer[i], r, c);
			c++;
			if(c == 64){
			r++;c=0;
			}
		}
	}

	else{
	writeBuffer[count] = keyValue(packet.modifiers, packet.keycode[0]);
	//ADD FUNC TO SHIFT ARAY
	fbputchar(writeBuffer[count], rowPointer,colPointer);
	//counter = count;
	}
     }

	if(colPointer == COL_LIMIT && numChars >=127){
		count++;
		fbputcursor(' ',rowPointer, colPointer);
	}
	if(colPointer == COL_LIMIT && numChars < 127){
		rowPointer++;
		count++;

		if(rowPointer > CLIENT_LIMIT){
			clear_client_side();
			rowPointer = ROW_CLIENT_START;
		}

		colPointer = 0;
		fbputcursor(' ',rowPointer, colPointer);
	}

	else if (colPointer < COL_LIMIT){
		/*if(rowPointer == CLIENT_LIMIT){
			clear_client_side();
			rowPointer = ROW_CLIENT_START;
		}*/
	fbputcursor(' ',rowPointer, colPointer+1);
        	//if(numChars != 127)
	colPointer++;
	count++;
	}
      }
      }
    }
 // }
}

  /* Terminate the network thread */
  pthread_cancel(network_thread);

  /* Wait for the network thread to finish */
  pthread_join(network_thread, NULL);

  return 0;
}

void clear_chat_side(){
        for(int i = 0; i < CHAT_LIMIT+1; i++){
                for(int j = 0; j < 64; j++){
                       fbputchar(' ', i, j);
                }
        }

}

void *network_thread_f(void *ignored)
{
  char recvBuf[BUFFER_SIZE];
  int n;
  int row_server_tracker = 1;
  /*Receive data*/

  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
   if(recvBuf[n]=='\0'){
	n=n-1;
}
    recvBuf[n] = '\0';
    printf("%s", recvBuf);
    //fbputs(recvBuf, 1, row_server_tracker);
    if(n >= 63){
	if(row_server_tracker >= CHAT_LIMIT){
		clear_chat_side();
		row_server_tracker = 1;
	}
	int col_temp = 0;
	for(int k = 0; k < n; k++){
		if(k ==64){
			row_server_tracker++;col_temp = 0;}

		fbputchar(recvBuf[k], row_server_tracker,col_temp);
		col_temp++;
	}
	row_server_tracker++;
    }

    else{
    	if(row_server_tracker >= CHAT_LIMIT){
		clear_chat_side();
		row_server_tracker = 1;
    	}

    	fbputs(recvBuf, row_server_tracker, 0);

    	row_server_tracker++;
	}
	for(int i = 0; i < sizeof(recvBuf); i++){
		recvBuf[i] = '\0';
	}
 }

  return NULL;
}

