#include "MQTT_frames.h"

pthread_mutex_t mutex;
int fd;
long kA;

f_PingRequest reqPing_frame;

f_Connect create_ConnectF(char username[]){
    f_Connect frame;

    frame.bFrameType = 0x10;
    frame.wMsgLen = sizeof(f_Connect);
    frame.wProtlNameLen = 0x0004;
    strcpy(frame.sProtName,"MQTT");
    frame.bVersion = 0x02;
    frame.bConnectFlags = 0x04;
    frame.bKeepAlive = kA;
    frame.wClientIdLen = strlen(username);
    strcpy(frame.sClientID, username);

    return frame;
}

f_PingRequest create_PReq(){
    f_PingRequest frame;

    frame.bFrameType = 0xC0;
    frame.bkeepAlive = 0x00;

    return frame;
}

f_Subscribe create_Sub(uint8_t topic){
    f_Subscribe frame;

    frame.bFrameType = 0x80;
    frame.bRemainLen = 0x01;
    frame.bTopic = topic;

    return frame;
}

f_Publish create_Pub(uint8_t topic, char msg[]){
    f_Publish frame;

    frame.bFrameType = 0x30;
    frame.bRemainLen = 0x02;
    frame.bTopic = topic;
    strcpy(frame.sMsg, msg);

    return frame;
}

int menu(){
   int option;
   printf("======================MENU========================\n");
   printf("1. Subscribe\n2. Publish\n3. Disconnect\n");
   printf("==================================================\n");
   printf("- ");
   fflush(stdin);
   scanf("%i",&option);

   return option;
}

void connectServ(){
   struct sockaddr_in server;
   struct hostent *lh;//localhost

   if ((fd = socket(AF_INET, SOCK_STREAM, 0))==-1){
      printf("failed to create socket\n");
      exit(-1);
   }

   if ((lh=gethostbyname("localhost")) == NULL){
      printf("failed to get hostname\n");
      exit(-1);
   }//obtain localhost

   server.sin_family = AF_INET;
   server.sin_port = htons(PORT);
   server.sin_addr = *((struct in_addr *)lh->h_addr);
   //bzero(&(server.sin_zero),8);

   if(connect(fd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1){
      printf("failed to connect\n");
      exit(-1);
   }
}

void timer_handler (int signum)
{
   send(fd,(char *)&reqPing_frame,sizeof(f_PingRequest),0);
}

void *timer_count(void *param){
   //Timer Variables
   struct sigaction sa;
	struct itimerval timer;

   //Install timer_handler as the signal handler for SIGVTALRM.
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGVTALRM, &sa, NULL);
	
	//Configure the timer to expire after seconds
	timer.it_value.tv_sec = kA/2;
	timer.it_value.tv_usec = 0;
	
	//and every certain seconds after that
	timer.it_interval.tv_sec = kA/2;
	timer.it_interval.tv_usec = 0;

   setitimer(ITIMER_VIRTUAL, &timer, NULL);//Timer start

   while(1);//Goes into infinite cycle so the timer can go off
}

void *receive_frame(void *param){
   int numbytes;
   f_Publish pub_frame;
   f_PingResponse resPing_frame;
   f_PubAcknowledge pubAck_frame;
   f_SubAcknowledge subAck_frame;

   while((numbytes=recv(fd,(char *)&pub_frame,sizeof(f_Publish),0)) > 0){
      if(pub_frame.bFrameType == 0xD0){
         resPing_frame.bFrameType = pub_frame.bFrameType;
         resPing_frame.bresponse = pub_frame.bRemainLen;
         if(resPing_frame.bresponse == 0x00){
            continue;
         }
      }else if(pub_frame.bFrameType == 0x40){
         pubAck_frame.bFrameType = pub_frame.bFrameType;
         pubAck_frame.bRemainLen = pub_frame.bRemainLen;
         if(pubAck_frame.bRemainLen == 0x00){
            printf("Publish successful\n");
         }
      }else if(pub_frame.bFrameType == 0x90){
         subAck_frame.bFrameType = pub_frame.bFrameType;
         subAck_frame.bRemainLen = pub_frame.bRemainLen;
         subAck_frame.bResponse = pub_frame.bTopic;
         if(subAck_frame.bResponse == 0x01){
            printf("You're already subscribed to that topic\n");
         }else if(subAck_frame.bResponse == 0x00){
            printf("Subscription successful\n");
         }
      }else if(pub_frame.bFrameType == 0x30){
         if(pub_frame.bTopic == 0x00){
            printf("Fashion: %s\n\n", pub_frame.sMsg);
         }else if(pub_frame.bTopic == 0x01){
            printf("Food: %s\n\n", pub_frame.sMsg);
         }else if(pub_frame.bTopic == 0x02){
            printf("Music: %s\n\n", pub_frame.sMsg);
         }
      }
   }
   return 0;
}

int main(int argc, char *argv[])
{
   int numbytes, option, topicNum;
   char message[50], username[20];
   pthread_t thread, thread2;
   
   //Frames
   f_Connect conn_frame;
   f_ConnAcknowledge connack_frame;
   f_Subscribe sub_frame;
   f_Publish pub_frame;
   reqPing_frame = create_PReq();

   connectServ();//Connection to server

   //Create frame with user inputs
   system("clear");
   printf("Username: ");
   scanf(" %19[^\n]", username);
   printf("Keep alive value: ");
   scanf("%li", &kA);
   conn_frame = create_ConnectF(username);

   if(pthread_mutex_init (&mutex, NULL) != 0){
      printf("Failed to initialize mutex");
   }//Initialize mutex variables

   send(fd,(char *)&conn_frame,sizeof(f_Connect),0);
   puts("Sent connect frame\n");
   if ((numbytes=recv(fd,(char *)&connack_frame,sizeof(f_ConnAcknowledge),0)) == -1){
      printf("failed to recieve\n");
   }//Recieve response frame
   
   if(connack_frame.bFrameType == 0x20 &&  connack_frame.bReturnCode == 0x00){
      printf("Connect frame successful\n");
      if(pthread_create(&thread, NULL, timer_count, NULL) < 0) {
         perror("Thread creation failed");
         exit(-1);
      }
   }
   else{
      printf("Connect frame failed");
      exit(-1);
   }

   if(pthread_create(&thread2, NULL, receive_frame, NULL) < 0) {
      perror("Thread creation failed");
      exit(-1);
   }//Create thread to recieve frames

   while(option != 3){
         option = menu();

         switch(option){
         case 1:
            printf("Which topic would you like to subscribe to?\n");
            printf("========================\n");
            printf("0. Fashion\n1. Food\n2. Music\n");
            printf("========================\n- ");
            scanf("%i", &topicNum);

            if(topicNum != 0 && topicNum != 1 && topicNum != 2){
               printf("Invalid topic!\n");
               break;
            }else{
               if(pthread_mutex_lock(&mutex) == 0){
                  sub_frame = create_Sub(topicNum);//Create subscribe frame with chosen topic
                  send(fd,(char *)&sub_frame,sizeof(f_Subscribe),0);
                  pthread_mutex_unlock(&mutex);
               }
               break;
            }
         case 2:
            printf("Which topic would you like to publish to?\n");
            printf("========================\n");
            printf("0. Fashion\n1. Food\n2. Music\n");
            printf("========================\n- ");
            scanf("%i", &topicNum);
            printf("Message to publish: ");
            scanf(" %49[^\n]", message);

            if(topicNum != 0 && topicNum != 1 && topicNum != 2){
               printf("Invalid topic!\n");
               break;
            }else{
               if(pthread_mutex_lock(&mutex) == 0){
                  pub_frame = create_Pub(topicNum, message);//Create publish frame with chosen topic and message
                  send(fd,(char *)&pub_frame,sizeof(f_Publish),0);
                  pthread_mutex_unlock(&mutex);
               }
               break;
            }
         
         case 3:
            close(fd);
            break;

         default:
            printf("Option not valid");
            break;
         }
   }

   return 0;
}