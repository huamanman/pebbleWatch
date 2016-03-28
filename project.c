#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

double tem,average;
double low = 1000;
double high = -1000;
double sum = 0;
int count = 0;

pthread_mutex_t lock;


void* myFun(void*p){



    //open the arduino
	int fd = open("/dev/cu.usbmodem1411", O_RDWR);
	if(fd==-1){
		printf("OOPS!\n");
		exit(1);
	}

	////// CONFIG options from instructions PDF
	struct termios options;
	// struct to hold options
	tcgetattr(fd, &options);
	// associate with this fd
	cfsetispeed(&options, 9600); // set input baud rate
	cfsetospeed(&options, 9600); // set output baud rate
	tcsetattr(fd, TCSANOW, &options); // set options

	//LOOP that is continuiously reading from arduino and writing to the screen

    
	int buffer_loc = 0;
	char read_buffer[200];
	char accum_buffer[100];
	char* token;
	int count;
	while(1){
		count = 3;
		int bytes_read = read(fd, read_buffer, 200);	
		if(bytes_read > 0){
//			printf("DEBUG: %d bytes read\n", bytes_read);
			int i;
			for(i = 0; i<bytes_read; i++){
//				printf("DEBUG: char %c read\n", read_buffer[i]);
				if(read_buffer[i]!='\n'){
					accum_buffer[buffer_loc] = read_buffer[i];
//					printf("DEBUG: no NEWLINE |%s|\n", accum_buffer);
					buffer_loc++;
				}
				else{
//					printf("DEBUG: yes NEWLINE |%s|\n", accum_buffer);
					if(buffer_loc != 0 ){
						accum_buffer[buffer_loc] = '\0';
						printf("%s\n",accum_buffer);
						token = strtok(accum_buffer," ");
						/*while(count >0){
							token = strtok(NULL," ");
							count--;
						}*/
						token = strtok(NULL," ");
						token = strtok(NULL," ");
						token = strtok(NULL," ");

						pthread_mutex_lock(&lock);
						if(token != NULL){
							tem = atof(token);
							sum = sum + tem;
							if(tem>high){
								high = tem;
							}
							if(tem<low){
								low = tem;
							}
							count++;
							if(count!=0){
								average = sum/count;
							}
						}
						pthread_mutex_unlock(&lock);
						printf("%f %f %f\n", tem,low,high);
						
						for(int j = 0; j<100; j++){
							accum_buffer[j] = '\0';
						}
						buffer_loc = 0;	
						}
									
				}
			}
//                        printf("for loop endd\n");
		}
	}
	return NULL;
}

int start_server(int PORT_NUMBER)
{

      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;    
      
      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Socket");
	exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
	perror("Setsockopt");
	exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUMBER); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 
      
      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
	perror("Unable to bind");
	exit(1);
      }

      // 3. listen: indicates that we want to listn to the port to which we bound; second arg is number of allowed connections
      if (listen(sock, 5) == -1) {
	perror("Listen");
	exit(1);
      }
          
      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
      fflush(stdout);
     
      while(1){
      	  // 4. accept: wait here until we get a connection on that port
	      int sin_size = sizeof(struct sockaddr_in);
	      int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
	      printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
	      
	      // buffer to read data into
	      char request[1024];
	      
	      // 5. recv: read incoming message into buffer
	      int bytes_received = recv(fd,request,1024,0);
	      // null-terminate the string
	      request[bytes_received] = '\0';
	      printf("Here comes the message:\n");
	      printf("%s\n", request);
          char reply [100];
	      // this is the message that we'll send back
	      /* it actually looks like this:
	        {}
	           "name": "cit595"
	        }
	      */
	      pthread_mutex_lock(&lock);
	      //strcpy(reply,tem);
	      /*sprintf(one,"{\n\"name\":\"Current temp:%f\n",tem);
	      sprintf(two,"Low temp:%f\"\n}\n",low);
	      strcpy(reply,one);
	      strcat(reply,two);*/
	      sprintf(reply,"{\n\"name\":\"Current temp: %.2f\\nLow temp is: %.2f\\nHigh temp is: %.2f\\nAverage temp is: %.2f\"\n}\n",tem,low,high,average);
	      pthread_mutex_unlock(&lock);
          printf("here\n");
	      
	      // 6. send: send the message over the socket
	      // note that the second argument is a char*, and the third is the number of chars
	      send(fd, reply, strlen(reply), 0);
	      //printf("Server sent message: %s\n", reply);
	      printf("already sent\n");

	      // 7. close: close the socket connection
	      close(fd);
      }
      
      close(sock);
      printf("Server closed connection\n");
  
      return 0;
} 


int main(int argc, char *argv[])
{
  // check the number of arguments
  if (argc != 2)
    {
      printf("\nUsage: server [port_number]\n");
      exit(0);
    }
 
  int PORT_NUMBER = atoi(argv[1]);
  pthread_t t;
  pthread_create(&t,NULL,&myFun,NULL);
  start_server(PORT_NUMBER);
  pthread_join(t,NULL);
}
