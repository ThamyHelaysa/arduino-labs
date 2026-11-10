## Language

Write everything in English, even where existing files use Portuguese:

- Code: identifiers, comments, file names.
- Docs: README, glossary, ADRs, issues and specs.
- Version control: commit messages, branch names, PR titles and descriptions.

The Portuguese triage labels below are the one exception.

## Commit authorship

Credit only human authors in commits. Omit any `Co-Authored-By` trailer naming Claude or another LLM.

## Agent skills

### Issue tracker

Issues and specs live as local markdown files under `.scratch/<feature-slug>/`. See `docs/agents/issue-tracker.md`.

### Triage labels

Portuguese label vocabulary: `avaliar`, `falta-info`, `pronto-agente`, `pronto-humano`, `descartado`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: one `GLOSSARY.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.
