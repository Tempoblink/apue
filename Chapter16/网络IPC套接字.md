## 16网络IPC：套接字

### 16.2套接字描述符

套接字是通信端点的抽象。正如使用文件描述符访问文件，进程用套接字描述符访问套接字。

套接字描述符在unix系统中被当作是一种`fd`。许多处理`fd`的函数(如read和write)可用于处理套接字描述符。

调用`socket()`创建一个套接字。

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
//函数的返回值：若成功，返回文件(套接字)描述符；若出错，返回−1
```

参数`domain`(域)确定通信方式。各个域都有表示地址的格式，各个域的常数都以`AF_`开头，指地址族(address family)。 

- `AF_INET`，IPv4因特网域
- `AF_INET6`，IPv6因特网域
- `AF_UNIX`，UNIX域
- `AF_UNSPEC`，未指定，以代表“任何”域。
- `AF_LOCAL`，`AF_UNIX`的别名

参数`type`确定套接字的类型，进一步确定通信特征。实现中可以自由增加其他类型的支 持。

- `SOCK_DGRAM`，固定长度的、无链接的、不可靠的报文传递
- `SOCK_RAW`，IP协议的数据报接口
- `SOCK_SEQPACKET`，固定长度的、有序的、可靠的、面向连接的报文传递
- `SOCK_STREAM`，有序的、可靠的、双向的、面向连接的字节流

参数`protocol`通常是 0，表示为给定的域和套接字类型选择默认协议。当对同一域和套接字类型支持多个协议时，可以使用`protocol`选择 一个特定协议。

- `IPPROTO_IP`，IPv4网际协议
- `IPPROTO_IPv6`，IPv6网际协议
- `IPPROTO_ICMP`，因特网控制报文协议(Internet Control Protocol)
- `IPPROTO_RAW`，原始IP数据包协议
- `IPPROTO_TCP`，传输控制协议(Transmission Control Protocol)
- `IPPROTO_UDP`，用户数据报协议(User Datagram Protocol)

在`AF_INET`通信域中，套接字类型`SOCK_STREAM`的默认协议是TCP，套接字类型`SOCK_DGRAM`的默认协议是UDP。

对于数据报(`SOCK_DGRAM`)接口，两个对等进程之间通信时不需要逻辑连接。只需要向对等进程所使用的套接字送出一个报文。因此数据报提供了一个无连接的服务。

对于字节流(`SOCK_STREAM`)要求在交换数据之前，在本地套接字和通信的对等进程的套接字之间建立一个逻辑连接。 

`SOCK_STREAM`套接字提供字节流服务，所以应用程序分辨不出报文的界限。这意味着从`SOCK_STREAM`套接字读数据时，它也许不会返回所有由发送进程所写的字节数。最终可以获得发送过来的所有数据，但也许要通过若干次函数调用才能得到。

`SOCK_SEQPACKET`套接字和`SOCK_STREAM`套接字很类似，只是从该套接字得到的是基于报文的服务而不是字节流服务。这意味着从`SOCK_SEQPACKET`套接字接收的数据量与对方所发送的一致。流控制传输协议(Stream Control Transmission Protocol，SCTP)提供了因特网域上的顺序数据包服务。

`SOCK_RAW`套接字提供一个数据报接口，用于直接访问下面的网络层。使用这个接口时，应用程序负责构造自己的协议头部，这是因为传输协议(如TCP和UDP)被绕过了。当创建一个原始套接字时，需要有root用户特权，这样可以防止恶意应用程序绕过内建安全机制来创建报文。

调用`socket()`与调用`open()`相类似，均可获得用于I/O的`fd`。当不再需要该`fd`时，调用`close()`来关闭对文件或套接字的访问，并且释放该`fd`以便重新使用。

虽然套接字描述符本质上是一个`fd`，但不是所有参数为`fd`的函数都可以接受套接字描述符。例如， `lseek()`不能以套接字描述符为参数，因为套接字不支持文件偏移量的概念。

套接字通信是双向的。调用`shutdown()`将禁止一个套接字的I/O。

```c
#include <sys/socket.h>
int shutdown (int sockfd, int how);
//函数的返回值：若成功，返回0；若出错，返回−1
```

参数`how`影响函数的行为：

- 若指定为`SHUT_RD`(关闭读端)，那么无法从套接字读取数据。

- 若指定为`SHUT_WR`(关闭写端)，那么无法使用套接字发送数据。

- 若指定为`SHUT_RDWR`，则既无法读取数据，又无法发送数据。 

对套接字调用`close()`和`shutdown()`的区别：

- 直到`close()`最后一个引用该套接字的`fd`，才会释放该套接字。而`shutdown()`允许使一 个套接字处于不活动状态，与引用它的`fd`数目无关。

-  `shutdown()`可以关闭套接字双向传输中的一个方向。

### 16.3寻址

目标通信进程标识由两部分组成。

- 网络地址，标识网络上通信目标的主机
- 端口号，标识目标的服务进程。

#### 16.3.1字节序

字节序是一个处理器架构特性，用于指示像整数这样的大数据类型内部 的字节如何排序。

- 若支持大端(big-endian)字节序，则最大字节地址出现在最低有效字节(Least Significant Byte，LSB)上。

- 若支持小端(little- endian)字节序，则最小字节地址出现在最低有效字节上。

> 不管字节如何排序，最高有效字节(Most Significant Byte，MSB)总是在左边，最低有效字节总是在右边。

例如，对于一个32位整数赋值`0x04030201`，然后用字符指针指向，其：

- 最高有效字节包含4，最低有效字节包含1。
- 对于小端字节序，cp[0]为01，cp[3]为04。
- 对于大端字节序，cp[0]为04，cp[3]为01。

对于TCP/IP，地址用网络字节序来表示。有4个函数用于处理器字节序和网络字节序的相互转换。

```c
#include <arpa/inet.h>
uint32_t htonl(uint32_t hostint32);
//函数的返回值：以网络字节序表示的32位整数
uint16_t htons(uint16_t hostint16);
//函数的返回值：以网络字节序表示的16位整数
uint32_t ntohl(uint32_t netint32);
//函数的返回值：以主机字节序表示的32位整数
uint16_t ntohs(uint16_t netint16);
//函数的返回值：以主机字节序表示的16位整数
```

> h表示“主机”字节序，n表示“网络”字节序。l表示“长”(即4字节) 整数，s表示“短”(即4字节)整数。
>
> 对于系统来说，这些函数也可实现为宏。

#### 16.3.2地址格式

一个地址标识一个特定通信域的套接字端点，地址格式与这个特定的通信域相关。为使不同格式地址能够传入到套接字函数，地址都会被强制转换成一个通用的地址结构`sockaddr`：

```c
struct sockaddr {
	sa_family_t	sa_family;//地址族(address family)
	char				sa_data[];//可变长度的地址(variable-length address)
	...
};
```

套接字实现可以自由地添加额外的成员并且定义`sa_data`成员的大 小。

在Linux中，`sa_data`结构定义如下：

```c
struct sockaddr {
	sa_family_t	sa_family;	//地址族(address family)
	char				sa_data[14];//地址(variable-length address)
};
```

在FreeBSD中，`sa_data`结构定义如下：

```c
struct sockaddr {
	unsigned char	sa_len;			//总长度(total length)
	sa_family_t		sa_family;	//地址族address family
	char					sa_data[14];//地址(variable-length address)
};
```

因特网地址定义在`<netinet/in.h>`头文件中。IPv4因特网域(`AF_INET`)套接字地址用结构`sockaddr_in`表示：

```c
struct in_addr {
	in_addr_t				s_addr;			//IPv4地址(IPv4 address)
};

struct sockaddr_in {
	sa_family_t			sin_family;	//地址族(address family)
	in_port_t				sin_port;		//端口号(port number)
	struct in_addr	sin_addr;		//IPv4地址(IPv4 address)
};
```

> 数据类型`in_port_t`定义成`uint16_t`。数据类型`in_addr_t`定义成`uint32_t`。

IPv6因特网域(`AF_INET6`)套接字地址用结构`sockaddr_in6`表示:

```c
struct in6_addr {
	uint8_t					s6_addr[16];	//IPv4地址(IPv4 address)
};

struct sockaddr_in6 {
	sa_family_t			sin6_family;	//地址族(address family)
	in_port_t				sin6_port;		//端口号(port number)
	uint32_t				sin6_flowinfo;//传输类型和流信息(traffic class and flow info)
	struct in_addr	sin6_addr;		//IPv4地址(IPv4 address)
	uint32_t				sin6_scope_id;//(set of interfaces for scope)
};
```

调用`inet_ntop()`和`inet_pton()`用于二进制地址格式与点分十进制字符表示的相互转换，支持IPv4地址和IPv6地址。

```c
#include <arpa/inet.h>
const char *inet_ntop(int domain, const void *restrict addr, char *restrict str, socklen_t size);
//函数的返回值：若成功，返回地址字符串指针；若出错，返回NULL
int inet_pton(int domain, const char *restrict str, void *restrict addr);
//函数的返回值:若成功，返回1；若格式无效，返回0;若出错，返回−1
```

参数`domain`仅支持两个值：`AF_INET`和`AF_INET6`。

对于 `inet_ntop()`，参数`size`指定了保存文本字符串的缓冲区(参数`str`)的大小,可选用下列常量。

- `INET_ADDRSTRLEN`定义了足够大的空间来存放一个表示IPv4地址的文本字符串
- `INET6_ADDRSTRLEN`定义了足够大的空间来存放一个表示IPv6地址的文本字符串。

#### 16.3.3地址查询

调用`gethostent()`可以获取当前系统的主机信息。

```c
#include <netdb.h>
struct hostent *gethostent(void);
//函数的返回值：若成功，返回指针；若出错，返回NULL
void sethostent(int stayopen);
void endhostent(void);
```

若主机数据库文件没有打开，`gethostent()`会打开它，并返回文件中的下一个条目。

调用`sethostent()`会打开文件，若文件已经被打开，则将其回绕。参数`stayopen`设为非0时，调用`gethostent()`之后，文件将依然是打开的，调用`endhostent()`才会关闭文件。

返回的`hostent`结构可能包含一个静态的数据缓冲区，每次调用`gethostent()`，缓冲区都会被覆盖。

```c
struct hostent{
	char	*h_name;			//(name of host)
	char	**h_aliases;	//(pointer to alternate host name array)
	int		h_addrtype;		//(address type)
	int		h_length;			//(length in bytes of address)
	char	**h_addr_list;//(pointer to array of network addresses)
	...
};
```

> 返回的地址采用网络字节序。

调用下列函数获得网络名字和网络编号。

```c
#include <netdb.h>
struct netent *getnetbyaddr (uint32_t net, int type);
struct netent *getnetbyname(const char* name); 
struct netent *getnetent(void);
//3个函数的返回值：若成功，返回指针；若出错，返回NULL
void setnetent(int stayopen);
void endnetent(void);
```

`netent`结构至少包含以下字段:

```c
struct netent {
	char			*n_name;//(network name)
  char			**n_aliases;//(alternate network name array pointer)
  int				n_addrtype;//(address type)
  uint32_t	n_net;//(network number)
};
```

网络编号按照网络字节序返回。地址类型(`n_addrtype`)是地址族常量之一(如`AF_INET`)。

调用下列函数在协议名字和协议编号之间进行映射。

```c
#include <netdb.h>
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);
struct protoent *getprotoent(void);
//3个函数的返回值：若成功，返回指针；若出错，返回NULL
void setprotoent(int stayopen);
void endprotoent(void);
```

POSIX.1定义的`protoent`结构至少包含以下成员:

```c
struct protoent {
	char	*p_name;		//(protocol name)
  char	**p_aliases;//(pointer to altername protocol name array)
  int		p_proto;		//(protocol number)
  ...
};
```

服务是由地址的端口号部分表示的。调用函数`getservbyname()`将服务名映射到端口号，调用函数`getservbyport()`将端口号映射到服务名，调用函数`getservent()`顺序扫描服务数据库。

```c
#include <netdb.h>
struct servent *getservbyname(const char *name, const char *proto); 
struct servent *getserbyport(int port, const char *proto);
struct servent *getservent(void);
//3个函数的返回值：若成功，返回指针；若出错，返回NULL
void setservent(int stayopen);
void endservent(void);
```

`servent`结构至少包含以下成员:

```c
struct servent {
	char	*s_name;		//(service name)
	char	**s_aliases;//(pointer to alternate service name array)
	int		s_port;			//(port number)
	char	*s_proto;		//(name of protocol)
  ...
};
```

POSIX.1定义，调用`getaddrinfo()`允许将一个主机名和一个服务名映射到一个地址。

```c
#include <sys/socket.h>
#include <netdb.h>
int getaddrinfo(const char *restrict host,
								const char *restrict service,
								const struct addrinfo *restrict hint,
								struct addrinfo **restrict res);
//函数的返回值：若成功，返回0；若出错，返回非0错误码
void freeaddrinfo(struct addrinfo *ai);
```

需要提供主机名(`host`)、服务名(`service`)，或者两者都提供。若仅提供其一，则另一个必须是`NULL`。主机名可以是一个节点名(如`localhost`)或点分格式的主机地址。

`getaddrinfo()`返回一个链表结构`addrinfo`。调用`freeaddrinfo()` 将释放一个或多个这种结构，这取决于用`ai_next`字段链接起来的结构数量。 `addrinfo`结构的定义至少包含以下成员:

```c
struct addrinfo {
	int							ai_flags;			//(customize behavior)
	int							ai_family;		//(address family)
	int							ai_socktype;	//(socket type)
	int							ai_protocol;	//(protocol)
	socklen_t				ai_addrlen;		//(length in bytes of address)
	struct sockaddr	*ai_addr;			//(address)
	char						*ai_canoname;	//(canonical name of host)
	struct					*ai_next;			//(next in list)
	...
};
```

可选参数`hint`是一个用于过滤地址的模板，来选择符合特定条件的地址，包括`ai_family`、`ai_flags`、`ai_protocol`和`ai_socktype`字段。剩余的整数字段必须设为0，指针字段为`NULL`。 `ai_flags`字段中的标志，可以用这些标志来自定义如何处理地址和名字。

- `AI_ADDRCONFIG`，查询配置的地址类型(IPv4或IPv6)
- `AI_ALL`，查询IPv4和IPv6地址(仅用于`AI_V4MAPPED`)
- `AI_CANONNAME`，需要一个规范的名字(与别名相对)
- `AI_NUMERICHOST`，以数字格式指定主机地址，不翻译
- `AI_NUMERICSERV`，将服务指定为数字端口号，不翻译
- `AI_PASSIVE`，套接字地址用于监听绑定
- `AI_V4MAPPED`，如没有找到IPv6地址，返回映射到IPv6格式的IPv4地址

若调用`getaddrinfo()`失败，需要调用`gai_strerror()`将返回的错误码转换成错误消息。

```c
#include <netdb.h>
const char *gai_strerror(int error);
//函数的返回值：指向描述错误的字符串的指针
```

调用`getnameinfo()`将一个地址转换成一个主机名和一个服务名。

```c
#include <sys/socket.h>
#include <netdb.h>
int getnameinfo(const struct sockaddr *restrict addr, socklen_t alen,
								char *restrict host, socklen_t hostlen,
								char *restrict service, socklen_t servlen, int flags);
//函数的返回值：若成功，返回0；若出错，返回非0值
```

套接字地址(`addr`)被翻译成一个主机名和一个服务名。若host非空，则指向一个长度为`hostlen`字节的缓冲区用于存放返回的主机名。若`service`非空，则指向一个长度为`servlen`字节的缓冲区用于存放返回的主机名。 

参数`flags`提供了一些控制翻译的方式。

- `NI_DGRAM`，服务基于数据报而非基于流
- `NI_NAMEREQD`，若找不到主机名，将其作为一个错误对待
- `NI_NOFQDN`，对于本地主机，仅返回全限定域名的节点名部分
- `NI_NUMERICHOST`，返回主机地址的数字形式，而非主机名
- `NI_NUMERICSCOPE`，对于IPv6，返回范围id的数字形式，而非名字
- `NI_NUMERICSERV`，返回服务地址的数字形式(即端口号)，而非名字

#### 16.3.4将套接字与地址关联

调用`bind()`来关联地址和套接字。

```c
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t len); 
//函数的返回值：若成功，返回0；若出错，返回−1
```

对于使用的地址有以下一些限制。

- 在进程正在运行的计算机上，指定的地址必须有效。不能指定一个其他机器的地址。

- 地址必须和创建套接字时的地址族所支持的格式相匹配。

- 除非该进程具有相应的root用户特权，否则地址的端口号必须不小于1024。

- 一般只能将一个套接字端点绑定到一个给定地址上，虽然有些协议允许多重绑定。

对于因特网域，若指定IP地址为`INADDR_ANY`(`<netinet/in.h>`中定义的)，套接字将被绑定到系统的所有网络接口上。

调用`getsockname()`将获取绑定到套接字上的地址。

```c
#include <sys/socket.h>
int getsockname(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict alenp);
//函数的返回值：若成功，返回0；若出错，返回−1
```

调用`getsockname()`之前，将参数`alenp`设为缓冲区`sockaddr`的长度。返回时，`alenp`会被设成返回地址的大小。若地址和提供的缓冲区长度不匹配，地址会被自动截断且不报错。若当前没有地址绑定到该套接字，其结果是未定义的。

若套接字已经和对等方连接，调用`getpeername()`将获取对方的地址。与`getsockname()`行为相同。

```c
#include <sys/socket.h>
int getpeername(int sockfd, struct sockaddr *restrict addr, 
                socklen_t *restrict alenp);
//函数的返回值：若成功，返回0；若出错，返回−1
```

### 16.4建立连接

若要处理一个面向连接的网络服务(`SOCK_STREAM`或`SOCK_SEQPACKET`)，那么在开始交换数据以前，需要在请求服务的进程套接字(客户端)和提供服务的进程套接字(服务器)之间建立一个连接。

调用`connect()`将使套接字建立连接。

```c
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *addr, socklen_t len);
//函数的返回值：若成功，返回0；若出错，返回−1
```

参数`addr`是通信对方的地址。若`sockfd`没有绑定到一个地址，函数会绑定一个默认地址。

要想一个连接请求成功，要连接的计算机必须是开启的，且正在运行，服务器必须绑定到连接的地址上，并且服务器的等待连接队列要有足够的空间。因此，程序必须处理`connect()`返回的错误，这些错误可能是由一些瞬时条件引起的。

若`connect()`失败，可迁移的程序需要关闭套接字。若想重试，必须打开一个新的套接字。

`connect()`还可以用于无连接的网络服务(`SOCK_DGRAM`)。这样每次传送报文时就不需要再提供地址。且仅能接收来自指定地址的报文。

调用`listen()`将监听连接请求。

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
//函数的返回值：若成功，返回0；若出错，返回−1
```

参数`backlog`提示系统该进程所要入队的未完成连接请求数量。其实际值由系统决定，但上限由`<sys/socket.h>`中的`SOMAXCONN`指定。

一旦队列满，系统就会拒绝多余的连接请求，所以`backlog`的值该基于服务器期望负载和处理量来选择，其中处理量是指接受连接请求与启动服务的数量。

开启监听后，调用`accept()`获得连接请求并建立连接。

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *restrict addr,
						socklen_t *restrict len);
//函数的返回值：若成功，返回文件(套接字)描述符；若出错，返回−1
```

`accept()`返回的`fd`是套接字描述符，该描述符连接到客户端。这个新的套接字描述符和`sockfd`具有相同的套接字类型和地址族。参数`sockfd`没有关联到这个连接，而是继续保持可用状态并接收其他连接请求。

参数`addr`为存放标识客户端地址的缓冲区，参数`len`为该缓冲区的字节大小。函数返回时缓冲区填充客户端的地址，并更新参数`len`来反映该地址的字节大小。
若没有连接请求在等待，`accept()`会阻塞直到一个请求到来。若`sockfd`处于非阻塞模式，函数返回−1，并设`errno`为`EAGAIN`或`EWOULDBLOCK`。

调用`accept()`会阻塞直到一个请求到来。使用`poll()`或`select()`来管理`fd`，参数`sockfd`会以可读的方式出现。

### 16.5数据传输

既然一个套接字端点表示为一个`fd`，那么只要建立连接，就可以使用read和write来通过套接字通信。

若想指定选项，从多个客户端接收数据包，或发送带外数据，就需要使用6个为数据传递而设计的套接字函数中的一个。

```c
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags); 
//函数的返回值：若成功，返回发送的字节数；若出错，返回−1
```

参数`buf`和`nbytes`的含义与`write()`中的一致。

参数`flags`指定标志来改变处理传输数据的方式。

- `MSG_CONFIRM`，提供链路层反馈以保持地址映射有效
- `MSG_DONTROUTE`，勿将数据包路由处本地网络
- `MSG_DONTWAIT`，允许非阻塞操作(等价于使用`O_NONBLOCK`)
- `MSG_EOF`，发送数据后关闭套接字的发送端
- `MSG_EOR`，若协议支持，标记记录结束
- `MSG_MORE`，延迟发送数据包允许写更多数据
- `MSG_NOSIGNAL`，在写无连接的套接字时不产生`SIGPIPE`信号
- `MSG_OOB`，若协议支持，发送带外数据

即使`send()`成功返回，也并不表示连接的另一端的进程就一定接收了数据。能保证的只是当`send()`成功返回时，数据已经被无错误地发送到网络驱动程序上。

对于支持报文边界的协议，若尝试发送的单个报文的长度超过协议所支持的最大长度 ，那么`send()`会失败，并设`errno`为`EMSGSIZE`。对于字节流协议，`send()`会阻塞直到整个数据传输完成。

`sendto`和`send`很类似。区别在于`sendto`可以在无连接的套接字上指定 一个目标地址。

```c
#include <sys/socket.h>
ssize_t sendto(int sockfd, const void *buf, 
               size_t nbytes, int flags, 
               const struct sockaddr *destaddr, 
               socklen_t destlen);
//函数的返回值：若成功，返回发送的字节数；若出错，返回−1
```

对于面向连接的套接字，目标地址是被忽略的，因为连接中隐含了目标地址。

对于无连接的套接字，除非先调用`connect()`设置了目标地址，否则不能使用`send`，而只能调用`sendto()`。

调用带有`msghdr`结构的`sendmsg`来指定多重缓冲区传输数据。

```c
#include <sys/socket.h>
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
//函数的返回值：若成功，返回发送的字节数；若出错，返回−1
```

POSIX.1定义了`msghdr`结构，它至少有以下成员：

```c
struct msghdr {
	void					*msg_name;
	socklen_t			msg_namelen;
	struct iovec	*mag_iov;
	int						msg_iovlen;
	void					*msg_control;
	socklen_t			msg_controllen;
	int						msg_flags;
	...
};

struct iovec {
  void		*iov_base;
  size_t	iov_len;
};
```

`recv()`和`read()`相似，但是`recv()`可以指定标志来控制如何接收数据。

```c
#include <sys/socket.h>
ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags); 
//函数的返回值：返回数据的字节长度；若无可用数据或对等方已经按序结束，返回0；若出错，返回−1
```

参数`flags`指定标志来控制如何接收数据。

- `MSG_CMSG_CLOEXEC`，为UNIX域套接字上接收的文件描述符设置执行时关闭标志
- `MSG_DONTWAIT`，启用非阻塞操作(相当于使用`O_NONBLOCK`)
- `MSG_ERRQUEUE`，接受错误信息作为辅助数据
- `MSG_OOB`，若协议支持，获取带外数据
- `MSG_PEEK`，返回数据包内容而不真正取走数据包
- `MSG_TRUNC`，即使数据包被截断，也返回数据包的实际长度
- `MSG_WAITALL`，等待直到所有的数据可用(仅`SOCK_STREAM`)

对于`SOCK_STREAM`套接字，接收的数据可以比预期的少。 `MSG_WAITALL`标志会阻止这种行为，直到所请求的数据全部返回， `recv()`才会返回。

对于`SOCK_DGRAM`和`SOCK_SEQPACKET`套接字，`MSG_WAITALL` 标志没有改变什么行为，因为这些基于报文的套接字类型一次读取就返回整个报文。

若发送者调用`shutdown()`来结束传输，或网络协议支持按默认的顺序关闭并且发送端已经关闭，那么当所有的数据接收完后，`recv()`会返回0。

调用`recvfrom()`将获取数据发送者的源地址。

```c
#include <sys/socket.h>
ssize_t recvfrom(int sockfd, void *restrict buf, 
									size_t len, int flags, 
									struct sockaddr *restrict addr,
									socklen_t *restrict addrlen);
//函数的返回值：返回数据的字节长度；若无可用数据或对等方已经按序结束，返回0；若出错，返回−1 
```

若`addr`非空，需要设参数`addrlen`为`addr`套接字缓冲区的字节长度。返回时，`addr`包含数据发送者的套接字端点地址，`addrlen`会被设为该地址的实际字节长度。 

因为可以获得发送者的地址，`recvfrom()`通常用于无连接的套接字。否则，`recvfrom()`等同于`recv()`。 

为了将接收到的数据送入多个缓冲区，类似于`readv()`，或者想接收辅助数据，可以调用`recvmsg()`。

```c
#include <sys/socket.h>
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags); 
//函数的返回值：返回数据的字节长度；若无可用数据或对等方已经按序结束，返回0；若出错，返回−1
```

用`msghdr`结构指定接收数据的输入缓冲区。设置参数`flags`来改变`recvmsg()`的默认行为。

- `MSG_CTRUNC`，控制数据被截断
- `MSG_EOR`，接收记录结束符
- `MSG_ERRQUEUE`，接收错误消息作为辅助数据
- `MSG_OOB`，接收带外数据
- `MSG_TRUNC`，一般数据被截断

函数返回时， `msghdr`结构中的`msg_flags`字段被设为所接收数据的各种特征。(调用`recvmsg()`时`msg_flags`被忽略。)

### 16.6套接字选项

套接字机制提供了两个套接字选项接口来控制套接字行为。可以获取或设置以下3种选项。

1. 通用选项，工作在所有套接字类型上。
2. 在套接字层次管理的选项，但是依赖于下层协议的支持。
3. 特定于某协议的选项，每个协议独有的。

调用`setsockopt()`来设置套接字选项。

```c
#include <sys/socket.h>
int setsockopt(int sockfd, int level, int option,
               const void *val, socklen_t len);
//函数的返回值：若成功，返回0；若出错，返回−1
```

参数`level`标识了选项应用的协议。

- 若选项是通用的套接字层次选项，则`level`设置成`SOL_SOCKET`。
- 否则设置成控制这个选项的协议编号。对于TCP，`level`是`IPPROTO_TCP`，对于IP，`level`是`IPPROTO_IP`。

参数`option`为套接字选项。

- `SO_ACCEPTCONN`，参数`val`类型为`int`。返回信息指示该套接字是否被监听(仅getsockopt)
- `SO_BROADCAST`，参数`val`类型为`int`。若`*val`非0，广播数据报
- `SO_DEBUG`，参数`val`类型为`int`。若`*val`非0，启动网络驱动调试功能
- `SO_DONTROUTE`，参数`val`类型为`int`。若`*val`非0，绕过通常路由
- `SO_ERROR`，参数`val`类型为`int`。返回挂起的套接字错误并清除(仅`getsockopt()`)
- `SO_KEEPALIVE`，参数`val`类型为`int`。若`*val`非0，启用周期性keep-alive报文
- `SO_LINGER`，参数`val`类型为`struct linger`。当还有未发报文而套接字已关闭时，延迟时间
- `SO_OOBINLINE`，参数`val`类型为`int`。若`*val`非0，将带外数据放在普通数据中
- `SO_RCVBUF`，参数`val`类型为`int`。接收缓冲区的字节长度
- `SO_RCVLOWAT`，参数`val`类型为`int`。接收调用中返回的最小数据字节数
- `SO_RCVTIMEO`，参数`val`类型为`timeval`。套接字接收调用的超时值
- `SO_REUSEADDR`，参数`val`类型为`int`。若`*val`非0，重用bind中的地址
- `SO_SNDBUF`，参数`val`类型为`int`。发送缓冲区的字节长度
- `SO_SNDLWAT`，参数`val`类型`int`。发送调用中传送的最小数据字节数
- `SO_SNDTIMEO`，参数`val`类型`struct timeval`。套接字发送调用的超时值
- `SO_TYPE`，参数`val`类型为`int`。标识套接字类型(仅`getcokopt`)

参数`val`根据选项的不同数据类型不同。一些选项是on/off开关。若整数非0，则启用选项。若整数为0，则禁止选项。

参数`len`指定了参数`val`指向的对象的大小。

调用`getsockopt()`来查看选项的当前值。

```c
#include <sys/socket.h>
int getsockopt(int sockfd, int level, int option, 
               void *restrict val, socklen_t *restrict lenp);
//函数的返回值：若成功，返回0；若出错，返回−1
```

参数`lenp`是一个指向整数的指针。在调用`getsockopt()`之前，设置该整数为复制选项缓冲区的长度。

- 若选项的实际长度大于此值，则选项会被截断。

- 若实际长度正好小于此值，那么返回时将此值更新为实际长度。

### 16.7带外数据

带外数据(out-of-band data)是一些通信协议所支持的可选功能，与普通数据相比，它允许更高优先级的数据传输。带外数据先行传输，即使传输队列已经有数据。TCP支持带外数据，但是UDP不支持。

TCP将带外数据称为紧急数据(urgent data)。TCP仅支持一个字节的紧急数据，但允许紧急数据在普通数据传递机制数据流之外传输。可以在3个`send`函数中指定`MSG_OOB`标志。若带`MSG_OOB`标志发送的超过一个字节，则最后一个字节为紧急数据字节。

若通过套接字安排了信号的产生，那么紧急数据被接收时，会发送`SIGURG`信号。

在`fcntl()`中使用`F_SETOWN`命令来设置一个套接字的所有权。第三个参数将影响函数行为：即`fcntl(sockfd, F_SETOWN, pid);`

- 若为正值，则指定的是进程id。
- 若为非-1的负值，则指定的是进程组id。

在`fcntl()`中使用`F_GETOWN`命令可以获取相关套接字的所有权。即`owner = fcntl(sockfd, F_GETOWN, 0);`

- 若owner为正值，则为接收套接字信号的进程的id。

- 若owner为负值，其绝对值为接收套接字信号的进程组的id。

TCP支持紧急标记(urgent mark)的概念，即在普通数据流中紧急数据所在的位置。若采用套接字选项`SO_OOBINLINE`，则可以在普通数据中接收紧急数据。

调用`sockatmark()`判断是否已经到达紧急标记。

```c
#include <sys/socket.h>
int sockatmark(int sockfd);
//函数的返回值：若在标记处，返回1；若没在标记处，返回0；若出错，返回−1
```

当下一个要读取的字节在紧急标志处时，`sockatmark()`返回1。

当带外数据出现在套接字读取队列时，`select()`会返回一个`fd`并且有一个待处理的异常条件。

可以在普通数据流上接收紧急数据，也可以在`recv`函数中采用`MSG_OOB`标志在其他队列数据之前接收紧急数据。

TCP队列仅用一个字节的紧急数据。若在接收当前的紧急数据字节之前又有新的紧急数据到来，那么已有的字节会被丢弃。

### 16.8非阻塞和异步I/O

通常，`recv()`没有数据可用时会阻塞等待。同样地，当套接字输出队列没有足够空间来发送消息时，`send()`会阻塞。在套接字非阻塞模式下，行为会改变。在这种情况下，这些函数不会阻塞而是会失败，设`errno`为`EWOULDBLOCK`或者`EAGAIN`。使用`poll()`或`select()`来判断能否接收或者传输数据。

在基于套接字的异步I/O中，当从套接字中读取数据时，或者当套接字写队列中缓冲区变为可用时，可以安排发送信号`SIGIO`。

启用异步I/O有两个步骤：

1. 建立套接字所有权，这样信号可以被传递到合适的进程。 用以下操作完成：
	- 在`fcntl()`中使用`F_SETOWN`命令。
	- 在`ioctl()`中使用`FIOSETOWN`命令。
	- 在`ioctl()`中使用`SIOCSPGRP`命令。
2. 当套接字当I/O操作不会阻塞时发送信号。 用以下操作完成：
	- 在`fcntl()`中使用`F_SETFL`命令并且启用文件标志`O_ASYNC()`
	- 在`ioctl()`中使用`FIOASYNC`命令。

