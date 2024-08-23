#include <linux/bpf.h>
#include <bpf_helpers.h>
#include <stdio.h>
#include <linux/types.h>

#define SEC(NAME) __attribute__((section(NAME), used))

//定义c的结构，将pid和comm封装到该结构体，将该结构体通过tracker_map传递给用户态
struct event_data_t {
    __u32 pid;
    char comm[20];
};

//定义ebpf的map
struct bpf_map_def SEC("maps") tracker_map = {
        .type = BPF_MAP_TYPE_HASH,
        .key_size = sizeof(int),
        .value_size = sizeof(struct event_data_t),
        .max_entries = 2048,
};

//ebpf程序挂载点，
SEC("tracepoint/syscalls/sys_enter_execve")
int bpf_prog(void *ctx) {
    int index = 9;
  
    struct event_data_t evt = {};
    
    
    __u64 id = bpf_get_current_pid_tgid();
    evt.pid = id >> 32;
    
    bpf_get_current_comm(&evt.comm, sizeof(evt.comm));
    bpf_map_update_elem(&tracker_map, &index, &evt, BPF_ANY);
    
    return 0;
}

//协议
char _license[] SEC("license") = "GPL";
