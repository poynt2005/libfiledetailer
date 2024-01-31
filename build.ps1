Write-Host "[Builder][Info] ***** filedetailer library builder *****" -ForegroundColor DarkGreen
Write-Host "[Builder][Info] Make sure you have dotnet sdk 8.x, go 1.21.x, nodejs(with npm) 20.x, Microsoft C++ Build Tools, MinGW x64 and python 3.10 installed on this machine to complete the building"  -ForegroundColor DarkGreen


Write-Host "[Builder][Info] Build libfiledetailer core lib" -ForegroundColor DarkGreen
Push-Location .\\libfiledetailer
& .\\build.ps1
Pop-Location

Write-Host "[Builder][Info] Build libshellstringconv demo" -ForegroundColor DarkGreen
Push-Location .\\libshellstringconv
& .\\build.ps1
Pop-Location

Write-Host "[Builder][Info] Build node native binding" -ForegroundColor DarkGreen
Push-Location .\\filedetailer-node
& .\\build.ps1
Pop-Location


Write-Host "[Builder][Info] Done" -ForegroundColor DarkGreen