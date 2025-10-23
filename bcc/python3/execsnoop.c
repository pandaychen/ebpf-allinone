#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
#include <linux/fs.h>

#define ARGSIZE 64
#define TOTAL_MAX_ARGS 5
#define FULL_MAX_ARGS_ARR (TOTAL_MAX_ARGS * ARGSIZE)
#define LAST_ARG (FULL_MAX_ARGS_ARR - ARGSIZE)

// 设置数据结构
struct data_t {
    u32 pid;
    char comm[TASK_COMM_LEN];
    int retval;
    unsigned int args_size;
    char argv[FULL_MAX_ARGS_ARR];
};
BPF_PERF_OUTPUT(events);

BPF_HASH(tasks, u32, struct data_t);

// 帮助函数从用户空间中读取数据
static int __bpf_read_arg_str(struct data_t *data, const char *ptr)
{
    // 如果已存储的参数长度大于最后一个参数的位置，则返回错误
 if (data->args_size > LAST_ARG) {
  return -1;
 }
 /*int bpf_probe_read_user_str(void *dst, int size, const void *src)
    - 成功返回包含结尾NULL（'\0'）字符的字符串长度（>0）
    - 出错返回负值（<0）
    该函数将用户地址空间中以NULL结尾的字符串复制到BPF栈中，以便BPF后续处理
    若字符串长度小于size参数，目标缓冲区不会用额外的NULL字节填充；
    若字符串长度大于size，则仅复制size-1个字节，并将最后一个字节设为NULL
 */
 int ret = bpf_probe_read_user_str(&data->argv[data->args_size], ARGSIZE,
       (void *)ptr);
 if (ret > ARGSIZE || ret < 0) {
  return -1;
 }
 
 // 如果不是第一个参数，将前一个参数的null终止符替换为空格
 if (data->args_size > 0 && data->args_size < FULL_MAX_ARGS_ARR) {
  data->argv[data->args_size - 1] = ' ';
 }
 
 // 增加参数长度，但确保不会越界
 data->args_size += ret;
 if (data->args_size >= FULL_MAX_ARGS_ARR) {
  data->args_size = FULL_MAX_ARGS_ARR - 1;
 }
 return 0;
}


/*
TRACEPOINT_PROBE：这是一个宏，用于对由category:event定义的跟踪点进行插桩
探针函数名为tracepoint____
参数可通过args结构体获取，这些参数即为跟踪点参数
args结构体可以替代需要上下文作为参数的每个函数中的ctx
*/
TRACEPOINT_PROBE(syscalls, sys_enter_execve)
{
    unsigned int ret = 0;
    // 使用const 防止后续修改的可能
    // 获取其启动参数
    //注意这里使用的指针的指针，用来获取启动参数的用户空间位置，并且使用const进行标识，不允许使用过程中进行修改。bpf校验器也会进行检查
    const char **argv = (const char **)(args->argv);
    // 获取进程名以及Pid
    struct data_t data = { };
    u32 pid = bpf_get_current_pid_tgid();
    data.pid = pid;
    bpf_get_current_comm(&data.comm, sizeof(data.comm));
    
    // 获取第一个参数（即可执行文件的名字）
    if (__bpf_read_arg_str(&data, (const char *)argv[0]) < 0) {
        goto out;
    }

    // #pragma unroll 告诉编译器，把源码中的循环自动展开。这就避免了最终的字节码中包含循环
    #pragma unroll
    for (int i = 1; i < TOTAL_MAX_ARGS; i++) {
        if (__bpf_read_arg_str(&data, (const char *)argv[i]) < 0) {
            goto out;
        }
    }
out:
    // 存储到哈希映射中
    tasks.update(&pid, &data);
    return 0;
}

TRACEPOINT_PROBE(syscalls, sys_exit_execve)
{
    u32 pid = bpf_get_current_pid_tgid();
    struct data_t *data = tasks.lookup(&pid);
    if (data != NULL) {
        data->retval = args->ret;
        events.perf_submit(args, data, sizeof(struct data_t));
        // 清理删除数据
        tasks.delete(&pid);
    }
    return 0;
}
