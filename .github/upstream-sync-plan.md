# Preserve EVU changes while merging latest upstream

Scope: XSAI_external_vector, its required submodules, NEMU only. Never modify nexus-am.
Target: DefaultExternalVectorMatrixConfig, common-clock Difftest. No local EMU, Verilog, guest bin.

1. Verified source archives, original indexes/binary diffs and independent Git bundles preserve
   pre-sync work. Independent clones under /tmp isolate integration from original workspaces.
2. Fetch official upstream/master for XSAI and NEMU, pin SHAs. Fetch child repositories only
   as required by the new XSAI gitlinks. Merge local source snapshots and local CDC commits.
   PASS: upstream SHAs are ancestors; snapshot SHAs remain ancestors; no lost source files.
3. Resolve parent/source/gitlink conflicts with both EVU ABI1 and upstream fixes preserved.
   Review Difftest/matrix interfaces, permissions and CSR state timing. Keep fixed RNE and
   Saturn reduction/Exp2 numeric methods.
4. Validate make comp; NEMU make -j8 CCACHE= git_commit=; native EVU compute, existing
   RTL trace replay, matrix/MMU regressions; guest -fsyntax-only and git diff --check.
   PASS: source compile and all positive regressions pass, numeric bounds unchanged.
   Native negative matrix tests intentionally log mismatches; final exit0/PASS required.
5. Import verified integration commits into original checkouts on new integration branches.
   Preserve original feature refs and immutable backups. Verify original source files survive
   and no unresolved conflicts. Retest in original working directories where applicable.

Full RTL Difftest, CDC, guest execution, Linux/long-run and performance remain server checks
per user restriction. No push or remote server mutation.
