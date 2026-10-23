#include "checkers.h"
#include "goldenmem.h"

#if defined(CONFIG_DIFFTEST_EVUSTOREEVENT) && defined(CONFIG_DIFFTEST_EVUCOMPLETIONEVENT)
EvuChecker::EvuChecker(DiffState *state, RefProxy *proxy, std::function<DiffTestState &()> get_dut)
    : DiffTestChecker(state, proxy), get_dut(std::move(get_dut)) {
  Assert(proxy->evu_available(),
         "EVU difftest requires an updated CONFIG_CUSTOM_EVU=y reference exporting difftest_evu_matrix");
  proxy->evu_set_async(true);
}

EvuChecker::~EvuChecker() {
  Info("[DiffTest][EVU] core=%d checked_stores=%lu checked_completions=%lu checked_matrix_beats=%lu pending=%d\n",
       state->coreid, stores, completions, matrix_beats, proxy->evu_pending());
}

int EvuChecker::do_step() {
  auto &dut = get_dut();
#ifdef CONFIG_DIFFTEST_EVUMATRIXEVENT
  auto &matrix = dut.evu_matrix;
  if (matrix.valid) {
    if (proxy->evu_matrix(matrix.row, matrix.col, matrix.data, sizeof(matrix.data),
                          matrix.mask, matrix.fflags, matrix.last)) {
      eprintf("[DiffTest][EVU] matrix result mismatch core=%d cycle=%lu row=%u col=%u\n",
              state->coreid, state->cycle_count, matrix.row, matrix.col);
      return STATE_DIFF;
    }
    matrix.valid = 0;
    ++matrix_beats;
  }
#endif
  auto &store = dut.evu_store;
  if (store.valid) {
    if (proxy->evu_store(store.addr, store.data, store.mask)) {
      eprintf("[DiffTest][EVU] store mismatch core=%d cycle=%lu addr=0x%lx\n",
              state->coreid, state->cycle_count, store.addr);
      return STATE_DIFF;
    }
#ifdef DEBUG_GOLDENMEM
    update_goldenmem(store.addr, (void *)store.data, store.mask, 64);
#endif
    store.valid = 0;
    ++stores;
  }
  auto &completion = dut.evu_completion;
  if (completion.valid) {
    if (proxy->evu_complete(completion.pc, completion.instr, completion.fflags,
                                completion.exception, completion.cause, completion.vxsat)) {
      eprintf("[DiffTest][EVU] completion mismatch core=%d cycle=%lu pc=0x%lx\n",
              state->coreid, state->cycle_count, completion.pc);
      return STATE_DIFF;
    }
    completion.valid = 0;
    ++completions;
    state->has_progress = true;
  }
#ifdef CONFIG_DIFFTEST_EVUDRAINEVENT
  auto &drain = dut.evu_drain;
  if (drain.valid) {
    if (proxy->evu_drain(drain.fflags, drain.exception, drain.cause, drain.pc, drain.tval, drain.tvalValid)) {
      eprintf("[DiffTest][EVU] drain mismatch core=%d cycle=%lu\n", state->coreid, state->cycle_count);
      return STATE_DIFF;
    }
    drain.valid = 0;
    state->has_progress = true;
  }
#endif
  if (dut.trap.hasTrap && proxy->evu_pending()) {
    eprintf("[DiffTest][EVU] program terminated with outstanding commands\n");
    return STATE_DIFF;
  }
#ifdef CONFIG_DIFFTEST_EVUVRFSTATE
  const uint32_t defined = proxy->evu_reg_valid();
  for (unsigned reg = 0; reg < CONFIG_DIFF_EVU_VRF_WIDTH; ++reg) {
    auto &snapshot = dut.evu_vrf[reg];
    if (!snapshot.valid) continue;
    if (!(defined & (UINT32_C(1) << reg))) { snapshot.valid = 0; continue; }
    uint64_t expected[sizeof(snapshot.value) / sizeof(uint64_t)] = {};
    if (proxy->evu_get_reg(reg, expected, sizeof(expected))) return STATE_ERROR;
    for (size_t word = 0; word < sizeof(expected) / sizeof(uint64_t); ++word) {
      if (snapshot.value[word] != expected[word]) {
        eprintf("[DiffTest][EVU] vr%u word=%zu DUT=0x%016lx REF=0x%016lx\n",
                reg, word, snapshot.value[word], expected[word]);
        return STATE_DIFF;
      }
    }
    snapshot.valid = 0;
  }
#endif
  return STATE_OK;
}

int EvuChecker::check_csr() {
  auto &dut = get_dut();
#ifdef CONFIG_DIFFTEST_EVUCSRSTATE
  if (dut.evu_csr.valid) {
    uint64_t expected[21] = {};
    const char *names[] = {"vl", "vtype", "vstart", "vlenb", "configured", "efflags", "evxsat", "mevstate",
      "evstatus", "evcause", "evepc", "evtval", "eqdesc0", "eqdesc1", "eqdesc2", "eqdesc3",
      "eqdesc4", "eqdesc5", "eqdesc6", "eqdesc7", "evcfg"};
    if (proxy->evu_get_csr(expected, sizeof(expected))) return STATE_ERROR;
    for (size_t field = 0; field < 21; ++field) {
      if (dut.evu_csr.value[field] != expected[field]) {
        eprintf("[DiffTest][EVU] %s DUT=0x%lx REF=0x%lx\n", names[field],
                dut.evu_csr.value[field], expected[field]);
        return STATE_DIFF;
      }
    }
    dut.evu_csr.valid = 0;
  }
#endif
  return STATE_OK;
}
#endif
