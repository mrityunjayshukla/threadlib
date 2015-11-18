#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ucontext.h>
#include <signal.h>
#include <time.h>
#define mem 64000
#define null NULL
#define max1 10

struct mjthread    // thread structure
{
    ucontext_t *context;
    int tid;
};
int c=0;
struct myqueue    //queue structure
{
    struct mjthread * data;
    struct myqueue * next;
};

ucontext_t last,Main;
sigset_t act;   //timer structure
struct myqueue* head=null; //head of linked list
struct myqueue* current=null;
struct mjthread *maintask;
struct itimerval it;
//linked list implementation

void insertfirst(struct mjthread* t)   //insert for first two threads
{
    struct myqueue* temp=(struct myqueue*)malloc(sizeof (struct myqueue));
    if(head==null)
    {
        t->tid=c++;
        temp->data=t;
        head=(struct myqueue*)malloc(sizeof (struct myqueue));
        head=temp;
        current=head;
        temp->next=null;
    }
    else
    {
        t->tid=-1;
        head->next=temp;
        temp->next=null;
        temp->data=t;
    }
}

void insert(struct mjthread* t)  //insert for other threads
{
    struct myqueue* temp=(struct myqueue*)malloc(sizeof(struct myqueue));
    t->tid=c++;
    struct myqueue* ptr;
    ptr=head;
    while(ptr->next->data->tid!=-1)
    {
        ptr=ptr->next;
    }
    temp->data=t;
    temp->next=ptr->next;
    ptr->next=temp;
}

void deletehead()  // delete head node and change head node
{
    struct myqueue* ptr=head;
    ptr=ptr->next;
    printf("%d %d %d id ",head->data->tid,ptr->data->tid,ptr->next->data->tid);
    head=ptr;
    current=ptr;
    setcontext(head->data->context);
    free(ptr);
}

void delete(int id)  // delete node of given id
{
    printf("%d  %d %d",id,current->data->tid,head->data->tid);
    struct myqueue* ptr;
    ptr=head;
    while(ptr->next->data->tid!=id)
        ptr=ptr->next;
    ptr->next=ptr->next->next;
    printf("%d %d id \n",ptr->data->tid,ptr->next->data->tid);
    fflush(stdout);
    current=ptr->next;
    printf("%d\n",current->data->tid);
    setcontext(ptr->next->data->context);
}

void block()   // block all the signals
{
    sigfillset(&act);
    sigprocmask(SIG_BLOCK,&act,null);
}

void unblock()   // unblock all the signals 
{
    sigprocmask(SIG_UNBLOCK,&act,null);
    sigemptyset(&act);
}

void kill_thread()    // to remove the fully executed thread
{
    block();
    printf("removing thread\n");
    if(current==head && current->next->data->tid==-1)
    {
        printf("last element\n");
        it.it_interval.tv_sec=0;
        it.it_interval.tv_usec=0;
        it.it_value.tv_sec=0;
        it.it_value.tv_usec=0;
        setitimer(ITIMER_PROF,&it,null);
        head=null;
        setcontext(current->next->data->context);
    }
    else if(current==head)
    {
        printf("234\n");
        deletehead();
    }
    else
    {
        printf("345\n");
        delete(current->data->tid);
    }
    unblock();
}

void thread_init(struct mjthread* t,int flag)   //threads initialization
{
    t->context=(ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(t->context);
    if(flag==0)
    {
        getcontext(&last);
        last.uc_stack.ss_sp=malloc(mem);
        last.uc_stack.ss_size=mem;
        last.uc_stack.ss_flags=0;
        makecontext(&last,kill_thread,0);
        t->context->uc_link=&last;
    }
    else
        t->context->uc_link=null;
    t->context->uc_stack.ss_sp=malloc(mem);
    t->context->uc_stack.ss_size=mem;
    t->context->uc_stack.ss_flags=0;
}
int fl=0;
void schedule()   // round robin scheduling method
{
    block();
    printf("scheduling thread\n");
    if(current==head)
    {
        if(fl==0)
        {
            printf("first time\n");
            swapcontext(maintask->context,current->data->context);
            fl=1;
        }
        else
        {
            printf("123\n");
            printf("%d\n",current->data->tid);
            struct myqueue* val;
            val=current;
            current=current->next;
            swapcontext(val->data->context,current->data->context);
        }
    }  
    else if(current->next==null)
    {
        struct myqueue* val;
        val=current;
        current=head;
        swapcontext(val->data->context,current->data->context);
    }
    else
    {
        struct myqueue* q=current;
        current=current->next;
        swapcontext(q->data->context,current->data->context);
    }
    unblock();
}


void init_timer(struct itimerval it)  //timer to set the time quantum for round robin scheduling
{
    struct sigaction act,act1;
    act.sa_handler=schedule;
    act.sa_flags=0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF,&act,&act1);
    it.it_interval.tv_sec=0;
    it.it_interval.tv_usec=1000;
    it.it_value.tv_sec=0;
    it.it_value.tv_usec=5000;
    setitimer(ITIMER_PROF,&it,null);
}

void add_to_queue(struct mjthread* t)  // adding new threads to linked list
{
    printf("adding new thread to queue\n");
    block();
    if(head==null)
    {
        insertfirst(t);
        init_timer(it);
        insertfirst(maintask);
    }
    else
    {
        insert(t);
    }
    unblock();
}
int fl1=0;
void thread_create(struct mjthread* t,void (*fun)())  //creation function
{
    if(fl1==0)
    {
        printf("first executing statement\n");
        maintask=(struct mjthread*)malloc(sizeof (struct mjthread));
        thread_init(maintask,1);
        fl1=1;
    }
    printf("request came\n");
    thread_init(t,0);
    makecontext(t->context,fun,0);
    printf("init done\n");
    add_to_queue(t);
    printf("added to the queue\n");
}