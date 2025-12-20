# Fail quietly
$ErrorActionPreference = 'SilentlyContinue'

# Find the RP2350 USB CDC device by VID (adjust if needed)
$port = Get-CimInstance Win32_PnPEntity |
    Where-Object {
        $_.PNPDeviceID -match 'VID_2E8A' -and $_.Name -match 'COM\d+'
    } |
    Select-Object -First 1

if (-not $port) {
    exit
}

# Extract COM port name (e.g. COM5)
$com = $port.Name -replace '.*\((COM\d+)\).*','$1'

# Open serial port
$sp = New-Object System.IO.Ports.SerialPort $com,115200,'None',8,'One'
$sp.NewLine = "`n"
$sp.Open()

# Send hostname
$sp.WriteLine("host: $env:COMPUTERNAME")

# Send IPv4 addresses
Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object { $_.IPAddress -notlike '169.254.*' } |
    ForEach-Object {
        $iface = $_.InterfaceAlias -replace '\s+','_'
        $sp.WriteLine("1: $iface inet $($_.IPAddress)")
    }

# Close port
$sp.Close()