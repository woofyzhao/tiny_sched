#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

enum register_t
{
    rbx,
    rsp,
    rbp,
    r12,
    r13,
    r14,
    r15,
    pc_addr,
};

typedef struct 
{
    /*
     * buffer[0]: rbx
     * buffer[1]: rsp
     * buffer[2]: rbp
     * buffer[3]: r12
     * buffer[4]: r13
     * buffer[5]: r14
     * buffer[6]: r15
     * buffer[7]: Program Counter
     */
    long buffer[8];
} ctx_buf_t;

typedef void *(*thread_handler_t)(void);
typedef struct 
{
  int id;
  void *stack;
  thread_handler_t handler;
  ctx_buf_t ctx; 
} thread_t;

//symbol from ctx.S 
extern int save_context(ctx_buf_t *from);
extern int restore_context(ctx_buf_t *to);

//user thread switch
void do_switch(thread_t *from, thread_t *to)
{
    int ret = save_context(&from->ctx); //保存上下文, 首次返回会设置eax = 0
    if (ret == 0) 
    {
        restore_context(&to->ctx);     //加载to的上下文, jmp之前会设置eax = 1
    }
    else
    {
        //restored from other threads, just return and continue
    }
}

//create a thread
int thread_create(thread_t *t, int id, thread_handler_t handler)
{
    int stack_size = (1 << 20);  //fixed 1M for demo
    void *stack_top = malloc(stack_size);  //or mmap with stack type sepcified

    t->id = id;
    t->stack = stack_top + stack_size; //栈是从上往下增长的，因此栈基址要指向最大处
    t->handler = handler;
    memset(&t->ctx, 0, sizeof(ctx_buf_t));
    t->ctx.buffer[rsp] = (long)t->stack;  //initialize stack pointer
    t->ctx.buffer[pc_addr] = (long)t->handler;  //initialize program counter
    return id;
}

void thread_destory(thread_t *t)
{
    free(t->stack);
}

//start a thread from main
int thread_start(thread_t *t)
{
    restore_context(&t->ctx);

    //never returns
    return -1;
}


thread_t g_thread_A;
thread_t g_thread_B;

void *func_A()
{
    printf("Thread [A] Start running in func_A...\n");

    //infinite loop for demo
    while (1)
    {
        printf("Thread [A] Switching to another thread...\n");

        do_switch(&g_thread_A, &g_thread_B);  //switch to thread B

        printf("Thread [A] returned to func_A, Stack Base[%p]\n", g_thread_A.stack);
        printf("Thread [A] doing stuff...\n");
        sleep(1);  //pretends to be busy in this thread. But this actually blocks the whole process.
    }
    return NULL;
}

void *func_B()
{
    printf("Thread [B] Start running in func_B...\n");

    while (1)
    {
        printf("Thread [B] Switching to another thread...\n");

        do_switch(&g_thread_B, &g_thread_A);  //switch to thread A

        printf("Thread [B] returned to func_B, Stack Base[%p]\n", g_thread_B.stack);
        printf("Thread [B] doing stuff...\n");
        sleep(1);  //pretends to be busy in this thread. But this actually blocks the whole process.
    }
    return NULL;
}


int main()
{
    thread_create(&g_thread_A, 1, func_A);     //A is ready
    thread_create(&g_thread_B, 2, func_B);     //B is ready

    thread_start(&g_thread_A);                 //A is started

    //never reaches here in this demo
    thread_destory(&g_thread_A);
    thread_destory(&g_thread_B);
    return 0;    
}


/*

gcc ctx.S demo.c -o demo

./demo output:

Thread [A] Start running in func_A...
Thread [A] Switching to another thread...
Thread [B] Start running in func_B...
Thread [B] Switching to another thread...
Thread [A] returned to func_A, Stack Base[0x7fd9d9bcc010]
Thread [A] doing stuff...
Thread [A] Switching to another thread...
Thread [B] returned to func_B, Stack Base[0x7fd9d95fa010]
Thread [B] doing stuff...
Thread [B] Switching to another thread...
Thread [A] returned to func_A, Stack Base[0x7fd9d9bcc010]
Thread [A] doing stuff...
...

*/

