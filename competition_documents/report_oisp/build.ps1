# Build the OISP report (pdflatex x2).
# Usage:  .\build.ps1            (build main.tex)
#         .\build.ps1 -Clean     (remove build by-products)

param(
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src  = 'main'          # entry point .tex
$doc  = 'report_oisp'   # compiled output name (via -jobname)

Push-Location $root
try {
    if ($Clean) {
        $patterns = '*.aux', '*.log', '*.out', '*.synctex.gz', '*.toc', '*.lof', '*.lot', '*.fls', '*.fdb_latexmk', '*.xdv'
        foreach ($p in $patterns) {
            Get-ChildItem -Path (Join-Path $root $p) -File -ErrorAction SilentlyContinue |
                Remove-Item -Force
        }
        Write-Host 'Build files removed.'
        exit 0
    }

    # Luu tru ban PDF hien tai vao versions\ truoc khi build moi ban
    # (moi ban build deu quan trong - giu lai toan bo lich su phien ban)
    $versions = Join-Path $root 'versions'
    $current  = Join-Path $root "$doc.pdf"
    if (Test-Path $current) {
        New-Item -ItemType Directory -Path $versions -Force | Out-Null
        $next = 1
        Get-ChildItem -Path $versions -File -Filter '*.pdf' -ErrorAction SilentlyContinue |
            ForEach-Object {
                if ($_.Name -match '^(\d{3})_') { $next = [Math]::Max($next, [int]$Matches[1] + 1) }
            }
        if ($next -gt 999) {
            Write-Host 'Versions folder da day (999). Khong luu tru them.' -ForegroundColor Yellow
        } else {
            $archived = Join-Path $versions ('{0:d3}_{1}.pdf' -f $next, $doc)
            Copy-Item $current $archived
            Write-Host ("Archived previous PDF -> versions\{0:d3}_{1}.pdf" -f $next, $doc) -ForegroundColor Cyan
        }
    }

    # Two passes so the .aux (hyperref, page refs) settles.
    foreach ($pass in 1..2) {
        pdflatex -synctex=1 -interaction=nonstopmode "-jobname=$doc" -file-line-error "$src.tex" | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Build failed on pass $pass. See $doc.log." -ForegroundColor Red
            exit $LASTEXITCODE
        }
        Write-Host "Pass $pass/2 OK"
    }
    Write-Host "Built $doc.pdf" -ForegroundColor Green
}
finally {
    Pop-Location
}
