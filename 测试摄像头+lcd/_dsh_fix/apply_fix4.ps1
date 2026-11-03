$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.Encoding]::UTF8

$root = 'D:\STM32F407VET6\项目\测试摄像头+lcd'
$fix  = Join-Path $root '_dsh_fix'
$gbk  = [Text.Encoding]::GetEncoding(936)
$utf8 = New-Object Text.UTF8Encoding($false)
$LF   = "`r`n"
function Read-Txt([string]$p, $enc) { [IO.File]::ReadAllText($p, $enc) }
function Write-Txt([string]$p, [string]$t, $enc) { [IO.File]::WriteAllText($p, $t, $enc) }
function Norm([string]$t) { (($t -replace "`r`n", "`n") -replace "`n", $LF) }

Copy-Item (Join-Path $root 'Hardware\lcd.c') (Join-Path $fix 'backup2\Hardware_lcd.c') -Force

$p = Join-Path $root 'Hardware\lcd.c'
$t = Read-Txt $p $gbk
$new = Norm (Read-Txt (Join-Path $fix 'lcd_show2.txt') $utf8)
$pat = 'void LCD_ShowOV7670PIC\s*\(\s*uint16_t\s*\*\s*Image\s*\)[\s\S]*$'
$m = [regex]::Matches($t, $pat)
if ($m.Count -ne 1) { throw "lcd.c 匹配数=$($m.Count)" }
Write-Txt $p ([regex]::Replace($t, $pat, { param($x) $new })) $gbk
'lcd.c : 已加入 OV7670_SWAP_BYTES 颜色字节序开关'

$log = Join-Path $fix 'build3.log'
if (Test-Path $log) { Remove-Item $log -Force }
$p2 = Start-Process -FilePath 'D:\KEIL\UV4\UV4.exe' -ArgumentList @('-b',(Join-Path $root 'Project.uvprojx'),'-j0','-o',$log) -Wait -PassThru
"UV4 exit=$($p2.ExitCode)"
if (Test-Path $log) {
    $t2 = [IO.File]::ReadAllText($log, $gbk)
    ($t2 -split "`r?`n" | Where-Object { $_ -ne '' }) | Select-Object -Last 20
}
