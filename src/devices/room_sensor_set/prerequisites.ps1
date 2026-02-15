# ---------------- CONFIG ----------------
# List of library folders to copy (names must match folders in src\lib\)
$Libraries = @(
    "logger",
    "transport"
)

# Resolve script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$SrcRoot = $ScriptDir

# Source libraries live here
$LibSource = Join-Path $SrcRoot "lib"

# Destination is the current working directory's lib\
$DestLib = Get-Location
# ----------------------------------------

function Copy-Library {
    param(
        [string]$LibName
    )

    $Src = Join-Path $LibSource $LibName
    $Dest = Join-Path $DestLib $LibName

    if (-not (Test-Path $Src -PathType Container)) {
        Write-Host "Library not found: $Src"
        return
    }

    if (-not (Test-Path $DestLib)) {
        New-Item -ItemType Directory -Path $DestLib | Out-Null
    }

    Write-Host "Copying $LibName..."
    
    if (Test-Path $Dest) {
        Remove-Item -Recurse -Force $Dest
    }

    Copy-Item -Recurse -Force $Src $Dest

    Write-Host "Installed: $Dest"
}

# Copy all libraries from the list
foreach ($Lib in $Libraries) {
    Copy-Library $Lib
}
