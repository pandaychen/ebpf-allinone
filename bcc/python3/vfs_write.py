#!/usr/bin/python
from bcc import BPF

prog = """
    #include <asm/ptrace.h>
    #include <linux/fs.h>

    int trace_vfs_write(struct pt_regs *ctx, struct file *file, const char __user *buf, size_t count, loff_t *pos)
    {
        u32 pid = bpf_get_current_pid_tgid() >> 32;
        u32 uid = bpf_get_current_uid_gid();

        bpf_trace_printk("%d %d %s\\n", pid, file->f_inode->i_ino, file->f_path.dentry->d_iname);
        return 0;
    }
"""

b = BPF(text = prog)
b.attach_kprobe(event = "vfs_write", fn_name = "trace_vfs_write")

print("%-18s %-16s %-6s %s"%("TIME(s)", "COMM", "PID", "MESSAGE"))

#output
while 1:
    try:
        (task, pid, cpu, flags, ts, msg) = b.trace_fields()
    except ValueError:
        continue
    print("%-18.9f %-16s %-6d %s"%(ts, task, pid, msg))


