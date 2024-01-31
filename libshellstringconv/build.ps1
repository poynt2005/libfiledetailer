Write-Host "[Builder][Info] ***** libshellstringconv builder *****" -ForegroundColor DarkGreen

Write-Host "[Builder][Info] Run with dotnet sdk" -ForegroundColor DarkGreen
Push-Location .\\FileString
& dotnet run
Pop-Location

Write-Host "[Builder][Info] Copy json file to go directory" -ForegroundColor DarkGreen
Copy-Item -Force -Path .\\FileString\\shell_mapping.json -Destination .\\stringconv\\shell_mapping.json

Write-Host "[Builder][Info] Compiling libshellstringconv dyn lib..."
go build --buildmode=c-shared -o libshellstringconv.dll