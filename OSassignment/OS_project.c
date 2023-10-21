/*
GROUP MEMBERS:
Aditya Saini(2019A3PS1292H)
Samar Jaish(2019A3PS1309H)
Tushar Tiwari(2019A81332H)
Udit Varshney(2019AAPS0295H)
Vishwajeet Dhankhar(2019A3PS1341H)
Vibhor Singh(2018B1A80831H)
*/
#include <stdio.h>
#include <time.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
struct timespec *exectme;

static int * scheduler;//schedules if FCFS or RR
pid_t ID[3];//stores pid of children

int m_c[3][2];//master to chiled pipes
int c_m[2];//child to master pipes

const int tq = 3;

char SLEEP[] = "SIGSTOP";//sleep signal
char AWAKE[] = "SIGCONT";//awake signal
char FINISHED[] = "SIGFIN";

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;//cond initializer
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;//mutex initializer
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

bool condition1 = false;//raise condition to come out of wait loop
bool condition2 = false;
bool condition3 = false;

bool first1 = true;//flag for entering the process for the first time
bool first2 = true;
bool first3 = true;
bool finished[]={false,false,false};//finish signal array




void* C1(void* arg);//child process function
void* C2(void* arg);//child process function
void* C3(void* arg);//child process function
void* listener(void* arg);//listener process function

int cnt = 0;

#define TRUE 1
#define FALSE 0
#define FULL 100

int* flag;

struct node
{
    pid_t  id;
    struct node *next;
};
typedef struct node node;

struct queue
{
    int count;
    node *front;
    node *rear;
};
typedef struct queue queue;

int isempty(queue *q)
{
    return (q -> count == 0);
}

void enque(queue *q, int value)
{
    if (q->count < FULL)
    {
        node *tmp;
        tmp = malloc(sizeof(node));
        tmp->id = value;
        tmp->next = NULL;
        if(!isempty(q))
        {
            q->rear->next = tmp;
            q->rear = tmp;
        }
        else
        {
            q->front = q->rear = tmp;
        }
        q->count++;
    }
    else
    {
        printf("List is full\n");
    }
}

int deque(queue *q)
{
    node *tmp;
    int n = q->front->id;
    tmp = q->front;
    q->front = q->front->next;
    q->count--;
    free(tmp);
    return(n);
}

int main()
{   
   printf("Enter '0' for FCFS or '1' for Round Robin scheduling\n");
   scheduler = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);                          //shared memory of schedular
   exectme = mmap(NULL,3*sizeof(struct timespec),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
   flag = mmap(NULL,3*sizeof(int),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);                //shared memory execution time
   struct timespec st[3];//starting time array
   struct timespec e[3];//ending time array
   scanf("%d",scheduler);
   printf("Enter the value of n1, which will act as an input for process C1.\n");
   int *n1=malloc(sizeof(*n1));
    scanf("%d",n1);
   for (int i = 0; i < 3; i++)
   {
      pipe(m_c[i]);
      flag[i] = 0;
   }
   pipe(c_m);
   double brst[3];//burst time array
   double tat[3];//turnaround time array
   queue* q = (queue*)malloc(sizeof(queue));//initialize the queue
   q -> count = 0;//initialize the queue
   q -> rear = NULL;//initialize the queue
   q -> front = NULL;//initialize the queue
   ID[0] = fork();//1st chile process creation

   if(ID[0] == 0)//1st child process
   {
      pthread_t t1;
      char readbuffer[80];
      close(m_c[0][1]);

      int flag_prev = 0;

      while(1)
      {

        while(flag_prev == flag[0])
        {
            usleep(10);
        }

        flag_prev = flag[0];

        //  int nbytes = read(m_c[0][0], readbuffer, 8);// read the value wheather to work or not
        //  fflush(stdout);


         if(flag_prev == 1)// on continuing 
         {
            if(first1)
            {
               
               pthread_create(&t1, NULL, &C1, (void *)n1);//creation of working thread of child 1
               condition1 = true;
            }
            else
            {
               condition1 = true;
               pthread_cond_signal(&cond1);//to bring it out of the wait loop
            }
         }
         else if(flag_prev == 0)
         {
            condition1 = false;//getting into the wait loop
         }
      }
   }

   else
   {
      enque(q, ID[0]);
      ID[1] = fork();

      if(ID[1] == 0)//2nd child process
      {
            pthread_t t2;
            char readbuffer[80];
            close(m_c[1][1]);
            int flag_prev = 0;

            while(1)
            {
               while(flag_prev == flag[1])
                {
                    usleep(10);
                }
                flag_prev = flag[1];

               if(flag_prev == 1)
               {
                  if(first2)
                  {
                      pthread_create(&t2, NULL, &C2, NULL);//creation of working thread of child 2
                  }
                  else
                  {
                     condition2 = true;
                     pthread_cond_signal(&cond2);
                     //sleep(3);//to bring it out of the wait loop
                  }
               }
               else if(flag_prev == 0)
               {
                  condition2 = false;//getting into the wait loop
               }
              
         }
      }

      else
      {
         enque(q, ID[1]);
         ID[2] = fork();

         if(ID[2] == 0)//3rd child process
         {
            pthread_t t3;
            char readbuffer[80];
            close(m_c[2][1]);
            int flag_prev = 0;

            while(1 )
            {
               while(flag_prev == flag[2])
                {
                    usleep(10);
                }
                flag_prev = flag[2];

               if(flag_prev == 1)
               {
                  if(first3)
                  {
                     pthread_create(&t3, NULL, &C3, NULL);//creation of working thread of child 3
                  }
                  else
                  {
                     condition3 = true;
                     pthread_cond_signal(&cond3);//to bring it out of the wait loop
                  }
               }
               else if(flag_prev == 0)
               {
                  condition3 = false;//getting into the wait loop
               }
            }
         }

         else//master process
         {
            enque(q, ID[2]);

            close(m_c[0][0]);//closing the reading ends
            close(m_c[1][0]);
            close(m_c[2][0]);

            close(c_m[1]);

            pthread_t list;
            pthread_create(&list, NULL, &listener, NULL);//creating listener thread
            // struct node
            if(*scheduler==0){

                //FCFS
                
                // write(m_c[0][1], SLEEP, (strlen(SLEEP) + 1));//sending all the 3 children to sleep
                // write(m_c[1][1], SLEEP, (strlen(SLEEP) + 1));//sending all the 3 children to sleep
                // write(m_c[2][1], SLEEP, (strlen(SLEEP) + 1));//sending all the 3 children to sleep
                clock_gettime(CLOCK_REALTIME, &st[0]);//noting the starting time of child1
                clock_gettime(CLOCK_REALTIME, &st[1]);//noting the starting time of child2
                clock_gettime(CLOCK_REALTIME, &st[2]);//noting the starting time of child3

               while (!isempty(q))//FCFS
               {
                  pid_t top = deque(q);
                  int i=0;

                  for (i = 0; i < 3; i++)//dequeueing the process
                  {
                     if (top == ID[i])
                     {
                        break;
                     }
                  }
                    printf("Process %d is starting(sending AWAKE)\n", i);
                    flag[i] = 1;
                    //write(m_c[i][1], AWAKE, (strlen(AWAKE) + 1));

                    while(!finished[i]){//waiting until it gets finishing
                    
                    usleep(10);
                }
                         clock_gettime(CLOCK_REALTIME,&e[i]);//noting its end time

                        brst[i]=((double)exectme[i].tv_nsec)/1000000000;//calculating burst time

               }
                            
                            printf("Burst time of 1st child is %f\n",brst[0]);
                            printf("Burst time of 2nd child is %f\n",brst[1]);
                            printf("Burst time of 3rd child is %f\n",brst[2]);
                for(int i=0;i<3;i++){
                    tat[i]=(double)(e[i].tv_sec-st[i].tv_sec)+(((double)(e[i].tv_nsec-st[i].tv_nsec))/1000000000.0);//calculating TAT
                printf("Turnaround Time of %d child is %f\n",i+1,tat[i]);
               }
               for(int i=0;i<3;i++){
                printf("Wait Time of %d child is %f\n",i+1,tat[i]-brst[i]);//calculating WT
               }
            }
            else{
                clock_gettime(CLOCK_REALTIME, &st[0]);//noting the starting time of child1
                clock_gettime(CLOCK_REALTIME, &st[1]);//noting the starting time of child2
                clock_gettime(CLOCK_REALTIME, &st[2]);//noting the starting time of child3

                  while (!isempty(q))
                  {
                     pid_t top = deque(q);//dequeueing the process
                     int i=0;

                     for (i = 0; i < 3; i++)//finding the child via pid
                     {
                        if (top == ID[i])
                        {
                           break;
                        }
                     }



                     printf("Process %d is starting(seding AWAKE)\n", i);//sending awake signal to implement the thread function
                     // fflush(stdout);
                    //  write(m_c[i][1], AWAKE, (strlen(AWAKE) + 1));
                    flag[i] = 1;
                     

                     usleep(100);//waiting for TIME QUANTUM

                     //wait for tq
                     
                     printf("Process %d is sleeping\n",i);
                     fflush(stdout);

                    //  write(m_c[i][1], SLEEP, (strlen(SLEEP) + 1));//sending sleep signal to preampt the child process
                    flag[i] = 0;
                     
                     fflush(stdout);
                  if (finished[i])//on finishing
                     {
                         clock_gettime(CLOCK_REALTIME,&e[i]);//calculating end time
                        printf("Process %d Finished .\n", i);
                        // kill child process
                        fflush(stdout);
                        brst[i]=((double)exectme[i].tv_nsec)/1000000000;//calculating burst time

                     }
                     else{//if not finished queuing it again in the queue
                        //enqueue
                        enque(q,top);
                     }
            }

                printf("Burst time of 1st child is %f\n",brst[0]);
               printf("Burst time of 2nd child is %f\n",brst[1]);
               printf("Burst time of 3rd child is %f\n",brst[2]);
               for(int i=0;i<3;i++){
                tat[i]=(double)(e[i].tv_sec-st[i].tv_sec)+(((double)(e[i].tv_nsec-st[i].tv_nsec))/1000000000.0);//calculating TAT
                printf("Turnaround Time of %d child is %f\n",i+1,tat[i]);
               }
               for(int i=0;i<3;i++){
                printf("Wait Time of %d child is %f\n",i+1,tat[i]-brst[i]);//calculating WT
               }

         }
      }
   }
   }
   return 0;
}
//working thread for child1
void* C1(void* arg){
      clockid_t tid1;
   pthread_getcpuclockid(pthread_self(),&tid1);//getting start time of the child working thread 
   close(c_m[0]);
   int n1 = *((int*) arg);//we have t6o read it from user using pipes
   int sum = 0;
   for(int i = 0; i < n1; i++)
   {
      if(*scheduler == 0){
          first1 = true;
       }
      while(!first1 && !condition1)//if not first and condition=false ,putting it into wait loop unless required
      {
         pthread_cond_wait(&cond1, &mutex1);
      }
      first1 = false;
      printf("Here %d\n", i);
      fflush(stdout);
      sum += i;
   }
   printf("%d\n",sum);
   printf("\nC1 completed\n");
   
   fflush(stdout);
   char fin[] = "00";
   write(c_m[1], fin, strlen(fin) + 1);//sending finish signal to the master  process
   clock_gettime(tid1,&exectme[0]);//finish time

}
//working thread for child2
void* C2(void* arg){
      clockid_t tid2;
   pthread_getcpuclockid(pthread_self(),&tid2);//getting start time of the child working thread
     FILE *myFile;
    myFile = fopen("file_100000.txt", "r");
    int num;
    while(fscanf(myFile, "%d", &num)!=EOF)
    {
       if(*scheduler == 0){
          first2 = true;
       }
             while(!first2 && !condition2)
      {
         pthread_cond_wait(&cond2, &mutex2);
      }
      first2 = false;
        printf("C2: Number is: %d\n\n", num);

    }
   printf("\nC2 completed\n");
   fflush(stdout);
   char fin[] = "01";
   write(c_m[1], fin, strlen(fin) + 1);//sending finish signal to the master  process
   clock_gettime(tid2,&exectme[1]);//finish time

}
//working thread for child3
void* C3(void* arg){
      clockid_t tid3;
   pthread_getcpuclockid(pthread_self(),&tid3);//getting start time of the child working thread
       FILE *myFile;
    myFile = fopen("file_100000 (copy).txt", "r");
    int number;
    int sum=0;

    while(fscanf(myFile, "%d", &number)!=EOF)
    {
       if(*scheduler == 0){
          first3 = true;
       }
      while(!first3 && !condition3)
      {
         pthread_cond_wait(&cond3, &mutex3);
      }
      first3 = false;
        printf("C3: Number is: %d\n\n", number);
        sum=sum+number;

    }
    printf("Sum=%d\n",sum);
   printf("\nC3 completed\n");
   fflush(stdout);
   char fin[] = "10";
   write(c_m[1], fin, strlen(fin) + 1);//sending finish signal to the master  process
   clock_gettime(tid3,&exectme[2]);//finish time

}

void* listener(void* arg)
{
   char readbuffer[80];//reads the finish signals of the corresponding processes and inmidates the master process 
   while(!(finished[0] && finished[1] && finished[2]))
   {
      int nbytes = read(c_m[0], readbuffer, 3);
      printf("rb = %s\n", readbuffer);

      if (readbuffer[0] == '0')
      {
         if (readbuffer[1] == '0')
         {
            finished[0] = true;
         }
         else
         finished[1] = true;
      }
      else
         finished[2] = true;
   }
}