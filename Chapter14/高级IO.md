## 14高级I/O

### 14.2非阻塞I/O

系统调用分成两类：“低速”系统调用和其他。低速系统调用可能会使进程永远阻塞

> 虽然读写磁盘文件会暂时阻塞调用者，但并不能将与磁盘I/O有关的系统调用视为“低速”。 
>

对于非阻塞I/O，`open()`、`read()`和`write()`，这些操作不会永远阻塞。若操作不能完成，则立即出错返回，表示如继续执行将阻塞。

对于一个给定的`fd`，有两种方法将其指定为非阻塞I/O。

- 若通过调用`open()`获取`fd`，则可指定`O_NONBLOCK`标志。
- 对于已经打开的一个`fd`，可调用`fcntl()`打开`O_NONBLOCK` 标志。

### 14.3记录锁

当第一个进程正在读或修改文件的某个部分时，使用记录锁可以阻止其他进程修改同一文件区。

> 对于unix系统而言，内核没有使用文件记录这种概念。更适合的术语是字节范围锁 (byte-range locking)，因为它锁定的只是文件中的一个区域(也可能是整个文件)。

#### 14.3.2fcntl记录锁

调用`fcntl()`将设置记录锁。

```c
#include <fcnt1.h>
int fcnt1(int fd, int cmd, .../* struct flock flockptr */);
//函数的返回值：若成功， 依赖于cmd(见下)；否则，返回−1
```

第三个参数是一个指向`flock`结构的指针。

```c
struct flock {
	short l_type;    			//所希望锁的类型：共享读锁、独占写锁或解锁一个人区域(F_RDLCK, F_WRLCK, or F_UNLCK) 
	short l_whence;   		//要加锁或解锁区域的起始位/* SEEK_SET, SEEK_CUR, or SEEK_END*/
	off_t l_start;        //要加锁或解锁区域的的字节偏移量(offset in bytes, relative to l_whence) 
	off_t l_len;     			//区域的字节长度(length, in bytes; 0 means lock to EOF)
	pid_t l_pid;     			//持有锁并能阻塞当前进程的进程id(returned with F_GETLK)
};
```

关于加锁或解锁区域的说明还要注意下列几项规则。 

- 锁可以在当前文件尾端处开始或者越过尾端处开始，但不能在文件起始位置之前开始。

- 若`l_len`为0，则表示锁的范围可以扩展到最大可能偏移量。这意味着不管向该文件中追加写了多少数据，它们都可以处于锁的范围内(不必猜测会有多少字节被追加写到了文件之后)，而且起始位置可以是文件中的任意一个位置。

- 为了对整个文件加锁，可设置`l_start`和`l_whence`指向文件的起始位置，并设`l_len`为0。

参数`cmd`可取值：

- `F_GETLK`，判断由`flockptr`所描述的锁是否会被另外一把锁所排斥 (阻塞)。若存在一把锁阻止创建由`flockptr`所描述的锁，则该锁的信息将重写`flockptr`。若不存在，则除了将`l_type`设为`F_UNLCK`外，其他信息保持不变。
- `F_SETLK`，设置由`flockptr`所描述的锁。如果我们试图获得一把锁，而兼容性规则阻止系统分配这把锁，则立即出错返回，`errno`设置为`EACCES`或`EAGAIN`。也可清除由`flockptr`指定的锁(`l_type`为`F_UNLCK`)。
- `F_SETLKW`，这个命令是`F_SETLK`的阻塞版本(W表示等待(wait))。若所请求的锁因另一个进程当前已经对所请求区域的某部分进行了加锁而不能被授予，那么调用进程会被置为休眠。如果请求创建的锁已经可用，或者休眠由信号中断，则该进程被唤醒。

用`F_GETLK`测试能否建立一把锁，然后用`F_SETLK`或 `F_SETLKW`试图建立该锁，这两步不是一个原子操作。因此不能保证在这两次`fcntl()`调用之间不会有另一个进程插入并建立一把相同的锁。若不希望阻塞等待锁变为可用，就必须处理`F_SETLK`的返回出错。

有两种类型的锁：共享读锁(`L_RDLCK`)和独占性写锁(`L_WRLCK`)。基本规则为：

- 对于多个进程而言，读写互斥，独读共享，独写互斥：
  - 任意多个进程在一个给定的字节上可以有一把共享的读锁，但是在一个给定字节上只能有一个进程有一把独占写锁。
  - 若在一个给定字节上已经有一把或多把读锁，则不能在该字节上再加写锁。
  - 若在一个字节上已经有一把独占性写锁，则不能再对它加任何读锁。
- 对于单个进程而言，同一区间，读写互换，仅有一把：
  - 若进程对一个文件区间已经有 了一把锁，后来该进程又试图在同一文件区间再加一把锁，那么新锁将替换已有锁。

> 由于`F_GETLK()`返回信息指示是否有现有的锁阻止调用进程设置它自己的锁。而对于同一进程总是替换现有的锁(若已存在)，所以调用进程决不会阻塞在自己持有的锁上，因此`F_GETLK()`命令决不会报告调用进程自己持有的锁。

- 加读锁时，该描述符必须是读打开。加写锁时，该描述符必须是写打开。

> 在设置或释放文件上的一把锁时，系统按要求组合(相邻的加锁区合并成一个区)或分裂相邻区(内核将维持两把锁)。

#### 14.3.3锁的隐含继承和释放

关于记录锁的自动继承和释放有3条规则。

- 锁与进程和文件两者相关联。表明当一个进程终止时，它所建立的锁全部释放；该进程通过`fd`引用的文件上的任何一把锁，在`fd`关闭后都会释放(这些锁都是该进程设置的)。
- 由`fork()`产生的子进程不继承父进程所设置的锁。对于从父进程处继承过来的`fd`，子进程需要调用`fcntl()`才能获得它自己的锁。
- 在执行`exec`后，新进程可以继承原执行进程的锁。但若对`fd`符设置了执行时关闭标志，则将释放相应文件的所拥有的锁。

#### 14.3.4FreeBSD实现

有了记录锁后，文件系统将增加`lockf`结构，它们由i-node结构开始相互链接起来。每个`lockf`结构描述了一个给定进程的一 个加锁区域(由偏移量和长度定义的)。

内核会从该描述符所关联的i节点开始，逐个检查`lockf`链接表中的各项，并释放由调用进程持有的各把锁。内核并不清楚(也不关心)进程是用哪个具体的`fd`来设置这把锁的。

#### 14.3.5在文件尾端加锁

#### 14.3.6建议性锁和强制性锁

### 14.4I/O多路转接

单一控制流简单的对存在多个低速系统调用的处理可能会造成进程永久堵塞，因此采用下面方法解决多路I/O问题：

- 多进程，每个进程都执行阻塞read。进程之间的同步需要信号机制，增加了复杂度。
- 多线程，每个线程都执行阻塞read。线程之间的同步用到了锁机制，增加了复杂度。
- 轮询，使用非阻塞I/O读取数据。会浪费CPU时间。
- 异步I/O(asynchronous I/O)，进程告诉内核:当描述符准备好可以进行I/O时，用一个信号通知它。但能用的信号数量仍远小于潜在的打开fd的数量。因此还需要轮询检查。
- I/O多路转接(I/O multiplexing)，先构造一张相关`fd`的列表，然后调用一个函数(poll、pselect、select以及epoll)，阻塞等等待列表中有已准备好进行I/O的fd时，该函数才返回，进程会被告知哪些fd已准备好可以进行I/O。

#### 14.4.1函数select和pselect

传给`select()`的参数告诉内核:

- 所关心的`fd`;
- 对于每个`fd`我们所关心的条件(读条件、写条件或异常条件)
- 愿意等待多长时间

从`select()`返回时，内核告诉我们:

- 已准备好的`fd`的总数量;
- 对于读、写或异常这3个条件中的每一个，哪些`fd`已准备好。

使用这种返回信息，就可调用相应的I/O操作(一般是read 或write)，并且使该操作不会阻塞。

```c
#include <sys/select.h>
int select(int maxfdp1, fd_set *restrict readfds,
					 fd_set restrict writefds, fd_set restrict exceptfds,
					 struct timeval *restrict tvptr);
//函数的返回值：准备就绪的描述符数目；若超时，返回0；若出错，返回−1
```

参数`tvptr`指定愿意等待的时间，有以下3种情况。

- `tvptr == NULL`，永久等待直到有相关`fd`准备好。如果捕捉到一个信号则中断此等待，函数返回-1，`errno`设置为`EINTR`。
- `tvptr->tv_sec == 0 && tvptr->tv_usec == 0`，测试所有相关`fd`并立即返回。这是轮询检测多个`fd`状态而不阻塞的方法。
- `tvptr->tv_sec != 0 || tvptr->tv_usec != 0`，指定的时间内等待相关`fd`准备好，期间可被捕捉信号中断。超过时立即返回0。

参数`readfds`、`writefds`和`exceptfds`是指向`fd`集的指针，分别为关心的可读、可写或处于异常条件。`fd`集的数据类型为`fd_set`。

在声明了一个`fd`集后，必须先调用`FD_ZERO`将`fd`集置为0，然后添加关心的`fd`。

```c
#include <sys/select.h>
int FD_ISSET(int fd, fd_set *fdset);
//函数返回值：若fd在描述符集中，返回非0值；否则，返回0
void FD_CLR(int fd, fd_set *fdset);
void FD_SET(int fd, fd_set *fdset);
void FD_ZERO(fd_set *fdset);
```

> 这些接口可实现为宏或函数。

调用`FD_ZERO`将一个`fd`集中的所有位设置为0。

调用`FD_SET`可以要开启`fd`集中的一位。

调用 `FD_CLR`可以清除`fd`集中的一位。

调用`FD_ISSET`测试`fd`集中的一个指定位是否已打开。

`select()`的三个`fd`集指针中的任意一个设为`NULL`，这表示对相应条件并不关心。

> 如果所有3个指针都是`NULL`，则`select()`提供了比`sleep()`更精确的定时器。

参数`maxfdp1`是要检查的`fd`总数，需要在3个`fd`集中找出最大`fd`编号值，因为`fd`编号从0开始，所以加1再赋值给`maxfdp1`。

> 可将`maxfdp1`设为`FD_SETSIZE`，这是`<sys/select.h>`中的一个常量，它指定最大描述符数 (经常是1024)。

`select()`返回的 3个`fd`集中仍旧打开的位对应于已准备好的`fd`。

> 若同一`fd`已准备好读和写，那么在返回总数中会对其计两次数。

对于“准备好”的含义更具体的说明：

- 若对读集(readfds)中的一个`fd`进行的read操作不会阻塞。

- 若对写集(writefds)中的一个`fd`进行的write操作不会阻塞。

- 若对异常条件集(exceptfds)中的一个`fd`有一个未决异常条件。

> 异常条件包括：在网络连接上到达带外的数据，或者在处于数据包模式的伪终端上发生了某些条件。

- 对于读、写和异常条件，普通文件的`fd`总是返回准备好。

> 一个描述符阻塞与否并不影响`select()`是否阻塞。
>
> - 若希望读一个非阻塞`fd`，并且以超时值为5秒调用`select()`，则最多阻塞5s。
> - 若指定永久等待， 则在该`fd`数据准备好，或捕捉到一个信号之前，则会一直阻塞。

若在一个`fd`上碰到了文件尾端，则`select()`会认为该描述符是可读的。调用`read()`返回0指示到达文件尾端。

> 很多人错误地认为，当到达文件尾端时， `select()`会指示一个异常条件。

POSIX.1也定义了一个`select()`的变体，称为`pselect()`。

```c
#include <sys/select.h>
int pselect(int maxfdp1, fd_set *restrict readfds,
						fd_set restrict writefds, fd_set restrict exceptfds, 
            const struct timespec *restrict tsptr,
						const sigset_t *restrict sigmask);
//函数的返回值：准备就绪的描述符数目；若超时，返回0；若出错，返回−1
```

除下列几点外，`pselect()`与`select()`相同。 

- `select()`的超时值用timeval结构指定，但`pselect()`使用`timespec`结构

- `pselect()`的超时值被声明为`const`，这保证了调用`pselect()`不会改变此值。

- `pselect()` 可使用可选信号屏蔽字。
  - 若`sigmask`为`NULL`，那么在与信号有关的方面，`pselect`的运行状况和`select()`相同。
  - 否则，在调用`pselect()`时，以原子操作的方式安装该信号屏蔽字。在返回时，恢复以前的信号屏蔽字。

#### 14.4.2函数poll

```c
#include <poll.h>
int poll(struct pollfd fdarray[], nfds_t nfds, int timeout);
//函数的返回值：准备就绪的描述符数目；若超时，返回0;若出错，返回-1
```

`poll()`构造一个`pollfd()`结构的数组，每个数组元素指定一个`fd`编号以及我们对该`fd`关注的事件。

```c
struct pollfd {
	int fd; /* file descriptor to check, or < 0 to ignore */ 
  short events; /* events of interest on fd */
	short revents; /* events that occurred on fd */
};
```

参数`nfds`用来指定`fdarray`数组中的条目数。

应将每个数组元素的`events`成员设置为下列一个或几个，告诉内核对于相关`fd`我们关心的哪些事件。

- 可读事件：
  - `POLLIN`，可不阻塞地读高优先级数据以外的数据，等效于`POLLRDNORM｜POLLRDBAND`
  - `POLLRDNORM`，可不阻塞地读普通数据
  - `POLLRDAND`，可不阻塞地读优先级数据
  - `POLLPRI`，可不阻塞地读高优先级数据
- 可写事件：
  - `POLLOUT`，可不阻塞地写普通数据
  - `POLLWRNORM`，与`POLLOUT`相同
  - `POLLWRBAND`，可不阻塞地写优先级数据

> 有些事件的名字中包含BAND，它指的是STREAMS当中的优先级波段。

`poll()`返回时，`revents`成员由内核设置，除了可设置可读事件和可写事件，还可设置异常事件。用于说明每个`fd`发生了哪些事件。 

- 异常事件：
  - `POLLERR`，已出错
  - `POLLHUP`，已挂断
  - `POLLNVAL`，`fd`没有引用一个打开文件

> `poll()`没有更改`events`成员。这与`select()`不同，`select()`修改其参数以指示哪一个描述符已准备好了。

当一个`fd`被挂断(`POLLHUP`)后，就不能再写该`fd`， 但任有可能可以从该`fd`读取到数据。

参数`timeout`指定的是我们愿意等待多长时间，有3种不同的情形。

- `timeout == -1`，永久等待直到有相关`fd`准备好。如果捕捉到一个信号则中断此等待，函数返回-1，`errno`设置为`EINTR`。

- `timeout == 0`，测试所有相关`fd`并立即返回。这是轮询检测多个`fd`状态而不阻塞的方法。

- `timeout > 0`，指定的时间内等待相关`fd`准备好，期间可被捕捉信号中断。超过时立即返回0。

> 与`select()`一样，一个`fd`是否阻塞不会影响`poll()`是否阻塞。

文件尾端与挂断事件之间的区别。

- 若正从终端输入数据，并键入相关`fd`，那么就会打开`POLLIN`，于是就可以读文件结束指示(`read()`返回0)。`revents`中的`POLLHUP`没有打开。
- 若正在读调制解调器，并且电话线已挂断，将收到`POLLHUP`事件。

### 14.5异步I/O

#### 14.5.1System V异步I/O



#### 14.5.2BSD异步I/O



#### 14.5.3POSIX异步I/O

异步I/O接口使用AIO控制块来描述I/O操作。AIO控制块的数据类型为`aiocb`。该结构至少包括下面这些字段：

```c
struct aiocb {
	int 						aio_fildes; 							//被打开用来读或写的fd(file descriptor)
  off_t 					aio_offset; 							//I/O操作的文件偏移量(file offset for I/O)
  volatile void  *aio_buf;									//I/O操作的缓冲区(buffer for I/O)
	size_t 					aio_nbytes;								//I/O操作的字节数(number of bytes to transfer)
	int   					aio_reqprio;							//优先级(priority)
	struct sigevent aio_sigevent;							//信号相关(signal information) 
  int 						aio_lio_opcode; 					//I/O操作列表时用到的I/O动作(operation for list I/O) 
};
```

异步I/O操作必须显式地指定偏移量，但并不会影响由操作系统维护的文件偏移量。若使用异步I/O接口向一个以追加模式打开的文件中写入数据，AIO控制块中的`aio_offset`字段会被系统忽略。

在I/O事件完成后，通过字段`sigevent`结构来描述如何通知应用程序。

```c
struct sigevent {
	int 						sigev_notify; 										//通知类型(notify type)
	int 						sigev_signo; 											//信号编号(signal number)
	union 					sigval sigev_value; 							//通知参数(notify argument)
	void (*sigev_notify_function)(union sigval); 			//通知函数(notify function)
	pthread_attr_t 	*sigev_notify_attributes; 				//通知函数的属性(notify attrs)
};
```

`sigev_notify`取值可能是以下3个中的一个。 

- `SIGEV_NONE`，异步I/O请求完成后，不通知进程。 

- `SIGEV_SIGNAL`，异步I/O请求完成后，产生由`sigev_signo`字段指定的信号。如果应进程已选择捕捉信号，且在建立信号处理程序的时候指定了`SA_SIGINFO`标志，那么该信号将被入队(若支持排队信号)。同时会传送给信号处理程序一个`siginfo`结构，该结构的`si_value`字段被设为`sigev_value` (若使用了`SA_SIGINFO`标志)。

- `SIGEV_THREAD`，当异步I/O请求完成时，由`sigev_notify_function`字段指定的函数被调用。`sigev_value`字段被作为它的唯一参数。除非`sigev_notify_attributes`字段设定了线程属性，否则该函数将在分离状态下的一个单独的线程中执行。

初始化异步I/O的AIO控制块后，调用`aio_read()`来进行异步读操作，`aio_write()`来进行异步写操作。

```c
#include <aio.h>
int aio_read(struct aiocb *aiocb); 
int aio_write(struct aiocb *aiocb);
//两个函数的返回值：若成功，返回0；若出错，返回−1
```

当这些函数成功返回时，异步I/O请求便已经被放入内核区的等待处理的队列中。

> 这些返回值与实际I/O操作的结果无任何关系。
>
> 在等待 I/O操作时，必须注意确保AIO控制块和缓冲区的内存始终合法，除非I/O操作完成，否则相关内存不能被复用。

要想强制所有等待中的异步操作不等待而写入持久化的存储中，可以设置一个 AIO控制块并调用`aio_fsync()`。

```c
#include <aio.h>
int aio_fsync(int op, struct aiocb *aiocb);
//函数的返回值：若成功，返回0；若出错，返回−1
```

AIO控制块中的`aio_fildes`字段指定异步写操作被同步的文件。 

参数`op`指定同步机制：

- 设为`O_DSYNC`，那么操作等同于 `fdatasync()`。
- 设为`O_SYNC`，那么操作等同于`fsync()`。

在设定完同步后，`aio_fsync()`成功返回。在异步同步操作完成之前，数据不会被持久化。

调用`aio_error()`可以获取一个异步读、写或者同步操作的完成状态。

```c
#include <aio.h>
int aio_error(const struct aiocb *aiocb);
//函数的返回值：(见下)
```

返回值为下面4种情况中的一种。

- 0，异步操作成功完成。需要调用`aio_return()`获取操作返回值。 
- -1，对`aio_error()`的调用失败并设置`errno`。

- `EINPROGRESS`，异步读、写或同步操作仍在等待。
- 其他情况，其他任何返回值是相关的异步操作失败返回的错误码。 

若异步操作成功，调用`aio_return()`可以获取异步操作的返回值。

```c
#include <aio.h>
ssize_t aio_return(const struct aiocb *aiocb);
//函数的返回值：(见下)
```

直到异步操作完成之前，调用`aio_return()`结果是未定义的。对每个异步操作应只调用一次`aio_return()`，一旦调用，内核就可以释放掉包含了I/O操作返回值的记录。 

调用`aio_return()`的返回值：

- -1，对`aio_return()`的调用失败并设置`errno`。
- 其他情况下，它将返回异步操作的结果，即会返回`read()`、`write()`或者`fsync()`在被成功调用时可能返回的结果。

若在完成了所有事务时，还有异步操作未完成，则可以调用`aio_suspend()`来阻塞进程，直到异步操作完成。

```c
#include <aio.h>
int aio_suspend(const struct aiocb *const list[], int nent, const struct timespec *timeout);
//函数的返回值：若成功，返回0；若出错，返回−1 
```

调用`aio_suspend()`可能会返回3种情况中的一种。

- 若函数阻塞被一个信号中断，它将会返回-1，并将`errno`设置为`EINTR`。
- 若在没有任何I/O操作完成的情况下，阻塞超时将返回-1，并将`errno`设置为`EAGAIN`。参数`timeout`设为`NULL`时函数永久阻塞。
- 若有任何I/O操作完成，直接返回0。

参数`list`是一个指向AIO控制块数组的指针，参数`nent`为该数组中的条目数。数组中的空指针会被跳过，其他条目都必须指向已用于初始化异步I/O操作的AIO控制块。

调用`aio_cancel()`可以尝试取消等待队列中的异步I/O操作。

```c
#include <aio.h>
int aio_cancel(int fd, struct aiocb *aiocb);
//函数的返回值：(见下) 
```

参数`fd`指定了等待队列中的异步I/O操作关联`fd`。

参数`aiocb`将影响函数行为：

- 若`aiocb`参数为`NULL`，则系统将会尝试取消所有该文件上未完成的异步I/O操作。

- 若`aiocb`非空，则系统将尝试取消由AIO控制块描述的单个异步I/O操作。

> 之所以说系统“尝试”取消操作，是因为无法保证系统能够取消正在进程中的任何操作。 

调用`aio_cancel()`可能会返回以下4个值中的一个。

- `AIO_ALLDONE`，所有操作在尝试取消它们之前已经完成。 
- `AIO_CANCELED`，所有要求的操作已被取消。 
- `AIO_NOTCANCELED`，至少有一个要求的操作没有被取消。 
- -1，对`aio_cancel()`的调用失败，并设置`errno`。 

若异步I/O操作被成功取消，对相应的AIO控制块调用`aio_error()`将会返回错误`ECANCELED`。若操作不能被取消，那么相应的AIO控制块不会因为`aio_cancel()`而被修改。

调用`lio_listio()`将提交一系列由AIO控制块列表描述的I/O操作请求。它既能以同步的方式来使用，又能以异步的方式来使用。

```c
#include <aio.h>
int lio_listio(int mode, struct aiocb *restrict const list[restrict], 
               int nent, struct sigevent *restrict sigev);
//函数的返回值：若成功，返回0；若出错，返回−1 
```

参数`mode`决定了I/O操作是否为异步：

- 设为`LIO_WAIT`时，函数将在所有由列表指定的I/O操作完成后返回。参数`sigev`将被忽略。
- 设为`LIO_NOWAIT`时，函数将在I/O请求入队后立即返回。参数`sigev`指定的异步通知是每个AIO控制块内的异步通知之外另加的，并且只会在所有的I/O操作完成后发送。 若不想被通知，可以把`sigev`设定为`NULL`。

参数`list`指向AIO控制块列表，该列表指定了要运行的I/O操作的。 参数`nent`指定了数组中的元素个数。AIO控制块列表可以包含`NULL`指针，这些条目将被忽略。 

在每一个AIO控制块中，`aio_lio_opcode`字段指定了对fd的相关操作

- 设为`LIO_READ`时，读操作会按照对应的AIO控制块被传给`aio_read()`来处理。
- 设为`LIO_WRITE`时，写操作会按照对应的AIO控制块被传给`aio_write()`来处理。
- 设为`LIO_NOP`时。被忽略的空操作。

具体实现会限制未完成的异步I/O操作的数量：

- 调用`sysconf(_SC_IO_LISTIO_MAX)` 来设定`AIO_LISTIO_MAX`的值(单个列表I/O调用中的最大I/O操作数)。


- 调用`sysconf(_SC_AIO_MAX)` 来设定`AIO_MAX`的值(未完成的异步I/O操作的最大数目)。

- 调用`sysconf(_SC_AIO_PRIO_DELTA_MAX)` 来设定`AIO_PRIO_DELTA_MAX`的值(进程可以减少的其异步I/O优先级的最大值)。

### 14.6函数readv和writev

调用`readv()`和`writev()`用于一次读、写多个非连续缓冲区。也称为散布读(scatter read)和聚集写(gather write)。

```c
#include <sys/uio.h>
ssize_t readv(int fd, const struct iovec *iov, int iovcnt); 
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
//两个函数的返回值：已读或已写的字节数；若出错，返回−1 
```

参数`iov`是一个指向`iovec`结构数组的指针:

```c
 struct iovec {
	void *iov_base; 						//缓冲区起始地址(starting address of buffer)
  size_t iov_len; 						//缓冲区大小(size of buffer)
};
```

参数`iovcnt`指定`iov`数组中的条目数，其最大值受限于`IOV_MAX`。

`writev()`从缓冲区中聚集输出数据的顺序是：`iov[0]`、`iov[1]`直至`iov[iovcnt-1]`。返回输出的字节总数，通常应等于所有缓冲区长度之和。

`readv()`则将读入的数据按上述同样顺序散布到缓冲区中。但总是先填满一个缓冲区，再填写下一个。返回读到的字节总数。若遇到文件尾端，已无数据可读，则返回0。

### 14.7函数readn和writen

管道、FIFO以及某些设备(特别是终端和网络)有下列两种性质。

- 一次读操作所返回的数据可能少于所要求的数据，即使还没达到文件尾端也可能是这样。这不是一个错误，应当继续读该设备。

- 一次写操作的返回值也可能少于指定输出的字节数。这可能是由某个因素造成的，例如，内核输出缓冲区变满。这也不是错误，应当继续写余下的数据。

> 通常，只有非阻塞`fd`，或捕捉到 一个信号时，才发生这种write的中途返回。

在读、写磁盘文件时从未见到过这种情况，除非文件系统用完了空间，或者接近了配额限制，不能将要求写的数据全部写出。

调用`readn()`和`writen()`将分别读、写指定的N字节数据至缓冲区。

> 这两个函数只是按需多次调用read和write直至读、写了N字节数据。

```c
#include "apue.h"
ssize_t readn(int fd, void *buf, size_t nbytes); 
ssize_t writen(int fd, void *buf, size_t nbytes);
//两个函数的返回值：读、写的字节数；若出错，返回−1
```

在要将数据写到管道、FIFO及某些设备时，就可调用`writen()`，但是仅当事先知道要接收数据的数量时，才调用`readn()`。

函数返回值可能小于要求值的情况：

- 若在已经读、写了一些数据后出错，则这两个函数返回的是已传输的数据量，而非错误。

- 若读达到文件尾端，但尚未满足所要求的量， 则`readn()`返回已复制到调用者缓冲区中的字节数。

### 14.8内存映射I/O

内存映射I/O(memory-mapped I/O)能将一个磁盘文件映射到内存空间中的一个缓冲区上。

调用`mmap()`将通知内核将指定文件映射到一 个内存空间中。

```c
#include <sys/mman.h>
void *mmap(void *addr, size_t len, int prot, 
           int flag, int fd, off_t off);
//函数的返回值：若成功，返回映射区的起始地址；若出错，返回MAP_FAILED
```

参数`addr`用于指定映射存储区的起始地址。通常将其设置为`NULL`，表示由内核指定该映射区的起始地址。

参数`fd`是指定要被映射文件。在文件映射到地址空间前，必须先打开该文件。

参数`len`是映射的字节数，`off`是要映射字节在文件中的起始偏移量。

`off`的值和`addr`的值通常被要求是系统虚拟内层页长的整倍数。

> 虚拟内层页长可调用`sysconf(_SC_PAGESIZE)`或`sysconf(_SC_PAGE_SIZE)`得到。

若映射区的长度不是页长的整数倍时，系统通常会提供对齐页长的映射区，补齐部分被设为0。可以修改补齐内存区，但任何变动都不会在文件中反映出来，必须先加长该文件。

参数`prot`指定了映射存储区的保护要求，可按位或：

- `PROT_READ`，映射区可读
- `PROT_WRITE`，映射区可写
- `PROT_EXEC`，映射区可执行
- `PROT_NONE`，映射区不可访问

> 对指定映射存储区的保护要求不能超过文件open模式访问权限。

参数`flag`影响映射存储区的多种属性，必须指定`MAP_SHARED`或`MAP_PRIVATE`。

- `MAP_FIXED`，返回值必须等于`addr`。因为这不利于可移植性，所以不建议使用此标志。如果未指定此标志，而且`addr`非0，则内核只把`addr`视为在何处设置映射区的一种建议，但是不保证会使用所要求的地址。将`addr`指定为0可获得最大可移植性。

- `MAP_SHARED`，表示对映射区的存储操作会修改映射文件。

- `MAP_PRIVATE`，表示对映射区的存储操作会创建该映射文件的一个私有副本，对该映射区的引用都是引用该副本。

> `MAP_PRIVATE`一种用途是用于调试程序，它将程序文件的正文部分映射至存储区，允许用户修改其中的指令。任何修改只影响程序文件的副本，而不影响原文件。

与映射区相关的信号有`SIGSEGV`和`SIGBUS`。

- 信号`SIGSEGV`通常用于指示进程试图访问对它不可用的存储区。若映射存储区被`mmap()` 指定成了只读的，那么进程试图将数据存入这个映射存储区的时候， 也会产生此信号。

- 信号`SIGBUS`通常用于指示进程试图访问映射区已不存在的部分。例如，假设用文件长度映射了一个文件，但在引用该映射区之前，另一个进程已将该文件截断。此时，如果进程试图访问对应于该文件已截去部分的映射区，将会接收到`SIGBUS`信号。

子进程能通过`fork()`继承存储映射区，但是新程序则不能通过`exec`继承存储映射区。

调用`mprotect()`可以更改一个现有映射的权限。

```c
#include <sys/mman.h>
int mprotect(void *addr, size_t len, int prot);
//函数的返回值：若成功，返回0；若出错，返回-1
```

参数`prot`的取值与`mmap()`中一样。参数`addr`的值必须是系统页长的整数倍。

如果修改的页是通过`MAP_SHARED`标志映射到地址空间的，那么修改并不会立即写回到文件中。若只修改了一页中的一个字节，当修改被写回到文件中时，整个页都会被写回。

调用`msync()`将共享映射中脏页冲洗到被映射的文件中。`msync()`类似于`fsync()`，但作用于内存映射区。

```c
#include <sys/mman.h>
int msync(void *addr, size_t len, int flags);
// 函数的返回值：若成功，返回0；若出错，返回-1
```

参数`flags`使我们对控制如何冲洗存储区。一定要指定`MS_ASYNC`和 `MS_SYNC`中的一个。

- 设为`MS_ASYNC`时，简单地调试要写的页。
- 设为`MS_SYNC`时，在函数返回之前等待写操作完成
- `MS_INVALIDATE`是一个可选标志，允许我们通知操作系统丢弃那些与底层存储器没有同步的页。

调用`munmap()`可以解除映射区映射。

```c
#include <sys/mman.h>
int munmap(void *addr, size_t len);
//函数的返回值：若成功，返回0；若出错，返回−1 
```

当进程终止时，会自动解除存储映射区的映射，关闭映射存储区时使用的`fd`并不解除映射区。调用`munmap()`并不会更新映射区的内容写到磁盘文件上。

