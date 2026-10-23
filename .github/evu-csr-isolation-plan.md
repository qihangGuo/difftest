# EVU CSR isolation implementation

## Scope and requirements

Modify XSAI RTL, NEMU EVU/CSR reference, Difftest interfaces/checker and EVU software together.
Target DefaultExternalVectorMatrixConfig, VLEN4096/DLEN256, fixed RNE/start0, drain before
context switching. Preserve unrelated dirty changes. No local EMU, Verilog or guest binaries.
Numeric kernels/reduction topology remain. Read-only integration audits were delegated as
required by docs/workflow.md; user already authorized implementation and local checks.

## Design rationale

Core-side authoritative state bank; immutable command config/descriptor snapshots.
Independent flags/context ownership and ordered state publication. Arithmetic asynchronous.
Shared address-space ownership changes drain; errors retain origin and do not imply rollback.
ABI and current limits: repository-root .github/evu-csr-architecture.md.

## Prerequisites

Commands below are from the named repository. Existing NEMU uses riscv64-xs-evu-ref_defconfig.
AM tree is sibling nexus-am. Local Scala compilation and native reference executables allowed;
server-only simulation commands are documented in the architecture document. No unrelated
submodule reset, checkout replacement or generated artifact check-in.

## Phase 1: RTL state and ordering

Files: NewCSR/EvuCSR, EvuCSRDefs, CSR wrapper, Backend/ROB/AMU, XSCore/Tile, EVU command,
backend/execution adapters and harness tie-offs. Add18 CSR addresses, snapshots and drain;
remove EVU/core flag/vstart coupling; error origin and group checks before retirement.

Check from XSAI_external_vector:

```sh
make comp > /tmp/evu-scala-compile.log 2>&1
```

PASS: main/test source compile success and consistent addresses. No elaboration or simulation.

## Phase 2: NEMU semantics and native regression

Files: evu state/model/compute/matrix, CSR permissions/export, checkpoint guards, host tests.
Use direct SoftFloat independent flags and preserve core state; reference faults need an oracle.

Check from NEMU:

```sh
make -j8 CCACHE= > /tmp/evu-nemu-build.log 2>&1
make -C tools/evu-ref-test > /tmp/evu-host-build.log 2>&1
./tools/evu-ref-test/evu_compute_ref_test ./build/riscv64-nemu-interpreter-so > /tmp/evu-host-test.log 2>&1
./tools/evu-ref-test/evu_compute_ref_test ./build/riscv64-nemu-interpreter-so --replay /tmp/evu-softmax-rtl.trace > /tmp/evu-replay-test.log 2>&1
```

Replay uses an already-recorded trace, only when that trace exists; it is not new RTL validation.
PASS: exit0, all positive tests pass, numeric bounds checked, no unhandled mismatches.

From XSAI_external_vector:

```sh
bash software/evu/scripts/check_matrix_ref.sh ../NEMU > /tmp/evu-matrix-test.log 2>&1
cc -std=gnu11 -O2 -D__ISA__=riscv64 -I ../NEMU/include -I ../NEMU/src/isa/riscv64/include -I ../NEMU/src/isa/riscv64/instr software/evu/host/evu_mmu_ref_test.c -o /tmp/evu-mmu-ref-test -ldl
/tmp/evu-mmu-ref-test ../NEMU/build/riscv64-nemu-interpreter-so > /tmp/evu-mmu-test.log 2>&1
```

PASS: exit0 and final PASS. Negative matrix tests intentionally log mismatches/incomplete drain.

## Phase 3: Difftest and software

Files: Difftest bundles/dependencies/events/refproxy/checker; software helpers/workload startup
and new context test. Expand snapshot21, complete_v2 vxsat, drain API; align state timing.

From XSAI_external_vector:

```sh
make comp > /tmp/evu-scala-compile.log 2>&1
riscv64-linux-gnu-gcc -std=gnu11 -march=rv64gcv -mabi=lp64d -fno-tree-vectorize -fsyntax-only -DARCH_H='"arch/riscv64-xs.h"' -I ../nexus-am/am -I ../nexus-am/am/include -I ../nexus-am/libs/klib/include -I software/evu/include software/evu/csr_context_evu_test.c software/evu/online_softmax_evu_test.c software/evu/exp2_evu_test.c software/evu/vector_add_evu_test.c software/evu/quanti8mma_evu_example.c software/evu/virtual_memory_evu_test.c > /tmp/evu-guest-syntax.log 2>&1
/usr/bin/python3 /tmp/evu_cxx_check.py
```

C++ check uses a temporary updated ABI header and syntax-only; actual generated layout/linking
remains server verification. PASS: exit0, consistent21 words and v2 event symbols.

## Phase 4: final review and acceptance

Local: ABI cross-check; `git diff --check` in XSAI_external_vector, difftest, NEMU and nexus-am;
update architecture/progress with actual checks and limits. PASS: clean whitespace check,
consistent CSR maps, no unsupported validation claims.

Server: regenerate matching RTL/NEMU/software, run softmax and csr_context plus existing
vector_add/exp2/quanti8mma/virtual_memory. Require GOOD TRAP, no mismatch, checked numeric
accuracy. Add matrix fault/backpressure/group/permissions/drain cases. CDC control checked
separately because existing full Difftest requires same clock. Full Linux/long-run and synthesis/
performance deferred to server per user instruction; never claim them passed locally.

## Final verification

Deliver concrete changes, software ABI/helper/test paths, completed local checks and remaining
server acceptance. No commit, deploy, EMU run, Verilog or guest binary requested locally.
