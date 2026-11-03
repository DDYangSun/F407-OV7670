$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.Encoding]::UTF8

$root = 'D:\STM32F407VET6\项目\测试摄像头+lcd'
$fix  = Join-Path $root '_dsh_fix'
$bak  = Join-Path $fix 'backup'
$gbk  = [Text.Encoding]::GetEncoding(936)
$utf8 = New-Object Text.UTF8Encoding($false)
$LF   = "`r`n"

function Read-Txt([string]$p, $enc) { [IO.File]::ReadAllText($p, $enc) }
function Write-Txt([string]$p, [string]$t, $enc) { [IO.File]::WriteAllText($p, $t, $enc) }
function Norm([string]$t) { (($t -replace "`r`n", "`n") -replace "`n", $LF) }
function Src([string]$n) { Join-Path $fix $n }

# ---------- 1. 回滚编码被改坏的两个文件 ----------
Copy-Item (Join-Path $bak 'Hardware_OV7670.c') (Join-Path $root 'Hardware\OV7670.c') -Force
Copy-Item (Join-Path $bak 'Hardware_SCCB.c')   (Join-Path $root 'Hardware\SCCB.c')   -Force
'OV7670.c / SCCB.c 已从备份还原'

# ---------- 2. OV7670.c (UTF-8) ----------
$p = Join-Path $root 'Hardware\OV7670.c'
$t = Read-Txt $p $utf8
$new = Norm (Read-Txt (Src 'ov7670_noit.txt') $utf8)
$pat = 'DCMI_ITConfig\s*\(\s*DCMI_IT_FRAME[\s\S]*?DCMI_Cmd\s*\(\s*ENABLE\s*\)\s*;'
$m = [regex]::Matches($t, $pat)
if ($m.Count -ne 1) { throw "OV7670.c: 匹配数=$($m.Count)，中止" }
$t = [regex]::Replace($t, $pat, { param($x) $new })
$t = $t.TrimEnd() + $LF + (Norm (Read-Txt (Src 'ov7670_checkid.txt') $utf8))
Write-Txt $p $t $utf8
'OV7670.c : DCMI 中断段已移除 + 已追加 OV7670_CheckID()  (UTF-8)'

# ---------- 3. SCCB.c (UTF-8) ----------
$p = Join-Path $root 'Hardware\SCCB.c'
Write-Txt $p (Norm (Read-Txt (Src 'sccb_c.txt') $utf8)) $utf8
'SCCB.c   : 已重写为 SCL=PB8 / SDA=PB9  (UTF-8)'

''
'================= 校验 1：OV7670_DCMI_Init 结尾 ================='
$t = Read-Txt (Join-Path $root 'Hardware\OV7670.c') $utf8
$i = $t.IndexOf('DCMI_Init (&DCMI_InitStructure);')
$t.Substring($i, [Math]::Min(760, $t.Length - $i))

''
'================= 校验 2：SCCB.c 引脚 ================='
(Read-Txt (Join-Path $root 'Hardware\SCCB.c') $utf8) -split "`n" | Select-String -Pattern 'SCCB_SCL_PIN|SCCB_SDA_PIN' | Select-Object -First 8 | ForEach-Object { $_.Line.Trim() }

''
'================= 校验 3：lcd.c 尾部 (GBK) ================='
$t = Read-Txt (Join-Path $root 'Hardware\lcd.c') $gbk
$t.Substring([Math]::Max(0, $t.Length - 1200))
