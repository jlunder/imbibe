#include "system.h"


#ifndef NDEBUG


#endif


extern main_thread(void *);


__segment system_seg;


int main(int argc, char * argv[])
{
  system_seg = _bheapseg();
  _bfreeseg(system_seg);
  return 0;
}


thread_id thread_create(void (* thread_func)(void *), void * context);
int thread_destroy(thread_id thread);
int thread_execute(thread_id thread);
int thread_suspend(thread_id thread);
int thread_cancel(thread_id thread);
int thread_join(thread_id thread);
int thread_detach(thread_id thread);
int thread_poll(thread_id thread);
int thread_is_attached(thread_id thread);

mutex_id mutex_create();
int mutex_destroy(mutex_id mutex);
int mutex_lock(mutex_id mutex);
int mutex_trylock(mutex_id mutex);
int mutex_unlock(mutex_id mutex);
int mutex_is_locked(mutex_id mutex);

cond_id cond_create(mutex_id mutex);
int cond_destroy(cond_id cond);
int cond_signal(cond_id cond);
int cond_broadcast(cond_id cond);
int cond_wait(cond_id cond);
int cond_timed_wait(cond_id cond, time t);

char const * system_get_last_error();
void system_panic_stop();
void __based(system_seg) * system_alloc(size_t n);
void system_free(void __based(system_seg) * block);


