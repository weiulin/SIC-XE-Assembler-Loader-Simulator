# SIC-XE-Assembler-Loader-Simulator
📌 專案簡介 

本專案模擬簡化指令電腦（Simplified Instructional Computer, SIC/XE）的組譯器（Assembler）與載入器（Linker/Loader），使用 C 語言實作，模擬將 source code 組譯為機器碼，再載入記憶體並模擬執行流程。


🛠 使用技術

C 語言

SIC/XE 指令集架構（指令格式、位址模式）

組譯流程設計（Pass1 / Pass2）

Symbol Table / Literal Table 建立

物件程式格式（Header, Text, End records）

Linking 與 relocation


🎯 專案特色


✅ 支援兩階段組譯（Pass1, Pass2）

✅ 可處理 Symbol Table 與程式 Counter 管理

✅ 產出物件程式格式，包含 Header、Text、End Record

✅ 模擬載入器載入多個物件程式

✅ 執行過程中顯示記憶體狀態與指令執行情況

✅ 模擬 SIC/XE 部分延伸指令與格式


🔧 組譯流程簡述

Pass 1：

讀入原始程式碼

建立符號表（Symbol Table）

計算指令位置（location counter）

Pass 2：

根據符號表產生機器碼

組成 object code 並格式化輸出


🖥️ 範例組譯輸入

COPY    START   1000
FIRST   STL     RETADR
CLOOP   JSUB    RDREC
        LDA     LENGTH
        COMP    ZERO
        JEQ     ENDFIL
...


📤 輸出物件程式範例

H^COPY^001000^00107A
T^001000^1E^141033^482039^001036^...
E^001000


🧪 執行與展示

gcc assembler.c -o assembler
./assembler input.asm
輸出：物件程式檔、Symbol Table、機器碼對照表

若有 loader 模組，可接續執行載入流程


📁 檔案說明
檔案名稱	            說明
assembler.c	        主程式，實作 Pass 1 & Pass 2
loader.c            載入器模組，模擬連結與重定位
input.asm	          測試程式碼範例
symbol_table.txt	  組譯後產生的符號表
object_code.txt	    輸出的機器碼與物件程式
