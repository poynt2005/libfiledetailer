
Write-Host "[Builder][Info] *****  libfiledetailer core builder *****" -ForegroundColor DarkGreen

Write-Host "[Builder][Info] Finding VS2022 installation infomation" -ForegroundColor DarkGreen
$VS2022_QueryResult = Get-CimInstance MSFT_VSInstance -Namespace root/cimv2/vs | Where-Object { $_.ChannelId -eq "VisualStudio.17.Release" } 

if ($VS2022_QueryResult -eq $null) {
    Write-Host "[Builder][Error] Fatal error, cannot find VS2022 installed" -ForegroundColor DarkRed
    Exit -1
}


Write-Host "[Builder][Info] Finding MSBuild for VS2022..." -ForegroundColor DarkGreen
$msbuild_path = [System.IO.Path]::Combine($VS2022_QueryResult.InstallLocation, "MSBuild\\Current\\Bin\\MSBuild.exe")

if (![System.IO.File]::Exists($msbuild_path)) {
    Write-Host "[Builder][Error] Fatal error, cannot find MSBuild installed" -ForegroundColor DarkRed
    Exit -1
}

Write-Host "[Builder][Info] Building project..." -ForegroundColor DarkGreen
& $msbuild_path  .\\libfiledetailer.sln /property:Configuration=Release