# GetIP.ps1

$wifiInterfaceAlias = "Wi-Fi"
$targetSSID = "Gukgung_Wifi"

# 현재 연결된 Wi-Fi 네트워크의 SSID를 가져옴
$interfaceInfo = netsh wlan show interfaces

# SSID 정보 추출
$ssidLine = ($interfaceInfo | Select-String " SSID").Line

if ($ssidLine) {
    $connectedSSID = $ssidLine.Split(":")[1].Trim()
    if ($connectedSSID -eq $targetSSID) {
        # Wi-Fi 인터페이스의 IPv4 주소를 가져옴
        $ip = (Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias $wifiInterfaceAlias).IPAddress
        $ip
    } else {
        # 연결된 SSID가 목표 SSID가 아닐 경우 빈 문자열 반환
        Write-Host ""
    }
} else {
    # SSID 정보를 가져오지 못한 경우 빈 문자열 반환
    Write-Host ""
}
