# Build the PKNU x HCMUT symposium paper (pdflatex x2).
# Usage:  .\build.ps1            (build minipaper.tex)
#         .\build.ps1 -Clean     (remove build by-products)

param(
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$doc  = 'minipaper'

Push-Location $root
try {
    if ($Clean) {
        $patterns = '*.aux', '*.log', '*.out', '*.synctex.gz', '*.toc', '*.fls', '*.fdb_latexmk', '*.xdv'
        foreach ($p in $patterns) {
            Get-ChildItem -Path (Join-Path $root $p) -File -ErrorAction SilentlyContinue |
                Remove-Item -Force
        }
        Write-Host 'Build files removed.'
        exit 0
    }

    # Two passes so the .aux (hyperref, page refs) settles.
    foreach ($pass in 1..2) {
        pdflatex -synctex=1 -interaction=nonstopmode -file-line-error "$doc.tex" | Out-Null
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
