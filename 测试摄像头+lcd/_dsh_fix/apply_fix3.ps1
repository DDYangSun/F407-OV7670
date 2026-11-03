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
function Src([string]$n) { Join-Path $fix $n }

# ---------- 0. 备份 ----------
$bak = Join-Path $fix 'backup2'
New-Item -ItemType Directory -Force -Path $bak | Out-Null
foreach ($f in 'User\main.c','System\Delay.c','Hardware\SCCB.c','Hardware\SCCB.h') {
    Copy-Item (Join-Path $root $f) (Join-Path $bak ($f -replace '[\\/]','_')) -Force
}
'备份完成 -> backup2'

# ---------- 1. Delay.c：延时函数自初始化（GBK） ----------
$p = Join-Path $root 'System\Delay.c'
$t = Read-Txt $p $gbk
$guard = "`t/* fac_us 为 0 说明没人调用过 Delay_Init()，这里自动补上，避免延时全部变成空操作 */`r`n`tif (fac_us == 0) { Delay_Init((uint8_t)(SystemCoreClock / 1000000)); }`r`n"
foreach ($fn in 'void Delay_us\(uint32_t nus\)','void Delay_ms\(uint16_t nms\)') {
    $pat = $fn + '(\s*)\{'
    $m = [regex]::Matches($t, $pat)
    if ($m.Count -ne 1) { throw "Delay.c: $fn 匹配数=$($m.Count)" }
    $t = [regex]::Replace($t, $pat, { param($x) $x.Value + "`r`n" + $guard }, 1)
}
Write-Txt $p $t $gbk
'Delay.c  : Delay_us / Delay_ms 加入自初始化保护'

# ---------- 2. SCCB.c：追加 SDA 连线自检（UTF-8） ----------
$p = Join-Path $root 'Hardware\SCCB.c'
$t = Read-Txt $p $utf8
if ($t -notmatch 'SCCB_TestLine') {
    $t = $t.TrimEnd() + $LF + (Norm (Read-Txt (Src 'sccb_testline.txt') $utf8))
    Write-Txt $p $t $utf8
    'SCCB.c   : 已追加 SCCB_TestLine()'
} else { 'SCCB.c   : 已存在 SCCB_TestLine，跳过' }

# ---------- 3. SCCB.h：补充声明（UTF-8） ----------
$p = Join-Path $root 'Hardware\SCCB.h'
Write-Txt $p (Norm (Read-Txt (Src 'sccb_h.txt') $utf8)) $utf8
'SCCB.h   : 已更新（新增 SCCB_TestLine，去掉未实现的 SCCB_WaitEvent）'

# ---------- 4. main.c：Delay_Init + 屏上诊断信息（GBK） ----------
$p = Join-Path $root 'User\main.c'
Write-Txt $p (Norm (Read-Txt (Src 'main_c2.txt') $utf8)) $gbk
'main.c   : 已更新（Delay_Init + PID/VER/LINE 显示到屏幕）'

''
'================= 校验：Delay.c 关键处 ================='
$t = Read-Txt (Join-Path $root 'System\Delay.c') $gbk
($t -split "`r?`n" | Select-String 'fac_us == 0|void Delay_' ).Line
