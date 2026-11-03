$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [Text.Encoding]::UTF8

$root = 'D:\STM32F407VET6\项目\测试摄像头+lcd'
$fix  = Join-Path $root '_dsh_fix'
$gbk  = [Text.Encoding]::GetEncoding(936)
$LF   = "`r`n"

function Read-Utf8([string]$p) { [IO.File]::ReadAllText($p, [Text.Encoding]::UTF8) }
function Read-Gbk([string]$p)  { [IO.File]::ReadAllText($p, $gbk) }
function Write-Gbk([string]$p, [string]$t) { [IO.File]::WriteAllText($p, $t, $gbk) }
function Norm([string]$t) { (($t -replace "`r`n", "`n") -replace "`n", $LF) }
function Src([string]$n) { Join-Path $fix $n }

# ---------- 0. 备份原始文件 ----------
$bak = Join-Path $fix 'backup'
New-Item -ItemType Directory -Force -Path $bak | Out-Null
$targets = @('User\main.c','Hardware\OV7670.c','Hardware\OV7670.h','Hardware\SCCB.c','Hardware\lcd.c')
foreach ($f in $targets) {
    $src = Join-Path $root $f
    $dst = Join-Path $bak ($f -replace '[\\/]', '_')
    Copy-Item $src $dst -Force
}
"备份完成 -> $bak"

# ---------- 1. OV7670.c：删掉 DCMI 中断 / NVIC / 提前使能 DCMI ----------
$p = Join-Path $root 'Hardware\OV7670.c'
$t = Read-Gbk $p
$new = Norm (Read-Utf8 (Src 'ov7670_noit.txt'))
$pat = 'DCMI_ITConfig\s*\(\s*DCMI_IT_FRAME[\s\S]*?DCMI_Cmd\s*\(\s*ENABLE\s*\)\s*;'
$m = [regex]::Matches($t, $pat)
if ($m.Count -ne 1) { throw "OV7670.c: DCMI 中断段匹配数=$($m.Count)，应为1，已中止" }
$t2 = [regex]::Replace($t, $pat, { param($x) $new })
Write-Gbk $p $t2
'OV7670.c : 已移除 DCMI_ITConfig(FRAME/OVF)、DCMI NVIC 和提前的 DCMI_Cmd(ENABLE)'

# ---------- 2. OV7670.c：追加器件 ID 自检函数 ----------
$p = Join-Path $root 'Hardware\OV7670.c'
$t = Read-Gbk $p
$add = Norm (Read-Utf8 (Src 'ov7670_checkid.txt'))
$marker = 'OV7670_CheckID'
if ($t -notmatch $marker) {
    Write-Gbk $p ($t.TrimEnd() + $LF + $add)
    'OV7670.c : 已追加 OV7670_CheckID()'
} else { 'OV7670.c : 已存在 OV7670_CheckID()，跳过' }

# ---------- 3. OV7670.h：添加原型 ----------
$p = Join-Path $root 'Hardware\OV7670.h'
$t = Read-Gbk $p
if ($t -notmatch 'OV7670_CheckID') {
    $t = $t -replace '(?m)^void OV7670DMA_DCMItoSRAM\(void\);\s*$', ('void OV7670DMA_DCMItoSRAM(void);' + $LF + 'uint8_t OV7670_CheckID(void);')
    Write-Gbk $p $t
    'OV7670.h : 已添加 uint8_t OV7670_CheckID(void);'
} else { 'OV7670.h : 已存在，跳过' }

# ---------- 4. SCCB.c：整体重写（换到 PB8/PB9，不与 LCD 抢脚） ----------
$p = Join-Path $root 'Hardware\SCCB.c'
Write-Gbk $p (Norm (Read-Utf8 (Src 'sccb_c.txt')))
'SCCB.c   : 已重写，SCL=PB8 / SDA=PB9（原 PB10/PB1 与 LCD_DC 冲突）'

# ---------- 5. lcd.c：LCD_ShowOV7670PIC 补窗口 + 处理 160/128 宽度 ----------
$p = Join-Path $root 'Hardware\lcd.c'
$t = Read-Gbk $p
$new = Norm (Read-Utf8 (Src 'lcd_show.txt'))
$pat = 'void LCD_ShowOV7670PIC\s*\(\s*uint16_t\s*\*\s*Image\s*\)[\s\S]*$'
$m = [regex]::Matches($t, $pat)
if ($m.Count -ne 1) { throw "lcd.c: LCD_ShowOV7670PIC 匹配数=$($m.Count)，已中止" }
Write-Gbk $p ([regex]::Replace($t, $pat, { param($x) $new }))
'lcd.c    : LCD_ShowOV7670PIC 已重写（加 LCD_Address_Set，按 128 宽裁剪）'

# ---------- 6. main.c：整体重写（初始化顺序 + SCCB 自检） ----------
$p = Join-Path $root 'User\main.c'
Write-Gbk $p (Norm (Read-Utf8 (Src 'main_c.txt')))
'main.c   : 已重写（先点屏 -> 再配摄像头 -> 自检 -> 采集）'

''
'================= 校验：OV7670_DCMI_Init 现在的结尾 ================='
$t = Read-Gbk (Join-Path $root 'Hardware\OV7670.c')
$i = $t.IndexOf('DCMI_Init (&DCMI_InitStructure);')
$t.Substring($i, [Math]::Min(700, $t.Length - $i))
