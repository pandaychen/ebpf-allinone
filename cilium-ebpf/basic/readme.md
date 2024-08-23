##  作用
该代码仅仅是一个 demo，无太多意义，因为 hashtable 的 key 是指定的

##  编译方法
其中，本机的 `bpf_helpers.h` 存放在 `/usr/include/bpf/bpf_helpers.h` 目录
```BASH
clang -O2 -target bpf -c bpf_program.c -I/usr/include/bpf/ -o bpf_program.o
go build bpf_program.go
```

## 输出

```TEXT
root@VM-16-15-ubuntu:~/bpfgo# ./bpf_program
2024/08/23 17:55:22 Waiting for events..
2024/08/23 17:55:23 reading map: lookup: key does not exist
9 [254 37 32 0 115 104 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:24 pid:2106878,comm:sh
9 [254 37 32 0 115 104 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:25 pid:2106878,comm:sh
9 [6 38 32 0 98 97 114 97 100 95 97 103 101 110 116 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:26 pid:2106886,comm:barad_agent
9 [15 38 32 0 115 104 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:27 pid:2106895,comm:sh
9 [30 38 32 0 115 104 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:28 pid:2106910,comm:sh
9 [34 38 32 0 98 97 114 97 100 95 97 103 101 110 116 0 0 0 0 0 0 0 0 0]
2024/08/23 17:55:29 pid:2106914,comm:barad_agent
```


##  代码注释（from gpt4）

```text
1、这段代码的含义，bpf_get_current_comm、bpf_map_update_elem 的作用

assistant
这段代码定义了一个 eBPF 程序，用于追踪 execve 系统调用。eBPF（Extended Berkeley Packet Filter）是 Linux 内核中的一种内核程序类型，可以用于实现各种内核级别的监控、分析和过滤等功能。

首先，定义了一个名为 event_data_t 的结构体，包含一个进程 ID（pid）和一个进程名称（comm）。这个结构体将用于在 eBPF 程序和用户空间之间传递数据。

接下来，定义了一个名为 tracker_map 的 eBPF 哈希映射，用于存储 event_data_t 结构体的实例。该映射的键大小为 int，值大小为 event_data_t 结构体的大小，最大条目数为 2048。

然后，定义了一个名为 bpf_prog 的 eBPF 程序，挂载在 tracepoint/syscalls/sys_enter_execve 上。这意味着每当有进程调用 execve 系统调用时，这个 eBPF 程序都会被执行。

在 bpf_prog 函数中，首先创建了一个 event_data_t 结构体实例 evt。然后，使用 bpf_get_current_pid_tgid() 函数获取当前进程的 ID，并将其存储在 evt.pid 中。注意，bpf_get_current_pid_tgid() 返回的是一个 64 位整数，其中高 32 位是进程 ID，低 32 位是线程 ID。这里通过右移 32 位来获取进程 ID。

接下来，使用 bpf_get_current_comm() 函数获取当前进程的名称，并将其存储在 evt.comm 中。这个函数的第一个参数是一个指向缓冲区的指针，用于存储进程名称；第二个参数是缓冲区的大小。

最后，使用 bpf_map_update_elem() 函数将 evt 结构体实例添加到 tracker_map 映射中。这个函数的第一个参数是指向映射的指针；第二个参数是指向映射键的指针；第三个参数是指向映射值的指针；第四个参数是一个标志，用于指定如何更新映射。这里使用 BPF_ANY 表示如果键已经存在，则更新对应的值；如果键不存在，则创建一个新条目。

总之，这段代码的目的是在每次 execve 系统调用发生时，记录进程 ID 和进程名称，并将其存储在一个 eBPF 哈希映射中。这可以用于监控进程执行情况，例如检测潜在的恶意进程。



2、那这段用户态的代码呢？其中 mapKey 的初始值为 9

这段代码是一个 Go 程序，用于与前面提到的 eBPF 程序交互。它从 eBPF 哈希映射 tracker_map 中读取数据，并将其反序列化为 Go 结构体，然后将进程 ID 和进程名称打印出来。

首先，创建一个名为 ticker 的定时器，每隔 1 秒触发一次。这意味着程序将每秒检查一次 tracker_map 映射中的数据。

然后，使用 log.Println() 打印一条消息，表示程序正在等待事件。

接下来，使用 for range ticker.C 循环每隔 1 秒执行一次循环体。

在循环体中，首先声明一个名为 value 的字节切片。然后，使用 obj.TrackerMap.Lookup() 函数尝试从 tracker_map 映射中查找键为 mapKey 的条目。这里，mapKey 的初始值为 9。如果查找失败，打印一条错误消息；如果查找成功，将找到的值赋给 value，并打印 mapKey 和 value。

接着，声明一个名为 event 的 EventData 结构体实例。使用 binary.Read() 函数从 value 字节切片中读取数据，并将其反序列化为 event 结构体。注意，这里使用 binary.LittleEndian 表示数据以小端字节序存储。如果反序列化失败，打印一条错误消息并跳过当前循环；如果反序列化成功，使用 log.Printf() 打印进程 ID（event.Pid）和进程名称（event.CommHex()）。

总之，这段 Go 代码用于与前面提到的 eBPF 程序交互，每隔 1 秒从 eBPF 哈希映射中读取数据，并将进程 ID 和进程名称打印出来。这样，用户可以实时监控 execve 系统调用的情况。

3、这里为啥用固定的 index 来做为 hashtable 的 key，是不是不合理？

您是对的，使用固定的 index（在这个例子中是 mapKey）作为哈希表的键可能会导致一些问题。在这种情况下，如果在 1 秒内有多个 execve 系统调用发生，它们的数据将会覆盖之前的数据，因为它们都使用相同的键。这可能导致您无法捕获到所有的 execve 事件。

要解决这个问题，您可以使用不同的策略来选择哈希表的键。例如，您可以使用进程 ID 作为键，因为它在系统中是唯一的。这样，您就可以在哈希表中存储每个进程的 execve 事件，而不会发生覆盖。当然，这也意味着您需要在用户空间代码中遍历哈希表以获取所有事件，而不是只查找一个固定的键。

另一个方法是使用一个递增的计数器作为键。在 eBPF 程序中，您可以使用一个名为 counter 的全局变量来跟踪当前的计数。每次 execve 事件发生时，将 counter 作为键，并将其值递增。这样，您可以确保每个事件都有一个唯一的键。在用户空间代码中，您需要遍历哈希表以获取所有事件。为了避免无限制地增长哈希表，您可以在达到最大条目数时将计数器归零。这种方法可能会导致一些事件被覆盖，但至少可以捕获到更多的事件。
```