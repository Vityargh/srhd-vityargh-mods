# CLAUDE.md — SRHD mod development

Read `AGENTS.md` and [Engineering Review Standard](docs/ENGINEERING_REVIEW_STANDARD.md) before substantial design, implementation or review. These complement, never override, the owner's instructions and project safety constraints.

As an implementer: define goal/invariants and alternatives **before coding**; write targeted functional evidence, review affected systems and interactions, and document remaining limits.

As a reviewer: independently challenge the **problem model and architecture**, not merely the author's patch. Seek falsifying examples and neighboring invariants. Separate actual defect, risk and untested hypothesis; do not invent findings or perform checklist theater.

For RE/native hooks verify precise EXE build/hash, signature cardinality, machine instruction boundaries, register/flags/stack preservation, exception/loader interactions, mod compatibility, and real gameplay evidence when available.

Return a short conclusion with files/SHA, three-layer assessment, real tests, untested environments and decision. Do not self-approve where an independent reviewer is required.
