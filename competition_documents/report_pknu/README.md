# PKNU x HCMUT — Symposium Paper (LaTeX)

Mini paper for the **PKNU–HCMUT Joint International Academic Symposium on Engineering**:

> **Design and Implementation of a Real-Time Flight Controller Architecture on ESP32-S3 for Quadcopter UAVs**

Grown out of the original one-page submission document
(`../261030_PKNU-HCMUT-Abstract_Real-Time Flight Controller.docx`); the layout layer
still follows that Word template.

## Files

| File | Purpose |
| --- | --- |
| `minipaper.tex` | Main file: metadata, title block, abstract, keywords + `\input` list |
| `01..06_*.tex` | One file per section, numbered in reading order (01 introduction → 06 references) — each carries the content plan from `../poster_draft_v2.md` as comments |
| `style.sty` | All layout/style (margins, fonts, colours, header/footer, blocks) |
| `minipaper.pdf` | Compiled PDF |
| `.zed/tasks.json` | Zed build tasks (2-pass build, single pass, clean, open PDF) |
| `build.ps1` | CLI build helper (pdflatex ×2, or `-Clean`) |
| `.gitignore` | Ignores LaTeX build by-products |

> `latexmk` is **not** used: this MiKTeX install has no Perl, so latexmk cannot run.
> Two `pdflatex` passes do the job.

## Build

- **Zed**: task palette → `pdflatex: Build Paper (2 Passes)`.
- **CLI**:

```powershell
.\build.ps1
# or
pdflatex -interaction=nonstopmode minipaper.tex
pdflatex -interaction=nonstopmode minipaper.tex
```

## Editing

- Title block metadata in the preamble of `minipaper.tex`:
  `\symposium{}`, `\title{}`, `\author{}`, `\affiliation{}`, `\corresponding{}`.
  `\maketitle` renders the template banner.
- Opening block: `\paperheading{Abstract}` + `\begin{leadblock} ... \end{leadblock}`
  (italic, justified, 9.5 pt), then `\keywordline{...}`.
- Section headings use the standard `\section{}` / `\subsection{}`.
- Units: `\us` prints `µs`; write `350~\us{}`.
- Requires `newtx`, `booktabs`, `tabularx`, `graphicx`, `caption`, `fancyhdr`,
  `microtype` (MiKTeX / TeX Live both fine).

## Layout notes (mapped from the .docx)

- A4, margins top/bottom 1.4 cm, left/right 1.8 cm (Word `sectPr`: 794/1020 twips).
- Running header (right, 8 pt, bold, `#586370`): `PKNU x HCMUT | JOINT SYMPOSIUM 2027`.
- Footer (7.5 pt, `#78818A`): `PKNU-HCMUT` left, page number centre, `JOINT SYMPOSIUM 2027` right.
- Typeface: Times look-alike (`newtxtext` + `newtxmath`).
- Word half-point sizes → pt: 30→15, 28→14, 22→11, 19→9.5, 18→9, 17→8.5, 15→7.5.
