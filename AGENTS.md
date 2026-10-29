# AGENTS.md — SRHD mod development

Before substantive development or review, follow [Engineering Review Standard](docs/ENGINEERING_REVIEW_STANDARD.md).
This is a project-level instruction for Codex and other agents. Explicit owner instructions and existing safety/ownership constraints take precedence.

- Ask three engineering questions for every code change: **functional correctness**, **system effects and invariants**, **architecture and a credible alternative**. Scale depth to actual risk.
- For reverse engineering, native hooks, inter-mod compatibility and shared runtime changes, independently challenge the proposed approach **before coding**; check results again afterward.
- Treat target version, executable hash, register/ABI/stack correctness, negative cases, hook conflicts, rollback and in-game behavior as distinct evidence categories.
- Report exact tested commands/results; never label a test as PASS if not run. Distinguish defect, plausible risk, and untested assumption.
- Do not invent problems, expand scope silently, duplicate reviews, or run expensive unrelated checks.
- Preserve game originals, release artifacts, and unrelated changes. Keep README author changes authoritative; do not overwrite documentation to match old agent assumptions.
- Do not claim an independent review of your own implementation. Use a different model/session when independence is required.
