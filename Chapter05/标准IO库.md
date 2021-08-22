## 5标准I/O库

### 5.2流和FILE对象

标准I/O与文件I/O的区别：

- 文件I/O函数都是围绕`fd`的，当打开一个文件时，即返回一个`fd`，然后该`fd`作用于后续的I/O操作。
- 标准I/O函数都是围绕**流**(stream)进行。当用标准I/O库打开或创建一个文件时，便使一个流与一个文件相关联。

> 本质上标准I/O是在用户层创建一个缓冲区用来拷贝内核层中的虚拟页数据，用来减少用户层与内核层之间的频繁调度。

**流的定向**(stream's orientation)决定了所读、写的字符是单字节还是多字节。

> 对于ASCII字符集，一个字符用一个字节表示。一个字节8bit位所以总共255个。
>
> 对于国际字符集，一个字符可用多个字节表示。(也称宽字节)

当一个流最初被创建时，它并没有定向。

- 若在未定向的流上使用一个宽字节I/O函数，则会将该流的定向设为宽定向。
- 若在未定向的流上使用一个单字节I/O函数，则会将该流的定向设为单字节定向。

只有两个函数可以改变流的定向。`freopne()`清除一个流的定向；`fwide()`可用于设置流的定向。

```c
#include<stdio.h>
#include<wchar.h>
int fwide(FILE *fp, int mode);
//函数的返回值：若流是宽定向的，返回正值；若流是字节定向的，返回负值；若流是未定向的，返回0
```

根据`mode`参数的不同，`fwide()`执行不同的工作：

- 若`mode`参数值为负，`fwide()`将试图使指定的流是字节定向。
- 若`mode`参数值为正，`fwide()`将试图使指定的流是宽定向。
- 若`mode`参数值为0，`fwide()`将不试图设置流的定向，但返回标识该流定向的值。

> 注意，`fwide()`并不改变已定向流的方向。
>
> 且`fwide()`无出错返回，所以在调用`fwide()`前先清除`errno`，从`fwide()`返回时检查`errno`的值。

当打开一个流时，标准I/O函数`fopen()`回返回一个指向`FILE`对象的指针。该对像通常是一个结构，它包含了标准I/O库为管理该流需要的所有信息，包括用于实际I/O的`fd`、指向用于该流缓冲区的指针、缓冲区的长度、当前在缓冲区中的字符数以及出错标志等。

> 书中称指向`FLIE`对象的指针(类型为`FILE*`)为文件指针。

### 5.3标准输入、标准输出和标准错误

标准I/O库也对一个进程预定义了3个流，并且这3个流可以自动地被进程使用。与文件I/O `STDIN_FILENO` `STDOUT_FILENO` `STDERR_FILENO`相对应的标准I/O流预定义的文件指针`stdin`、`stdout`和`stderr`。这三个文件指针定义在头文件`<stdio.h>`。

### 5.4缓冲

标准I/O库提供缓冲的目的是尽可能减少使用`read()`和`write()`调用的次数。对每个I/O流自动得进行缓冲管理，从而避免了应用程序需要考虑这一点所带来的麻烦。

标准I/O提供了以下3种类型的缓冲：

1. 全缓冲。在填满标准I/O缓冲区后才进行实际I/O操作。
   - 对驻留在磁盘上的文件通常是由标准I/O库实施全缓冲的。
2. 行缓冲。当在输入和输出中遇到换行符时，标准I/O库执行I/O操作。
   - 由于缓冲区的长度是固定的，所以只要填满了缓冲区，那么即使还没有写入一个换行符，也进行I/O操作。
   - 任何时候只要通过标准I/O库从一个不带缓冲的流或一个行缓冲的流得到输入数据(向内核请求需要的数据)，那么就会冲洗所有行缓冲输出流。

> 在一个流上执行第一次I/O操作时，相关标准I/O函数通常调用`malloc()`获得需使用的缓冲区。
>
> 术语**冲洗**(flush)说明标准I/O缓冲区的写操作。缓冲区可由标准I/O列程自动地冲洗，或者可以调用`fflush()`手动冲洗一个流。

3. 不带缓冲。标准I/O库不对字符进行缓冲存储。
   - 实际的I/O操作无需冲洗就可立即输出。标准错误流`stderr`通常是不带缓冲的。

> ISO C要求下列缓冲特征：
>
> - 当且仅当标准输入和标准输出并不指向交互式设备时，它们才是全缓冲的。
> - 标准错误绝不会是全缓冲。

调用`setbuf()`和`setvbuf()`可以更改流的缓冲类型。

```c
#include<stdio.h>
void setbuf(FILE *restrict fp, char *restrict buf);
int servbuf(FILE *restrict fp, char *restrict buf, int mode, size_t size);
//两个函数的返回值：若成功，返回0；若出错，返回非0
```

这些函数一定要在流已经被打开后调用，而且也应在对该流执行任何一个操作之前调用。

使用`setbuf()`可以打开或关闭缓冲机制：

1. 为了带缓冲进行I/O，参数`buf`必须指向一个长度为`BUFSIZ`的缓冲区。

> 该常量定义在`<stdio.h>`中，通常在此之后该流就是全缓冲的，但是如果该流与一个终端设备相关，那么某些系统也可将其设置为行缓冲。

2. 为了关闭缓冲，将`buf`设置为`NULL`。

使用`setvbuf()`可以精确的说明所需要的缓冲类型。由`mode`参数实现：`_IOFBF`：全缓冲。 `_IOLBF`：行缓冲。 `_IONBF`：不带缓冲。

- 若指定了一个不带缓冲的流，则忽略`buf`和`size`参数。
- 若指定全缓冲或行缓冲：
  - `buf`和`size`可选择地指定一个缓冲区及其长度。
  - `buf`为`NULL`时，标准I/O库将自动地为该流分配`BUFSIZ`长度的缓冲区。

> 若在一个函数内分配的是自动变量(栈区)的标准I/O缓冲区，则从该函数返回之前，必须关闭该流。
>
> 因为一些实现将缓冲区的一部分用于存放它自己的管理操作信息，所以可以存在缓冲区的实际数据字节数小于`size`。

由系统选择缓冲区的长度，并自动分配缓冲区，关闭此流时将被自动释放缓冲区。

调用`fflush()`将强制冲洗一个流。

```c
 #include<stdio.h>
int fflush(FILE *fp);
//函数的返回值：若成功，返回0；若出错，返回EOF
```

此函数使该流所有未写的数据都被传送至内核。若`fp`是`NULL`，则此函数将导致所有输出流被冲洗。

### 5.5打开流

下列3个函数将打开一个标准I/O流。

```c
#include<stdio.h>
FILE *fopen(const char *restrict pathname, const char *restrict type);
FILE *freopen(const char *restrict pathname, const char *restrict type, FILE *restrict fp);
FILE *fdopen(int fd, const char *type);
//三个函数的返回值：若成功，返回文件指针；若出错，返回NULL
```

这三个函数的区别是：

1. `fopen()`打开路径名为`pathname`的一个指定的文件。
2. `freopen()`在一个指定的流上打开一个指定的文件，如若该流已经打开，则先关闭该流。若该流已经定向，则使用`freopen()`清除该定向。此函数一般用于将一个指定的文件打开为一个预定义的流：标准输入、标准输出和标准错误。
3. `fdopen()`使一个标准的I/O流与参数`fd`相结合。因为一些特殊类型的文件不能用标准I/O函数打开，所以先调用设备专用函数以获得一个`fd`，然后用`fdopen()`使一个标准I/O流与该`fd`相结合。

三个函数中`type`参数指定对该I/O流的读、写方式：

- `r`或`rb`：为读而打开。																			== `O_RDONLY`
- `w`或`wb`：把文件截断至0长，或为写而创建。							           == `O_WRONLY｜O_CREAT｜O_TRUNC`
- `a`或`ab`：追加；为在文件尾写而打开，或为写而创建。                      == `O_WROLNY｜O_CREAT｜O_APPEND`
- `r+`或`r+b`或`rb+`：为读和写而打开。                                                     == `O_RDWR`
- `w+`或`w+b`或`wb+`：把文件截断至0长，或为读和写而打开。                == `O_RDWR｜O_CREAT｜O_TRUNC`
- `a+`或`a+b`或`ab+`：为在文件尾读和写而打开或创建。                          == `O_RDWR｜O_CREAT｜O_APPEND`

> 使用字符`b`作为`type`的一部分，这使得标准I/O系统可以区别文本和二进制文件。
>
> 因为unix内核并不对这两种文件进行区分，所以在unix系统环境下指定字符`b`作为`type`的一部分实际上并无作用。

 对于`fdopen()`，`type`参数的意义稍有区别。因为该`fd`已经按权限打开，所以`fdopen()`为写而打开并不截断该文件。此外，标准I/O追加写方式也不能用于创建该文件。

当用追加写类型打开一个文件后，每次写都将数据写到文件的当前尾端处。如果有多个进程用标准I/O追加写方式打开同一文件，那么来自每个进程的数据都将正确地写到文件中。

当以读和写类型打开一个文件时(`type`参数中`+`号)，具有下列限制：

- 若中间没有`fflush()`、`fseek()`、`fsetpos()`或`rewind()`，则在输出的后面不能直接跟随输入。
- 若中间没有`fseek()`、`fsetpos()`或`rewind()`，或者一个输入操作没有到达文件尾端，则在输入操作之后不能直接跟随输出。

> 在指定`w`或`a`类型创建一个新文件时，实际无法说明该文件的访问权限位。POSIX.1要求实现使用如下的权限位集来创建文件：
>
> `S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH`          同时受`umask`的影响。

调用`fclose()`可以关闭一个打开的流。

```c
#include<stdio.h>
int fclose(FILE *fp);
//函数的返回值：若成功，返回0；若出错，返回EOF
```

在该文件被关闭之前，冲洗缓冲中的输出数据。缓冲区中的任何输入数据被丢弃。如果标准I/O库已经为该流自动分配了一个缓冲区，则释放该缓冲区。

当一个进程正常终止时(直接调用`exit()`或`return()`)，则所有存在未写缓冲数据的标准I/O流都被冲洗，所有关闭所有打开的标准I/O流。

### 5.6读和写流

一旦打开了流，则可在3中不同类型的非格式化I/O中进行选择，对其进行I/O操作：

1. 每次一个字符的I/O。一次读或写一个字符，如果流是带缓冲的，则标准I/O函数处理所有缓冲。
2. 每次一行的I/O。若果想要一次读或写一行，则使用`fgets()`和`fputs()`。每行都以一个换行符终止。
3. 直接I/O。`fread()`和`fwrite()`支持这种类型的I/O。每次I/O操作读或写某种数量的对象，而每个对象具有指定的长度。这两个函数常用于从二进制文件中每次读或写一个结构。

#### 5.6.1输入函数

下列3个函数可用于一次读一个字符。

```c
#include<stdio.h>
int getc(FILE *fp);
int fgets(FILE *fp);
int getchar(void);
//三个函数的返回值：若成功，返回下一个字符；若已到文件尾端或出错，返回EOF
```

函数`getchar()`等同于`getc(stdin)`。前两个函数的区别是，`getc()`可被实现为宏，而`fgetc()`不能实现为宏：

1. `getc()`的参数不应当是具有副作用的表达式，因为他可能会被计算多次。

> 比如` #define a x + y` 和` #define a (x + y)`。  当计算a * 3时，前者为x + y * 3，后者为(x + y) * 3。
>
> 宏只进行替换，不进行逻辑判断。返回值EOF在<stdio.h>中被要求是一个负值，其值经常是-1。

这3个函数不管出错还是到达文件尾端都返回`EOF`。为了区分这两种不同的情况，必须调用`ferror()`和`feof()`检查。

```c
#include<stdio.h>
int ferror(FILE *fp);
int feof(FILE *fp);
//两个函数的返回值：若条件为真，返回非0(真)；否则返回0(假)
void clearerr(FILE *fp);
```

每个流在`FILE`对象中维护了两个标志：**出错标志**和**文件结束标志**。而调用`clearerr()`可以清除这两个标志。

调用`ungetc()`将从流中读取的单字节再压送回流中。

```c
#include<stdio.h>
int ungetc(int c, FILE *fp)
//函数的返回值：若成功，返回c；若出错，返回EOF
```

压送回到流中的字符以后又可以从流中读出，但读出字符的顺序与压送回的顺序相反。(单调栈性质)

可以执行任意次数的压入单个字符，但是不支持一次压入多个字符。

不能回送`EOF`，但是但已经达到文件尾端时，仍可以回送一个字符。由于成功调用`ungetc()`回清除该流的文件结束标志，所以下次读该字符后还会返回一个`EOF`。

#### 5.6.2输出函数

下列3个函数可用于一次读一个字符。

```c
#include<stdio.h>
int putc(int c, FILE *fp);
int fputc(int c, FILE *fp);
int putchar(int c);
//三个函数的返回值：若成功，返回c；若出错，返回EOF
```

与输入函数一样，`putchar(c)`等同于`putc(c， stdout)`，`putc()`可以被实现为宏，而`fputc()`不能实现为宏。

### 5.7每次一行I/O

下列2个函数可用于一次输入一行。

```c
#include<stdio.h>
char *fgets(char *restrict buf, int n, FILE *restrict fp);
char *gets(char *buf);
//两个函数的返回值：若成功，返回buf，若已达到文件尾端或出错，返回NULL
```

这两个函数都指定了缓冲区的地址，读入的行将送入其中，`gets()`从标准输入读，而`fgets()`则从指定的流读。

`fgets()`必须指定缓冲区的长度`n`，此函数一直读到下一个换行符为止，但是不超过n-1个字符。

- 该缓冲区以`null`字节结尾。
- 如果该行包括最后一个换行符的字符数超过n-1，则`fgets()`值返回一个不完整的行，下次调用会继续读该行。

> 不推荐使用`gets()`，可能产生缓冲区溢出bug。

`fputs()`和`puts()`可用于一次输出一行。

```c
#include<stdio.h>
int fputs(const char *restrict str, FILE *restrict fp);
int puts(const char *str);
//两个函数的返回值：若成功，返回非符负值；若出错，返回EOF
```

`fputs()`将一个以`null`字节终止的字符串写到指定的流，尾端的终止符`null`不写出。

> 若需要每次输出一行，则需在终止符前加上换行符。

`puts()`将一个以`null`字节终止的字符串写到标准输出，终止符不写出。但是`puts()`回随后将一个换行符写到标准输出。

### 5.9二进制I/O

下列2个函数以执行二进制I/O操作。

```c
#include<stdio.h>
size_t fread(void *restrict ptr, size_t size, size_t nobj, FILE *restrict fp);
size_t fwrite(const void *restrict ptr, size_t size, size_t nobj, FILE *restrict fp);
//两个函数的返回值：读或写的对象数
```

两个函数中参数`ptr`指向待读写的缓冲区地址，参数`size`说明单个对象的字节大小，参数`nobj`对面对象的个数。

`fread()`和`fwrite()`返回读或写的对象数。

- 对于读，如果出错或到达文件尾端，返回值都可能小于`nobj`。应调用`ferror()`和`feof()`检查。
- 对于写，如果返回值小于`nobj`，则出错。

> 使用二进制I/O的基本问题是，它只能用于读在同一系统上已写的数据。主要是由于
>
> 1. 不同的系统对数据对齐的方式不同。
> 2. 不同的系统的字节序不同，存在大端字节序和小端字节序。

### 5.10定位流

下列3个函数可用于标准I/O流的定位。

```c
#include<stdio.h>
long ftell(FILE *fp);
//函数的返回值：若成功，返回当前文件位置指示；若出错，返回-1
int fseek(FILE *fp, long offset, int whence);
//函数的返回值：若成功，返回0；若出错，返回-1
void rewind(FILE *fp);
```

处理对象不同，函数的行为有所不同：

- 对于一个二进制文件，其文件位置指示器是从文件起始位置开始度量，并以字节为度量单位的。`ftell()`用于二进制文件时其返回值就是这种字节位置。为了用`fseek()`定位一个二进制文件，必须指定一个字节`offset`，以及解释这种偏移量的方式。参数`whence`的值与`lseek()`的相同。
- 对于一个文本文件，当前位置可能不以简单的字节偏移量来度量。参数`whence`必须是`SEEK_SET`，且参数`offset`只能有两种值：`0`(后退到文件的起始位置)，或是对该文件的`ftell()`所返回的值。

调用`rewind()`可将一个流设置到文件的起始位置。

除了偏移量类型是`off_t`而非`long`以外，`ftello()`与`ftell()`相同，`fseeko()`与`fseek()`相同。

```c
#include<stdio.h>
off_t ftello(FILE *fp);
//函数的返回值：若成功，返回当前文件位置；若出错，返回(off_t)-1
int  fseeko(FILE *fp, off_t offset, int whence);
//函数的返回值：若成功，返回0；若出错，返回-1
```

若需要移植到非unix系统上运行的应用程序应当使用`fgetpos()`和`fsetpos()`。

```c
#include<stdio.h>
int fgetpos(FILE *restrict fp, fpos_t *restrict pos);
int fsetpos(FILE *fp, const fpos_t *pos);
//两个函数的返回值：若成功，返回0；若出错，返回非0
```

`fgetpos()`将文件位置指示器的当前值存入由`pos`指向的对象中。在以后调用`fsetpos()`时，可以使用此值将流重新定位至该位置。

### 5.11格式化I/O

#### 5.11.1格式输出

下列5个`printf`函数可用于处理格式化输出。

```c
#include<stdio.h>
int printf(const char *restrict format, ...);
int fprintf(FILE *restrict fp, const char *restrict format, ...);
int dprintf(int fd, const char *restrict format, ...);
//三个函数的返回值：若成功，返回输出字符数；若出错，返回负值
int sprintf(char *restrict buf, const char *restrict format, ...);
//函数的返回值：若成功，返回存入数组的字符数，若编码出错，返回符值
int snprintf(char *restrict buf, size_t n, const char *restrict format, ...);
//函数的返回值：若缓冲区足够大，返回将要存入数组的字符数；若编码出错，返回负值
```

`printf()`将格式化数据写到标准输出。

`fprintf()`写至指定的流。

`dprintf()`写至指定的`fd`。避免需调用`fdopen()`将`fd`转换成`FILE`结构。

`sprintf()`将格式化的字符写入缓冲区`buf`。`sprintf()`会在数组中字符结尾加上终止符`null`，但该字符不包括在返回值中。

`snprintf()`与`sprintf()`相同，但为避免缓冲区`buf`溢出显示添加缓冲区的长度`n`(包含null)，其返回值若小于`n`说明未被截断，若大于`n`则被截断。

`format`参数控制其余参数如何编写，如何显示。每个参数按照`format`编写，`format`以百分号`%`开始，`format`中除格式外的其他字符串不经任何修改被复制输出，`format`模版为：`%[flags][fldwidth][precision][lenmodifier]convtype`。

- `[flags]`可取值：

  - `‘`(撇号)：将整数为按千位分组字符
  - `-`：在字段内左对齐输出
  - `+`：总是显示带符号转换的正负号
  - ` `(空格)：若第一个字符不是正负号，则在其前面加上一个空格
  - `#`：指定另一种转换形式(例如，对于十六进制格式，加0x前缀)
  - `0`：添加前导0(而非空格)进行填充

- `[fldwidth]`说明最小字段宽度。一个非负十进制数，或是一个星号(*)组成。

  - 转换后参数字符数若小于宽度，则多余字符位置默认空格填充

- `[precision]`说明有效位数。由一个点(.)跟随一个可选的非负十进制数或一个星号(*)组成。

  - 整型转换后最少输出数字位数
  - 浮点型转换后小数点后的最少位数
  - 字符串转换后最大字节数

- `[lenmodifier]`说明参数长度。

  - `hh`：将相应的参数按`signed`或`unsigned` `char`类型输出
  - `h`：将相应的参数按`signed`或`unsigned` `short`类型输出
  - `l`：将相应的参数按`signed`或`unsigned` `long`类型输出
  - `ll`：将相应的参数按`signed`或`unsigned` `longlong`类型输出
  - `j`：`intmax_t`或`uintmax_t`
  - `z`：`size_t`
  - `t`：`ptrdiff_t`
  - `L`：`long double`

- `[convtype]`不是可选的，它控制如何解释参数。

  - `d`、`i`：   有符号十进制
  - `o`：          无符号八进制
  - `u`：          无符号十进制

  - `x`、`X`：   无符号十六进制
  - `f`、`F`：   双精度浮点数
  - `e`、`E`：   指数格式双精度浮点数(e^n)
  - `g`、`G`：   根据转换后的值解释为`f`、`F`、`e`或`E`
  - `a`、`A`：   十六进制指数格式双精度浮点数
  - `c`：          字符(若带长度修饰符`l`，为宽字符)
  - `s`：          字符串(若带长度修饰符`l`，为宽字符)
  - `p`：          指向`void`的指针
  - `n`：          到目前为止，此`printf`调用输出的字符的数目将被写入到指针所指向的带符号整型中
  - `%`：         一个%字符(若想输出`%`， 则用`%%`)
  - `C`：          宽字符(XSI扩展，等小于`lc`)
  - `S`：          宽字符串(XSI扩展，等效于`ls`)

> 有两种格式转化的语法，且不能混用：
>
> 1. 转换时按照出现在`format`参数之后的顺序应用于`format`中的。
> 2. 在`format`参数中显示的使用`%n$`序列来标识第n个参数。参数从1开始计数。

下列5种`printf`族的变体类似上面的5种，但是可变参数表(`...`)替换成了`va_list`类型的`arg`。

```c
#include<stdarg.h>
#include<stdio.h>
int vprintf(const char *restrict format, va_list arg);
int vfprintf(FILE *restrict fp, const char *restrict format, va_list arg);
int vdprintf(int fd, cosnt char *restrict format, va_list arg);
//三个函数的返回值：若成功，返回输出字符数；若出错，返回负值
int vsprintf(char *restrict buf, const char *restrict format, va_list arg);
//函数的返回值：若成功，返回存入数组的字符数，若编码出错，返回符值
int vsnprintf(char *restrict bug, size_t n, const char *restrict format, va_list arg);
//函数的返回值：若缓冲区足够大，返回将要存入数组的字符数；若编码出错，返回负值
```

#### 5.11.2格式化输入

下列3个`scanf`函数可用于处理格式化输入。

```c
#include<stdio.h>
int scanf(const char *restrict format, ...);
int fscanf(FILE *restrict fp, const char *restrict format, ...);
int sscanf(const char *restrict buf, const char *restrict format, ...);
//三个函数的返回值：赋值的输入项数；若输入出错或在任一转换前已到达文件尾端，返回EOF
```

`format`参数控制如何转换参数，以便对它们赋值。`format`以百分号`%`开始，`format`中除格式和空白字符外，格式字符串中的其他字符必须与输入匹配，若有一个字符不匹配，则停止后续处理，不再读输入的其余部分。

`format`模版为：`%[*][fldwidth][m][lenmodifier]convtype`。

- `[*]`用于抑制转换。按照转换说明的其余部分对输入进行转换，但转换结果并不存放在参数中

- `[fldwidth]`说明最大宽度(即最大字符数)。取值同`printf`
- `[lenmodifier]`说明要转换结果赋值的参数大小。取值同`printf`
- `[convtype]`字段类似`printf`的，但输入中的带符号数据给赋值给无符号类型。
  - `d`：      有符号十进制，基数为10
  - `i`：       由符号十进制，基数由输入格式决定
  - `O`：     无符号八进制(输入可选地有符号)
  - `u`：      无符号十进制，基数为10(输入可选地有符号)
  - `x`、`X`：无符号十六进制(输入可选地有符号)
  - `a`、`A`、`e`、`E`、`f`、`F`、`g`、`G`：浮点数
  - `c`：      字符
  - `s`：      字符串
  - `[`：       匹配列出的字符序列，以`]`终止
  - `[^`：     匹配除列出的字符序列，以`]`终止
  - `p`：      指向`void`指针
  - `n`：      将到目前为止该函数调用读取的字符数写入到指针向的无符号整型中
  - `%`：     一个%符号(%%)
  - `C`：      宽字符(XSI扩展，等小于`lc`)
  - `S`：     宽字符串(XSI扩展，等效于`ls`)
- `[m]`是赋值分配符。可用于`%c`、`%s`以及`%[`转换符，迫使内存缓冲区分配空间以接纳转换字符串。相关的参数必须是指针地址(`**ptr`)，分配的缓冲区地址必须复制给该指针。若调用成功，该缓冲区不再使用时，由调用者负责`free()`该缓冲区。

下列3种`scanf`族的变体类似上面的3种，但是可变参数表(`...`)替换成了`va_list`类型的`arg`。

```c
#include<stdarg.h>
#include<stdio.h>
int vscanf(const char *restrict format, va_list arg);
int vfscanf(FILE *restrict fp, const char *restrict format, va_list arg);
int vsscanf(const char *restrict buf, const char *restrict format, va_list arg);
//三个函数的返回值：赋值的输入项数；若输入出错或在任一转换前已到达文件尾端，返回EOF
```

### 5.12实现细节

调用`fileno()`可以获取与标准I/O流相关联的`fd`

```c
#include<stdio.h>
int fileno(FILE *fp);
//函数的返回值：与该流相关联的文件描述符
```

### 5.13临时文件

ISO C标准I/O库提供了两个函数创建临时文件。

```c
#include<stdio.h>
char *tmpnam(char *ptr);
//函数的返回值：指向唯一路径名的指针
FILE* tmpfile(void);
//函数的返回值：若成功，返回文件指针；若出错，返回NULL
```

`tmpnam()`产生一个与现有文件名不同的有效路径名字符串。每次调用它时，都产生一个不同的路径名，最多调用次数是`TMP_MAX`。`TMP_MAX`定义在`<stdio.h>`中。参数`ptr`有两种解释：

- 若`ptr`是`NULL`，则所产生的路径名存放在一个静态区中，指向该静态区的指针作为函数的返回值。后续调用`tmpnam()`时，会重写该静态区。(说明需要保存该路径名字符串的副本)
- 若`ptr`不是`NULL`，则认为它应该是指向长度至少`L_tmpnam`个字符的数组。所产生的路径名存放在该数组中，`ptr`也作为函数的返回值。

`tmpfile()`创建一个临时二进制文件(类型`wb+`)，在关闭该文件或程序结束时将自动删除这种文件。

> `tmpfile()`原理是先调用`tmpnam()`创建一个唯一路径，然后在该路径名下创建一个文件，并立即`unlink()`它。

SUS为处理临时文件定义了定外两个函数：

```c
#include<stdlib.h>
char *mkdtemp(char *template);
//函数的返回值：若成功，返回指向目录名的指针；若出错，返回NULL
int mkstemp(char *template);
//函数的返回值：若成功，返回文件描述符；若出错，返回-1
```

`mkdtemp()`创建了一个名称唯一的目录；`mkstemp()`创建了一个名称唯一的文件。

名称使用通过`template`字符串进行选择的。这个字符串的后6位设置为`XXXXXX`的路径名。

### 5.14内存流

内存流虽仍使用`FILE`指针进行访问，但并没有底层文件。所有的I/O都在缓冲区与主存之间来回传送字节。

有三个函数用于创建内存流，第一个是`fmemopen()`。

```c
#include<stdio.h>
FILE *fmemopen(void *restrict buf, size_t size, const char *restrict type);
//函数的返回值：若成功，返回流指针；若出错，返回NULL
```

参数`buf`有两个解释：

- 当`buf`为`NULL`，`fmemopen()`会分配参数`size`字节的缓冲区，当流关闭时缓冲区会被释放。
- 当`buf`不为`NULL`时，调用者提供的缓冲区用于内存流，`size`参数指定了缓冲区大小的字节数。

参数`type`控制如何使用流，取值可基于文件的标准I/O流的`type`参数取值。但有细微差别：

- 无论何时以追加写方式打开内存流，当前文件位置设为缓冲区中的第一个`null`字节。若缓冲区中不存在`null`字节，则当前位置就设为缓冲区中的第一个`null`字节。
- 当流并不是以追加写方式打开时，当前位置就设为缓冲区的开始位置。
- 若参数`buf`为`NULL`，则打开流进行只读或只写都没有任何意义。

- 任何时候需要增加缓冲区中的数据量以及调用`fclose()`、`fflush()`、`fseek()`、`fseeko()`以及`fsetpos()`时都会在当前位置写入一个`null`字节。

其他两个函数分别是`open_memstream()`和`open_wmemstream()`。

```c
#include<stdio.h>
FILE *open_memstream(char **bufp, size_t *sizep);
#include<wchar.h>
FILE *open_wmenstream(wchar_t **bufp, size_t *sizep);
//两个函数的返回值：若成功，返回流指针；若出错，返回NULL
```

`open_memstream()`创建的流是面向单字节的，`open_wmemstream()`创建的流是面向宽字节的。这两个函数与`fmemopen()`的区别在于：

- 创建的流只能写打开
- 不能指定自己的缓冲区，但可以分别通过`bufp`和`sizep`参数访问缓冲区地址和大小
- 关闭流后需要自行释放缓冲区
- 对流添加字节会增加缓冲区大小

在缓冲区地址和大小的使用上同时必须遵守一些原则：

- 缓冲区地址和长度只有在调用`fclose()`和`fflush()`后才有效
- 这些值只有在下一次流写入或调用`fclose()`前才有效。因为缓冲区动态增长，可能需要重写分配。

> 因为避免了缓冲区溢出，内存流非常适用于创建字符串。因为内存流只访问主存，不访问磁盘上的文件，所以对于把标准I/O流作为参数用于临时文件的函数来说，会有很大的性能提升。(存储在内存的临时文件)
