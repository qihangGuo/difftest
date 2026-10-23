# Upstream synchronization progress

| Phase | Status |
| --- | --- |
| Source archives/Git bundles verified | PASS |
| Independent integration snapshots | PASS |
| Fetch and merge | Merged; validation in progress |
| Source/native validation | PASS |
| Import to original workspaces | Pending |

Backup: /tmp/xsai-evu-upstream-sync-20261023-232704
Original files, refs, indexes and binary diffs are preserved.

Pinned official upstream: XSAI c1c8c65101903d931604be937e98f5ba55cc2ef3; NEMU 16f993840b97afb491a12fc43ce50eb58f49e101.
Child merges preserve local EVU/CDC snapshots and parent-pinned CUTE/Difftest/Cache/Utility changes.
Root conflicts resolved by preserving EVU dispatch/bridge and upstream matrix prefetch. Difftest keeps both EVU and store-hash reference APIs, plus upstream clock-edge ordering and checked period parsing.

Static integration review completed. Fixes: deterministic defaults for new ArchEvent/StoreEvent ordering fields; EVU AB writes now require fire so stalled writes cannot override loaders; standalone CUTE wrapper forwards result metadata; reject unsupported matrix-prefetch plus accelerator-CDC combination until notification CDC exists; matrix HostTLB prototypes preserve char matrix identity; EVU defconfig includes new official options.
First make comp: PASS (main and test Scala source compilation only). Recompile required after compatibility fixes.

## Validation after compatibility fixes
- `make comp`: PASS, main and test Scala sources. No elaboration or Verilog generation.
- NEMU `make CCACHE= git_commit= riscv64-xs-evu-ref_defconfig` then `make -j8 CCACHE= git_commit=`: PASS.
- Native EVU compute: PASS, 896 FP cases/all sticky-flag histories, m8/cross-page, CSR isolation/error/ACK, and 7x512 softmax cases.
- Softmax max relative error: exp 1.64186475e-6; sum 8.7303498e-8; output 1.64186611e-6; max |sum(p)-1| 6.59646116e-8. Reduction sum 0x4200d92a, unchanged.
- Existing RTL trace replay: PASS for scalar flags 0/1/31, each 690 instructions, 112 stores/896 physical beats. This reuses an earlier trace; it is not a new RTL simulation.
- Matrix native: PASS, 3 launches, 6144 result beats and 6144 RMW stores. Expected negative checks deliberately emit mismatch diagnostics before final PASS.
- MMU native: PASS, 9 physical stores, cross-page VRF transfers/remap/precise faults/retry.
- Official `scripts/fpga/tests/test_arch_event_abi.py`: PASS.
- Guest `riscv64-linux-gnu-gcc -fsyntax-only`: PASS for 6 source files, no guest binaries. Initial harness lacked software/evu/include; adding the include path fixed the harness, with no source change.
- Difftest C++ `-fsyntax-only`: PASS for EVU checker, main checker, reference proxy using an explicitly augmented temporary ABI header. This is not generated ABI validation.
- Original source SHA256 manifest: unchanged in all six repositories before import.
Full regenerated RTL/EMU Difftest, asynchronous CDC simulations and performance are server validation, per user restriction.
