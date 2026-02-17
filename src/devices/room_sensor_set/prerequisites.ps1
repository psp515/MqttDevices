# ---------------- CONFIG ----------------
# List of library folders to copy (names must match folders in src\lib\)
$Libraries = @(
    "logger",
    "transport",
    "configuration"
)

# Resolve script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$SrcRoot = Resolve-Path (Join-Path $ScriptDir "..\..") 

# Source libraries live here
$LibSource = Join-Path $SrcRoot "lib"

# Destination is the current working directory's lib\
$DestLib = Get-Location
# ----------------------------------------

function Copy-Library {
    param([string]$LibName)

    $Src = Join-Path $LibSource $LibName
    $Dest = $DestLib  # copy directly into current directory

    if (-not (Test-Path $Src -PathType Container)) {
        Write-Host "Library not found: $Src"
        return
    }

    Write-Host "Copying contents of $LibName into $Dest..."

    # Copy all files and subfolders from library to destination
    Get-ChildItem -Path $Src -Recurse | ForEach-Object {
        $TargetPath = $_.FullName.Replace($Src, $Dest)
        if ($_.PSIsContainer) {
            if (-not (Test-Path $TargetPath)) {
                New-Item -ItemType Directory -Path $TargetPath | Out-Null
            }
        } else {
            Copy-Item -Path $_.FullName -Destination $TargetPath -Force
        }
    }

    Write-Host "Installed contents of $LibName into $Dest"
}


# Copy all libraries from the list
foreach ($Lib in $Libraries) {
    Copy-Library $Lib
}
