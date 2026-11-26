# Build nhanh cac hinh TikZ preview (khong build ca report).
# Usage:  .\build.ps1            (build main.tex -> fig_hw_block.pdf/.png)
#         .\build.ps1 -Clean     (xoa san pham build)
param(
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src  = 'main'

Push-Location $root
try {
    if ($Clean) {
        Get-ChildItem -Path $root -Include '*.aux','*.log','*.pdf','*.png' -File -Recurse:$false |
            Remove-Item -Force
        Write-Host 'Preview build files removed.'
        exit 0
    }

    pdflatex -interaction=nonstopmode -file-line-error "$src.tex" | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Preview build failed. See $src.log." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    pdftoppm -png -r 150 -singlefile "$src.pdf" 'fig_hw_block'
    if ($LASTEXITCODE -ne 0) {
        Write-Host 'pdftoppm failed.' -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host 'Built fig_hw_block.png (150 dpi)' -ForegroundColor Green
}
finally {
    Pop-Location
}
