#include "imbibe.h"

#include "inplace.h"
// #include "main_task.h"
#include "main_tas.h"

#include "key_mana.h"


inplace<main_task> main_instance;


int main(int argc, char * argv[]) {
  (void)argc;
  (void)argv;
  main_instance.setup();
  main_instance->start();
  main_instance->run_loop();
  main_instance.teardown();
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
  {100, 2000, 0x1B},
  { 10,  200,    0},
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
  fprintf(stderr, "sim_step [%u:%u, %u ms]\n",
    sim_seq_i, sim_seq_i_step, sim_seq_i_ms);
}

bool aux_key_manager__key_avail() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  bool avail = (sim_seq[sim_seq_i].key != 0)
    && (sim_seq_i_step >= sim_seq[sim_seq_i].steps);
  fprintf(stderr, "key_avail: %d\n", (int)avail);
  return avail;
}

uint16_t aux_key_manager__read_key() {
  assert(sim_seq_i < LENGTHOF(sim_seq));
  assert(aux_key_manager__key_avail());
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
    fprintf(stderr, "read_key: 0x%04X\n", key);
    return key;
  } else {
    assert(!"read_key no key available!");
    return 0;
  }
}

void aux_text_window__set_text_asm() {
  fprintf(stderr, "set text mode 03h\n");
}

void aux_text_window__restore_text_asm() {
  fprintf(stderr, "restore mode\n");
}

void aux_timer__enter_crit() {
}

void aux_timer__leave_crit() {
}

void aux_timer__ackint() {
  //fprintf(stderr, "ackint\n");
}

void aux_timer__outb(uint16_t port, uint8_t value) {
  fprintf(stderr, "outb 0x%02X, 0x%02X\n", port, value);
}

void aux_timer__outwb(uint16_t port, uint16_t value) {
  fprintf(stderr, "outwb 0x%02X, 0x%02X\n", port, value);
}

uint16_t aux_timer__inb(uint16_t port) {
  fprintf(stderr, "inb 0x%02X: 0\n", port);
  return 0;
}

uint16_t aux_timer__inwb(uint16_t port) {
  fprintf(stderr, "inwb 0x%02X: 0\n", port);
  return 0;
}

void _dos_setvect(int int_no, void (*int_handler)()) {
  assert(int_no == 8);
  fprintf(stderr, "setvect 0x%02X, %p (overwrites %p)\n", int_no,
    int_handler, pit_int_handler);
  pit_int_handler = int_handler;
}

void (*_dos_getvect(int int_no))() {
  assert(int_no == 8);
  fprintf(stderr, "getvect 0x%02X: %p\n", int_no, pit_int_handler);
  return pit_int_handler;
}

void _chain_intr(void (*int_handler)()) {
  assert(int_handler);
  fprintf(stderr, "chain_intr %p\n", int_handler);
  int_handler();
}

#endif


