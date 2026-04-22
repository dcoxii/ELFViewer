# AGENTS.md

## Project

`/opt/build/projects/android/ELFViewer`

Cross-platform ELF viewer/editor. The current project is described as an “ELF file viewer/editor for Windows, Linux and MacOS.” This agent guide is focused on updating it into a more modern, and more capable ELF analysis and editing application. The existing README indicates the current product already supports viewing/editing ELF files and is distributed cross-platform. fileciteturn0file0

---

## Mission

Modernize `ELFViewer` into a dependable ELF inspection and structured-editing tool with:

- stronger and more defensive ELF parsing
- clearer internal separation between parsing, analysis, and UI
- improved support for modern ELF realities on Android and Linux
- easier automated testing and future extension

The goal is **not** to become a blind hex editor with ELF labels. The goal is to become a **schema-aware ELF workbench** that understands ELF structure deeply enough to make correct edits, detect corruption, and explain changes.

---

## Primary Objectives

### 1. Parser modernization

Improve parser correctness and resilience for:

- ELF32 and ELF64
- little-endian and big-endian inputs
- malformed, truncated, fuzzed, and adversarial files
- extended section numbering and edge-case header layouts
- GNU and Android-specific notes / metadata where present
- symbol/versioning structures when available
- relocations, dynamic sections, notes, program headers, and string tables

Parsing must be:

- bounds-checked
- offset-aware
- overflow-safe
- explicit about trust boundaries
- decoupled from UI concerns

### 2. Editing modernization

Support structured edits with validation, not raw blind mutation.

Preferred editing capabilities:

- change ELF header fields where legal
- rename sections or symbols when layout permits
- patch entry point / interpreter / dynamic strings with strict validation
- edit notes and selected metadata blocks
- add, remove, or resize data only through operations that explicitly recalculate impacted offsets/sizes
- preview change sets before writing
- write backup copies and generate diff summaries

Every write path should have:

- precondition checks
- post-write validation
- failure rollback or safe-write temp-file replacement

### 3. UX modernization

The UI should help a user understand an ELF quickly.

Priority UX improvements:

- tree view for ELF structures
- detail panes with typed interpretation
- raw-bytes view synchronized to structured selections
- warnings panel for malformed structures
- edit forms for all fields
- comparison view between original and modified file
- validation report after parse and before save

### 4. Quality and maintainability

The project should become easier to extend than the legacy implementation.

Priority engineering outcomes:

- parser library separated from GUI
- unit tests for each ELF structure reader/writer
- regression corpus for real-world binaries
- fuzz harnesses for all parse entry points
- deterministic error reporting
- minimal hidden global state

---

## Non-Goals

Avoid turning this project into:

- a generic disassembler competitor
- a full decompiler
- a dynamic loader emulator
- an unrestricted binary rewriter without structural guarantees

It is acceptable to integrate with external tools later, but the core mission here is **reliable ELF parsing + explainable editing**.

---

## Recommended Architecture

### Core layers

#### 1. Byte source layer

Responsibilities:

- open/read file and memory buffers
- provide checked slicing helpers
- handle endian-aware primitive reads
- centralize offset/size overflow checks

Suggested design:

- `ByteView` / `BinaryReader`
- immutable read API
- separate mutable patch/writer API

#### 2. Parse/model layer

Responsibilities:

- parse on-disk ELF data into typed internal models
- preserve exact offsets and raw ranges for every structure
- distinguish between:
  - parsed successfully
  - partially parsed with warnings
  - invalid/unreadable

Suggested model families:

- `ElfFile`
- `ElfHeader`
- `ProgramHeader`
- `SectionHeader`
- `DynamicEntry`
- `Symbol`
- `Relocation`
- `Note`
- `VersionInfo`
- `ValidationIssue`

#### 3. Analysis layer

Responsibilities:

- derive relationships across structures
- resolve names through string tables
- map virtual addresses to file offsets
- detect inconsistencies
- generate summaries for UI and save validation

#### 4. Edit/transform layer

Responsibilities:

- represent edits as explicit operations
- compute impact before applying
- emit patch plans and updated layout metadata

Examples:

- `RenameSection`
- `UpdateEntryPoint`
- `PatchInterpreter`
- `EditDynamicString`
- `ReplaceNotePayload`

#### 5. Serialization/write layer

Responsibilities:

- apply validated operations
- update dependent offsets/sizes if required
- write atomically
- reparse written output for verification

#### 6. UI layer

Responsibilities:

- render parsed model and issues
- never perform raw offset math directly
- use only public parser/analyzer/editor APIs

---

## Parsing Rules

1. **Never trust file-provided offsets or counts without range validation.**
2. **Check integer overflow before every `offset + size`, `count * entry_size`, or table-span calculation.**
3. **Treat all string table references as untrusted until validated against table bounds.**
4. **Support partial parsing when feasible; record warnings instead of crashing.**
5. **Preserve raw offsets/ranges for each parsed object so the UI can correlate bytes to structures.**
6. **Separate parsing from interpretation.** Read bytes first, then resolve cross-references.
7. **Expose structured diagnostics.** No silent failure.
8. **Use typed enums and named constants** for ELF class/data/machine/type values where known.
9. **Gracefully handle unknown machine/ABI/note types.** Unknown does not mean invalid.
10. **Re-parse after writes** to verify structural integrity.

---

## Editing Rules

1. **Every edit must be modeled as an explicit transaction or operation.**
2. **No direct UI-to-bytes write path.** UI submits edit requests; core validates and applies.
3. **Reject edits that would invalidate dependent structures unless relocation/rebuild logic exists.**
5. **Always produce a preview:**
   - fields changed
   - offsets impacted
   - sections/tables affected
   - validation warnings introduced or resolved
6. **Always write to a temp file first**, then replace atomically where possible.
7. **Always preserve original file timestamps/permissions only if user opts in.**
8. **Always keep an undo-able in-memory edit plan when possible.**

---

## Modernization Priorities

### Highest priority

- isolate parser core from GUI code
- audit all offset/count math
- add structured diagnostics and validation engine
- build a test corpus with normal + malformed ELF files
- implement save flow with reparse verification

### Medium priority

- richer symbol/dynamic/relocation/version display
- edit previews and structured patch plans
- synchronized raw/structured views
- plugin-style analyzers for Android/Linux specifics

### Later priority

- diff mode between two ELF files
- batch validation mode / CLI
- export reports (JSON / Markdown)
- optional integration with external tools like `readelf`, `objdump`, or disassemblers for comparison only

---

## Suggested Tech Direction

Choose the exact stack based on current codebase constraints, but prefer these principles:

- modern C++ with clear ownership and RAII
- centralize binary reading utilities
- use strong typing for addresses, offsets, and sizes where practical
- keep Qt UI thin if the current app already depends on Qt
- consider introducing a standalone core library such as:
  - `libelfviewer_core`
  - `libelfviewer_parse`
  - `libelfviewer_edit`

If large refactors are risky, use a staged migration:

1. wrap old parser behind interfaces
2. build new parser modules in parallel
3. route selected structures to new modules first
4. delete legacy code only after regression coverage exists

---

## Testing Expectations

### Unit tests

Must exist for:

- ELF identification parsing
- header parsing
- program/section header parsing
- string table resolution
- symbol parsing
- dynamic section parsing
- relocation parsing
- note parsing
- validation rules

### Regression tests

Maintain a corpus covering:

- valid ELF32/ELF64 samples
- stripped binaries
- PIE and shared objects
- Android-linked binaries where available
- corrupted/truncated samples
- files with overlapping or inconsistent metadata
- fuzz-discovered crashers

### Fuzzing

Fuzz targets should include:

- top-level parse entry
- section parser
- note parser
- dynamic parser
- symbol/string resolution logic
- edit-plan validation entry points

Any crash, hang, OOB read, or UB discovered by fuzzing is a release blocker for parser-related changes.

---

## Diagnostics Standard

Diagnostics should be structured with:

- severity: `info | warning | error`
- code: stable machine-readable identifier
- message: short human-readable summary
- offset/range: where applicable
- related structure: optional
- remediation hint: optional

Examples:

- `ELF_HDR_TRUNCATED`
- `PHDR_TABLE_OUT_OF_RANGE`
- `SHDR_NAME_OOB`
- `DYNAMIC_STRTAB_INVALID`
- `WRITE_REPARSE_MISMATCH`

Do not bury parser failures in generic exception text.

---

## Performance Guidance

- prioritize correctness over micro-optimization
- avoid repeated full-file rescans when indexes can be cached
- permit lazy decoding for expensive secondary structures
- never trade away bounds checking for speed
- large-file support should remain responsive in UI via incremental rendering or background analysis threads where appropriate

---

## Security Posture

This application opens attacker-controlled binary files. Treat it like hostile input handling software.

Required posture:

- no unchecked memory access
- no trust in embedded counts/offsets
- no UI lockups on malformed files when avoidable
- no save operation without validation
- no implicit execution of embedded content
- sanitize any exported text derived from binary strings when shown in rich UI contexts

For any parser crash bug:

- add a reproducer sample if legally distributable
- add a regression test
- document root cause in the fix commit

---

## Code Change Policy for Agents

When making changes in this repository:

1. Prefer small, reviewable commits.
2. Keep parser refactors separate from UI refactors.
3. Add tests in the same change whenever parser or writer behavior changes.
4. Do not introduce silent fallback behavior for invalid ELF structures.
5. Preserve backward compatibility in user-visible behavior.
6. When replacing legacy parsing logic, document which structures are now handled by the new path.
7. For editing features, document the exact all subset supported.

---

## Suggested Near-Term Roadmap

### Phase 1 — Stabilize core parsing

- inventory current parser modules
- build shared checked binary reader utilities
- add diagnostics pipeline
- add tests for ELF header, program headers, section headers

### Phase 2 — Separate core from UI

- move parsing and validation into standalone library
- expose stable model APIs for UI
- remove UI-owned binary parsing logic

### Phase 3 — Rich analysis and modern UX

- structured panels for symbols, relocations, notes, versions
- raw/structured cross-highlighting
- diff/compare workflow
- export validation report

---

## Definition of Done for Major Changes

A parser/editor modernization task is not complete unless:

- code builds cleanly
- tests pass
- malformed input handling is covered
- diagnostics are clear
- new write paths reparse successfully
- user-visible behavior is documented when changed

---

## Agent Deliverables

When an agent works on this repo, useful outputs include:

- parser audit notes
- proposed module split
- test plan and sample corpus list
- concrete refactor patches
- validation rule additions
- edit-operation design docs
- UI workflows for all editing

Preferred format for substantial design work:

- problem
- constraints
- current risk
- proposed change
- compatibility impact
- tests added
- follow-up work

---

## Repository Context

The uploaded README describes the project as a cross-platform ELF viewer/editor with documentation for running, building, and releases. This guide assumes that existing foundation remains useful, but that the parser/editor internals should be upgraded substantially for modern reliability, and maintainability. fileciteturn0file0
