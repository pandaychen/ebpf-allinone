// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2021 Sartura */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("kprobe/do_unlinkat")
int BPF_KPROBE(do_unlinkat, int dfd, struct filename *name)
{
	pid_t pid;
	const char *filename;

	pid = bpf_get_current_pid_tgid()>> 32;
	filename = BPF_CORE_READ(name, name);
	//filename = (char *)PT_REGS_PARM2(ctx);
	bpf_printk("KPROBE ENTRY pid = %d, filename = %s\n", pid, filename);
	return 0;
}

SEC("kretprobe/do_unlinkat")
int BPF_KRETPROBE(do_unlinkat_exit, long ret)
{
	pid_t pid;

	pid = bpf_get_current_pid_tgid()>> 32;
	bpf_printk("KPROBE EXIT: pid = %d, ret = %ld\n", pid, ret);
	return 0;
}

SEC("kprobe/vfs_mkdir")
//int BPF_KPROBE(vfs_mkdir)
int kprobe__vfs_mkdir(struct pt_regs *ctx)
{
       struct dentry *dentry;
       struct qstr d_name;
       dentry = (struct dentry *)PT_REGS_PARM3(ctx);
    pid_t pid;
      pid = bpf_get_current_pid_tgid()>> 32;
  //  bpf_printk("kprobe,mkdir (vfs hook point)%s\n",dentry->d_name.name);
  	 char name[32];
	 // bpf_probe_read((void *)&_val, sizeof(_val), &ptr);
   // bpf_probe_read_kernel(&name, sizeof(name), &dentry->d_name.name);
     bpf_probe_read((void *)&d_name,sizeof(struct qstr),&dentry->d_name);

   bpf_probe_read_str((void *)&name, sizeof(name), d_name.name);

    bpf_printk("kprobe,mkdir (vfs hook point)%s,%d\n", name,pid);
    return 0;
}

SEC("kretprobe/vfs_mkdir")
int kretpobe_mkdir(void *ctx)
{
    bpf_printk("kretprobe,mkdir (vfs hook point)\n");
    return 0;
}
