遇到 `core dumped` 時，重點是：**錯誤通常發生在更早的地方，只是在 `free()` 時才被偵測出來**。你這個案例就是 `fscanf` 寫爆記憶體，最後 `free()` 才報：

```text
free(): invalid pointer
Aborted (core dumped)
```

建議依照以下順序 debug。

---

## 1. 先用 AddressSanitizer，通常最快

重新編譯時加入：

```bash
make clean
make CFLAGS="-g -O0 -Wall -Wextra -std=c99 -fsanitize=address -fno-omit-frame-pointer" \
     LDFLAGS="-g -Wall -fsanitize=address" imageloader
```

或直接手動編譯：

```bash
gcc -g -O0 -Wall -Wextra -std=c99 -fsanitize=address \
    -fno-omit-frame-pointer -c imageloader.c

gcc -g -O0 -Wall -Wextra -std=c99 -fsanitize=address \
    -fno-omit-frame-pointer -c imageloadertester.c

gcc -g -fsanitize=address -o imageloadertester \
    imageloader.o imageloadertester.o
```

執行：

```bash
./imageloadertester testInputs/JohnConway.ppm \
    > studentOutputs/JohnConway.ppm
```

AddressSanitizer 通常會指出類似：

```text
ERROR: AddressSanitizer: heap-buffer-overflow
...
READ/WRITE of size 4
...
#0 ... in fscanf
#1 ... in readData imageloader.c:...
```

這比只看到 `free(): invalid pointer` 有用很多，因為它會指出**第一次非法寫入的位置**。

注意：修正完測試後，要重新編譯所有 `.o`，否則舊的 object file 可能仍然被使用：

```bash
make clean
make ...
```

---

## 2. 使用 Valgrind

先確認有 debug symbol：

```bash
make clean
make CFLAGS="-g -O0 -Wall -Wextra -std=c99" \
     LDFLAGS="-g -Wall"
```

執行 Valgrind：

```bash
valgrind \
  --leak-check=full \
  --track-origins=yes \
  --show-leak-kinds=all \
  ./imageloadertester testInputs/JohnConway.ppm \
  > studentOutputs/JohnConway.ppm
```

如果想把 Valgrind 訊息另外存檔：

```bash
valgrind \
  --leak-check=full \
  --track-origins=yes \
  --show-leak-kinds=all \
  --log-file=valgrind.log \
  ./imageloadertester testInputs/JohnConway.ppm \
  > studentOutputs/JohnConway.ppm
```

然後查看：

```bash
less valgrind.log
```

你這個案例可能看到類似：

```text
Invalid write of size 4
    at ... fscanf
    by ... readData imageloader.c:...
Address ... is 3 bytes after a block of size ...
```

因為：

```c
uint8_t R;
```

只有 1 byte，但：

```c
%u
```

要求的是：

```c
unsigned int *
```

通常會寫入 4 bytes。

---

## 3. 使用 GDB 查看 core dump

### 先打開 core dump

```bash
ulimit -c unlimited
```

執行程式：

```bash
./imageloadertester testInputs/JohnConway.ppm \
    > studentOutputs/JohnConway.ppm
```

如果程式 crash，查看是否產生 core：

```bash
ls -lh core*
```

有些 Linux 系統會把 core 存在其他位置，可查看設定：

```bash
cat /proc/sys/kernel/core_pattern
```

### 使用 core 啟動 GDB

```bash
gdb ./imageloadertester core
```

進入 GDB 後：

```gdb
bt
```

`bt` 是 backtrace，查看呼叫堆疊。

接著：

```gdb
frame 0
frame 1
list
```

常見情況會停在：

```text
free()
```

這只能說明「free 時發現 heap 已經壞了」，不一定是 bug 真正發生的位置。

也可以查看所有 thread：

```gdb
thread apply all bt
```

離開：

```gdb
quit
```

---

## 4. 直接在 GDB 中執行

不一定要產生 core，也可以直接：

```bash
gdb ./imageloadertester
```

進入後：

```gdb
run testInputs/JohnConway.ppm
```

程式停止後：

```gdb
bt
```

如果停在 `free()`，可以查看上一層：

```gdb
up
list
```

對你的程式而言，可能會看到：

```text
#0  free
#1  freeImage (...) at imageloader.c:...
#2  main (...) at imageloadertester.c:...
```

這表示 `freeImage()` 執行時發現 heap 損壞，但仍要回頭檢查之前的記憶體寫入。

---

## 5. 在可疑位置設 breakpoint

例如你懷疑讀取 pixel 的部分：

```bash
gdb ./imageloadertester
```

```gdb
break readData
run testInputs/JohnConway.ppm
```

逐步執行：

```gdb
next
next
```

查看變數：

```gdb
print image
print image->rows
print image->cols
print i
print j
print image->image[i][j]
```

也可以在 `fscanf` 前設 breakpoint：

```gdb
break imageloader.c:某一行
run testInputs/JohnConway.ppm
```

但是要注意：GDB 不一定能直接偵測 heap overflow；它主要是讓你觀察程式執行流程與變數。

---

## 6. 為什麼編譯器沒有 warning？

這種程式碼：

```c
fscanf(f, "%u", &some_uint8_variable);
```

編譯器有時不會警告，因為 `fscanf` 是 variadic function：

```c
int fscanf(FILE *stream, const char *format, ...);
```

第三個參數的型別是透過 `...` 傳入，編譯器無法像普通函式一樣完整檢查它是否為正確的 pointer type。

所以即使開了：

```bash
-Wall -Wextra
```

仍可能沒有警告。

可以額外使用：

```bash
-Wformat=2
```

但它也不一定能抓到所有 `scanf` pointer 型別錯誤。對 `uint8_t`，推薦這樣寫：

```c
unsigned int r;
unsigned int g;
unsigned int b;

if (fscanf(f, "%u %u %u", &r, &g, &b) != 3) {
    // 處理輸入錯誤
}

image->image[i][j].R = (uint8_t)r;
image->image[i][j].G = (uint8_t)g;
image->image[i][j].B = (uint8_t)b;
```

也可以使用 `%hhu`：

```c
fscanf(f, "%hhu %hhu %hhu",
       &image->image[i][j].R,
       &image->image[i][j].G,
       &image->image[i][j].B);
```

但實務上先讀到 `unsigned int`，再檢查範圍並轉型，通常更清楚：

```c
if (r > 255 || g > 255 || b > 255) {
    // 像素值不合法
}
```

---

## 7. 一般化的 debug 流程

看到：

```text
Segmentation fault
Aborted (core dumped)
free(): invalid pointer
double free or corruption
```

可以照這個流程：

```bash
# 1. 完整重編，避免使用舊的 .o
make clean
make CFLAGS="-g -O0 -Wall -Wextra -std=c99" \
     LDFLAGS="-g -Wall"

# 2. 優先使用 ASan
make clean
make CFLAGS="-g -O0 -fsanitize=address -fno-omit-frame-pointer" \
     LDFLAGS="-g -fsanitize=address"

# 3. 或使用 Valgrind
valgrind --leak-check=full --track-origins=yes ./program args

# 4. 必要時使用 GDB
gdb ./program
run args
bt
```

判斷方式：

- `heap-buffer-overflow`：陣列越界或配置大小錯誤
- `stack-buffer-overflow`：區域陣列越界
- `use-after-free`：釋放後仍使用
- `double-free`：同一指標釋放兩次
- `invalid free`：`free()` 的指標不是合法配置起始位置
- `memory leak`：配置後沒有釋放

對目前這個案例，最有效的命令是：

```bash
make clean
make CFLAGS="-g -O0 -Wall -Wextra -std=c99 -fsanitize=address -fno-omit-frame-pointer" \
     LDFLAGS="-g -fsanitize=address" imageloader
```

它應該會把問題從「在 `free()` 崩潰」提前定位到 `readData()` 中錯誤的 `fscanf()` 呼叫。