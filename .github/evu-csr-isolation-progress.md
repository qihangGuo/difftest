# EVU CSR isolation progress

| Phase | Status | Local checks | Server RTL Difftest |
| --- | --- | --- | --- |
| 1 RTL/ABI | Implemented | Final make comp PASS | Pending |
| 2 NEMU | Implemented | Reference build, arithmetic/CSR/error/m8 PASS | Pending |
| 3 Difftest/software | Implemented | Scala/C++ syntax, six guest sources syntax PASS | Pending |
| 4 Integration | Local checks complete | Recorded-trace replay, matrix/MMU, ABI/diff checks PASS | Pending |

No local EMU, Verilog elaboration/generation or guest binary generation was run.
Target: DefaultExternalVectorMatrixConfig, VLEN4096/DLEN256. User authorized coding.
Full updated RTL simulation, CDC tests, synthesis and performance are server acceptance.

## Phase 1: RTL and ABI

### Changes made

- Added authoritative NewCSR/EvuCSR bank and 18 custom CSR addresses. Old proposed
  locations conflicted with AME/Mbmc; final addresses are 0x8c0..2,0x8d0..7,0xbc8,0xcd0..5.
- Independent flags/sat/ES; XS reflects EVU ES. Removed EVU updates to core FCSR/vxsat/VS,
  eliminating the EVU vsDirty path that could clear core vstart.
- Immutable config/descriptor/PC command snapshots, normal CSR permissions/stateen,
  fixed RNE/start0, shared address-space changes serialized with dedicated drain.
- Drain has one ordered transaction, with CDC queues and certificate/FENCE reuse.
  Status reads drain and return Busy0. No explicit generation tag or independent-endpoint reset.
- Preserved first-error PC/address; matrix uses LAUNCH origin. Memory jobs have one owner.
  Failed matrix writes suppress further stores and consume the finite result stream.
- Fixed early-chunk vxsat loss using per-VAT accumulation and retained completion metadata.
- Fixed job-table acceptance gating and prevented overlapping matrix launches.
- Expanded group hazards and maximum LMUL address/preflight capacity. QuantToCute m2/m4/m8
  is rejected before retirement because its current CUTE stream covers one physical register.
  Widening above eight registers is illegal; reduction seed/result use single registers.

### Checks

Final `make comp` PASS (main and test Scala sources). /tmp/evu-scala-compile.log.
No Chisel elaboration, FIRRTL validation, Verilog or cycle-level RTL simulation claimed.

## Phase 2: NEMU

### Changes made

- Independent cpu.evu CSR/VRF state, permissions, standard config WARL/LMUL and CSR import/export.
- Direct SoftFloat operations accumulate per-instruction flags independent of core history;
  preserve global rounding/flags/tininess. Core FS/VS Off and vstart37 do not affect EVU.
- RNE, Saturn unordered reduction topology and Exp2 numeric method retained.
- Matrix flags consumed once at FENCE/drain. Dedicated drain rejects incomplete jobs/writes.
- Load/store fault oracle checks first-error preservation, ACK, address/PC and failed-load VRF.
- Legacy checkpoint/replay reject unsupported active EVU contexts rather than lose state.

### Checks

- Reference `make -j8 CCACHE=` PASS. /tmp/evu-nemu-build.log.
- Host compute PASS: vector-add 3 widths×3 rounds, quantization, reduction, 896 FP16/32
  flag-history cases, real CSR instructions, XS/SD/core preservation, first-error/ACK.
- m8 e8/e16/e32 LOAD/STORE PASS: 4096 bytes crossing pages, eight VRFs, full-line RMW
  guards, illegal register group/oversized widening/QuantToCute rejection.
- Softmax 7×512 PASS: relative exp1.64186475e-6, sum8.7303498e-8,
  output1.64186611e-6; max|sum(p)-1|6.59646116e-8. /tmp/evu-host-test.log.
- Existing RTL trace replay PASS for prior flags0/1/31: each690 instructions,112 stores,
  896 physical beats, pending0. /tmp/evu-replay-test.log. This is an old trace, not new RTL.
- Matrix PASS: 3 launches,6144 result beats,6144 RMW stores; flags, guards, premature
  FENCE/drain rejection, clear then repeated drain0. /tmp/evu-matrix-test.log.
- MMU PASS: cross-page/remap/fault/retry,9 physical stores. /tmp/evu-mmu-test.log.

### Issues and fixes

- Old FCSR-delta collection lost repeated NX when scalar flags already had NX. Direct
  SoftFloat collection fixes the root cause; no comparison or exception bit masking.
- The first m8 host test supplied a partial physical store mask. HBL2 emits full-line
  RMW stores; corrected the driver to UINT64_MAX while checking preserved inactive bytes.
- A new negative drain test found the checker API called the asserting architectural
  drain before testing matrix completion. It now returns failure first for incomplete
  streams/writes; both negative and successful paths pass.
- Reference build initially hit an external ccache path; disabled ccache for local builds.
- Matrix external bus-error injection is not implemented in the reference. Unexplained
  DUT faults remain mismatches; no end-to-end matrix bus-fault Difftest claim.

## Phase 3: Difftest and software

### Changes made

- Expanded EVU snapshot5→21 words; Busy excluded. Single producer in NewCSR, VRF in Saturn.
- Added vxsat in completion v2, drain event/ref hook, correct dependency/squash grouping.
- Process EVU store/completion/drain before scalar execution; compare CSR after commits;
  process completion-only cycles. Aligned bank state changes with delay3 commit/completion.
- Added evu_csr.h configuration/enable/context APIs and csr_context_evu_test.c.
- Complete16KiB VRF save/restore plus config/flags/descriptors/state; errors return-2,
  including failures during the context transfer. CUTE context and RO error restore excluded.
- Updated existing five workloads to enable ES; S-mode VM startup enables mstateen0.C.

### Checks

- Scala source compile PASS.
- g++ syntax-only checker/difftest/refproxy PASS against a temporary ABI-adjusted header
  in /tmp/evu-difftest-abi-check. /tmp/evu-cxx-syntax.log. No actual layout generation/link test.
- Six guest C sources riscv64-linux-gnu-gcc -fsyntax-only PASS. /tmp/evu-guest-syntax.log.
- No guest object/bin, EMU or Verilog generated locally.

## Phase 4: handoff

Final ABI cross-check PASS: RTL/NEMU/software match all 18 addresses; snapshot21 and v2
completion agree. Final git diff --check PASS in XSAI_external_vector, difftest, NEMU and
nexus-am. Six guest sources pass syntax-only; GCC only warns that the VM test's existing
Clang optnone attribute is ignored. User-facing ABI/config/error/performance/verification
documentation is repository-root .github/evu-csr-architecture.md.

Server must regenerate matching RTL/Difftest/NEMU/guest artifacts and run the six workloads,
including the new isolation/context test, to GOOD TRAP without mismatch. Add in-flight control,
permissions/group hazards/matrix fault/CDC reset cases; measure softmax kernel cycles and
synthesis frequency/area. Full Linux/long-run tests remain deferred to server by user instruction.
Do not report this implementation as having passed updated RTL Difftest before those runs.
