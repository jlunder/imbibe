#include "imbibe.h"

// #define RENDER_TEST

#include "inplace.h"

#if defined(RENDER_TEST)
#include "render_test_task.h"
#else
#include "main_task.h"
#endif

#include "keyboard.h"


#define logf_imbibe(...) disable_logf("IMBIBE: " __VA_ARGS__)


int main(int argc, char * argv[]) {
  (void)argc;
  (void)argv;
#if defined(RENDER_TEST)
  render_task_instance.setup();
  render_task_instance->start();
  render_task_instance->run_loop();
  render_task_instance.teardown();
#else
  main_task_instance.setup();
  main_task_instance->start();
  main_task_instance->run_loop();
  main_task_instance.teardown();
#endif
  return 0;
}


#if defined(SIMULATE)


uint16_t dummy_screen[16384];

struct key_seq_entry {
  uint32_t steps;
  uint32_t ms;
  uint16_t key;
};

key_seq_entry const sim_seq[] = {
  {100, 2000, 0x011B},
  { 10,  200,      0},
};
uint32_t sim_seq_i = 0;
uint32_t sim_seq_i_step = 0;
uint32_t sim_seq_i_ms = 0;

uint32_t pit_tick_counter = 0;
void pit_tick_counter_int_handler() { ++pit_tick_counter; }
void (*pit_int_handler)() = pit_tick_counter_int_handler;


void step_simulator() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  ++sim_seq_i_step;
  if((sim_seq_i_step >= sim_seq[sim_seq_i].key)
      && (sim_seq[sim_seq_i].key == 0)) {
    assert(sim_seq_i + 1 < LENGTHOF(sim_seq));
    ++sim_seq_i;
    sim_seq_i_step = 0;
    sim_seq_i_ms = 0;
  }
  uint32_t target_ms = (uint64_t)sim_seq_i_step * sim_seq[sim_seq_i].ms
    / sim_seq[sim_seq_i].steps;
  while(sim_seq_i_ms < target_ms) {
    if(pit_int_handler != pit_tick_counter_int_handler) {
      pit_int_handler();
    }
    ++sim_seq_i_ms;
  }
  logf_imbibe("sim_step [%u:%u, %u ms]\n",
    sim_seq_i, sim_seq_i_step, sim_seq_i_ms);
}


bool keyboard::key_event_available() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  bool avail = (sim_seq[sim_seq_i].key != 0)
    && (sim_seq_i_step >= sim_seq[sim_seq_i].steps);
  logf_imbibe("key_avail: %d\n", (int)avail);
  return avail;
}


key_code_t keyboard::read_key_event() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  assert(key_event_available());
  if (sim_seq_i >= LENGTHOF(sim_seq)) {
    return 0;
  }
  if (sim_seq_i_step >= sim_seq[sim_seq_i].steps) {
    assert(sim_seq_i_ms >= sim_seq[sim_seq_i].ms);
    uint16_t key = sim_seq[sim_seq_i].key;
    assert(sim_seq_i + 1 < LENGTHOF(sim_seq));
    ++sim_seq_i;
    sim_seq_i_step = 0;
    sim_seq_i_ms = 0;
    logf_imbibe("read_key: 0x%04X\n", key);
    return key;
  } else {
    assert(!"read_key no key available!");
    return 0;
  }
}


void _dos_setvect(int int_no, void (*int_handler)()) {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("setvect 0x%02X, %p (overwrites %p)\n", int_no,
    int_handler, pit_int_handler);
  pit_int_handler = int_handler;
}


void (*_dos_getvect(int int_no))() {
  (void)int_no;
  assert(int_no == 8);
  logf_imbibe("getvect 0x%02X: %p\n", int_no, pit_int_handler);
  return pit_int_handler;
}


void _chain_intr(void (*int_handler)()) {
  assert(int_handler);
  logf_imbibe("chain_intr %p\n", int_handler);
  int_handler();
}


#endif


