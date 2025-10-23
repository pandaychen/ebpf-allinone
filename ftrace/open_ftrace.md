####    open系统调用链

本次实验基于内核版本`6.6.30-5.tl4.x86_64`

ftrace配置：
```BASH
[root@VM-X-X-tencentos tracing]# cat set_graph_function 
do_sys_openat2

[root@VM-X-X-tencentos tracing]# cat set_ftrace_pid 
3804468
```

测试代码：

```CPP
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define FILE_PATH "testfile.txt"      // 要打开的文件路径
#define OPERATIONS_PER_SECOND 1    // 每秒执行的openat操作数
#define INTERVAL_SECONDS 30             // 执行间隔（秒）

int main() {
    int fd;
    int i, j;
    time_t start_time, current_time;
    double elapsed_seconds = 0.0;

    while (1) {
        start_time = time(NULL);

        for (i = 0; i < OPERATIONS_PER_SECOND; i++) {
            // 使用open打开文件
            fd = open(FILE_PATH, O_RDWR|O_CREAT|O_APPEND );
            if (fd < 0) {
                perror("open failed");
                exit(EXIT_FAILURE);
            }

            if (write(fd, "a", 1)!= 1) {
                perror("write failed");
                close(fd);
                continue;
            }          
            // 立即关闭文件描述符
            if (close(fd) < 0) {
                perror("close failed");
                exit(EXIT_FAILURE);
            }
        }

        // 计算已用时间
        current_time = time(NULL);
        elapsed_seconds = difftime(current_time, start_time);

        // 如果已经过了指定的间隔时间，继续下一个周期
        if (elapsed_seconds >= INTERVAL_SECONDS) {
        } else {
            // 否则，等待剩余的时间
            sleep((int)(INTERVAL_SECONDS - elapsed_seconds));
        }
    }

    return 0;
}
```

ftrace跟踪结果：

```TEXT
[root@VM-X-X-tencentos tracing]# cat trace
# tracer: function_graph
#
# CPU  TASK/PID         DURATION                  FUNCTION CALLS
# |     |    |           |   |                     |   |   |   |
 4) open2-3801782  |               |  do_sys_openat2() {        #https://elixir.bootlin.com/linux/v6.15/source/fs/open.c#L1412
 4) open2-3801782  |               |    getname() {
 4) open2-3801782  |               |      getname_flags.part.0() {
 4) open2-3801782  |               |        kmem_cache_alloc() {
 4) open2-3801782  |   0.530 us    |          __cond_resched();
 4) open2-3801782  |   0.340 us    |          should_failslab();
 4) open2-3801782  |   2.054 us    |        }
 4) open2-3801782  |   2.993 us    |      }
 4) open2-3801782  |   4.033 us    |    }
 4) open2-3801782  |               |    get_unused_fd_flags() {
 4) open2-3801782  |               |      alloc_fd() {
 4) open2-3801782  |   0.575 us    |        _raw_spin_lock();
 4) open2-3801782  |   0.346 us    |        expand_files();
 4) open2-3801782  |   0.334 us    |        _raw_spin_unlock();
 4) open2-3801782  |   3.355 us    |      }
 4) open2-3801782  |   3.965 us    |    }
 4) open2-3801782  |               |    do_filp_open() {
 4) open2-3801782  |               |      path_openat() {
 4) open2-3801782  |               |        alloc_empty_file() {
 4) open2-3801782  |               |          kmem_cache_alloc() {
 4) open2-3801782  |   0.320 us    |            __cond_resched();
 4) open2-3801782  |   0.346 us    |            should_failslab();
 4) open2-3801782  |   0.348 us    |            __rcu_read_lock();
 4) open2-3801782  |               |            __get_obj_cgroup_from_memcg() {
 4) open2-3801782  |   0.469 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.272 us    |              __rcu_read_unlock();
 4) open2-3801782  |   1.701 us    |            }
 4) open2-3801782  |   0.271 us    |            __rcu_read_unlock();
 4) open2-3801782  |               |            obj_cgroup_charge() {
 4) open2-3801782  |   0.243 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.259 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.266 us    |              __rcu_read_unlock();
 4) open2-3801782  |   0.246 us    |              __rcu_read_unlock();
 4) open2-3801782  |               |              try_charge_memcg() {
 4) open2-3801782  |               |                page_counter_try_charge() {
 4) open2-3801782  |   0.494 us    |                  propagate_protected_usage();
 4) open2-3801782  |   0.248 us    |                  propagate_protected_usage();
 4) open2-3801782  |   0.276 us    |                  propagate_protected_usage();
 4) open2-3801782  |   0.285 us    |                  propagate_protected_usage();
 4) open2-3801782  |   3.231 us    |                }
 4) open2-3801782  |               |                refill_stock() {
 4) open2-3801782  |               |                  drain_stock() {
 4) open2-3801782  |               |                    page_counter_uncharge() {
 4) open2-3801782  |   0.279 us    |                      propagate_protected_usage();
 4) open2-3801782  |   0.292 us    |                      propagate_protected_usage();
 4) open2-3801782  |   0.274 us    |                      propagate_protected_usage();
 4) open2-3801782  |   2.535 us    |                    }
 4) open2-3801782  |   0.287 us    |                    __rcu_read_lock();
 4) open2-3801782  |   0.284 us    |                    __rcu_read_unlock();
 4) open2-3801782  |   4.332 us    |                  }
 4) open2-3801782  |   0.277 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.259 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   6.043 us    |                }
 4) open2-3801782  | + 10.398 us   |              }
 4) open2-3801782  |               |              memcg_account_kmem() {
 4) open2-3801782  |               |                __mod_memcg_state() {
 4) open2-3801782  |   0.292 us    |                  cgroup_rstat_updated();
 4) open2-3801782  |   1.320 us    |                }
 4) open2-3801782  |   1.934 us    |              }
 4) open2-3801782  |   0.269 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.271 us    |              __rcu_read_unlock();
 4) open2-3801782  |               |              refill_obj_stock() {
 4) open2-3801782  |   0.303 us    |                drain_obj_stock();
 4) open2-3801782  |   0.274 us    |                __rcu_read_lock();
 4) open2-3801782  |   0.258 us    |                __rcu_read_unlock();
 4) open2-3801782  |   0.260 us    |                __rcu_read_lock();
 4) open2-3801782  |   0.297 us    |                __rcu_read_unlock();
 4) open2-3801782  |               |                obj_cgroup_uncharge_pages() {
 4) open2-3801782  |   0.259 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.279 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.290 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   0.286 us    |                  __rcu_read_unlock();
 4) open2-3801782  |               |                  memcg_account_kmem() {
 4) open2-3801782  |               |                    __mod_memcg_state() {
 4) open2-3801782  |   0.279 us    |                      cgroup_rstat_updated();
 4) open2-3801782  |   1.105 us    |                    }
 4) open2-3801782  |   1.592 us    |                  }
 4) open2-3801782  |   0.293 us    |                  refill_stock();
 4) open2-3801782  |   0.279 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.266 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   6.094 us    |                }
 4) open2-3801782  | + 10.196 us   |              }
 4) open2-3801782  | + 26.726 us   |            }
 4) open2-3801782  |   0.287 us    |            __rcu_read_lock();
 4) open2-3801782  |   0.280 us    |            __rcu_read_unlock();
 4) open2-3801782  |   0.305 us    |            mod_objcg_state();
 4) open2-3801782  |   0.223 us    |            __rcu_read_lock();
 4) open2-3801782  |   0.207 us    |            __rcu_read_unlock();
 4) open2-3801782  | + 34.743 us   |          }
 4) open2-3801782  |               |          init_file() {
 4) open2-3801782  |               |            security_file_alloc() {
 4) open2-3801782  |               |              kmem_cache_alloc() {
 4) open2-3801782  |   0.169 us    |                __cond_resched();
 4) open2-3801782  |   0.170 us    |                should_failslab();
 4) open2-3801782  |   1.051 us    |              }
 4) open2-3801782  |   0.355 us    |              selinux_file_alloc_security();
 4) open2-3801782  |   0.162 us    |              bpf_lsm_file_alloc_security();
 4) open2-3801782  |   2.459 us    |            }
 4) open2-3801782  |   0.160 us    |            __mutex_init();
 4) open2-3801782  |   3.366 us    |          }
 4) open2-3801782  | + 38.938 us   |        }
 4) open2-3801782  |               |        path_init() {
 4) open2-3801782  |   0.164 us    |          __rcu_read_lock();
 4) open2-3801782  |   0.806 us    |        }
 4) open2-3801782  |               |        link_path_walk.part.0.constprop.0() {
 4) open2-3801782  |               |          inode_permission() {
 4) open2-3801782  |               |            generic_permission() {
 4) open2-3801782  |   0.168 us    |              make_vfsuid();
 4) open2-3801782  |   0.546 us    |            }
 4) open2-3801782  |               |            security_inode_permission() {
 4) open2-3801782  |               |              selinux_inode_permission() {
 4) open2-3801782  |   0.173 us    |                __inode_security_revalidate();
 4) open2-3801782  |   0.159 us    |                __rcu_read_lock();
 4) open2-3801782  |   0.236 us    |                avc_lookup();
 4) open2-3801782  |   0.172 us    |                __rcu_read_unlock();
 4) open2-3801782  |   1.791 us    |              }
 4) open2-3801782  |   0.168 us    |              bpf_lsm_inode_permission();
 4) open2-3801782  |   2.612 us    |            }
 4) open2-3801782  |   3.787 us    |          }
 4) open2-3801782  |   0.160 us    |          make_vfsuid();
 4) open2-3801782  |   4.550 us    |        }
 4) open2-3801782  |               |        open_last_lookups() {
 4) open2-3801782  |               |          try_to_unlazy() {
 4) open2-3801782  |   0.176 us    |            legitimize_links();
 4) open2-3801782  |               |            __legitimize_path() {
 4) open2-3801782  |   0.224 us    |              __legitimize_mnt();
 4) open2-3801782  |   0.679 us    |            }
 4) open2-3801782  |   0.176 us    |            __rcu_read_unlock();
 4) open2-3801782  |   1.890 us    |          }
 4) open2-3801782  |               |          mnt_want_write() {
 4) open2-3801782  |   0.159 us    |            __cond_resched();
 4) open2-3801782  |   0.188 us    |            __mnt_want_write();
 4) open2-3801782  |   0.983 us    |          }
 4) open2-3801782  |               |          down_write() {
 4) open2-3801782  |   0.159 us    |            __cond_resched();
 4) open2-3801782  |   0.655 us    |          }
 4) open2-3801782  |               |          lookup_open.isra.0() {
 4) open2-3801782  |               |            d_lookup() {
 4) open2-3801782  |               |              __d_lookup() {
 4) open2-3801782  |   0.163 us    |                __rcu_read_lock();
 4) open2-3801782  |   0.165 us    |                _raw_spin_lock();
 4) open2-3801782  |   0.188 us    |                d_same_name();
 4) open2-3801782  |   0.166 us    |                _raw_spin_unlock();
 4) open2-3801782  |   0.160 us    |                __rcu_read_unlock();
 4) open2-3801782  |   2.108 us    |              }
 4) open2-3801782  |   2.536 us    |            }
 4) open2-3801782  |   2.901 us    |          }
 4) open2-3801782  |   0.168 us    |          up_write();
 4) open2-3801782  |   0.185 us    |          mnt_drop_write();
 4) open2-3801782  |               |          step_into() {
 4) open2-3801782  |               |            dput() {
 4) open2-3801782  |   0.162 us    |              __cond_resched();
 4) open2-3801782  |   0.163 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.163 us    |              __rcu_read_unlock();
 4) open2-3801782  |   1.177 us    |            }
 4) open2-3801782  |   1.537 us    |          }
 4) open2-3801782  |   9.913 us    |        }
 4) open2-3801782  |               |        do_open() {
 4) open2-3801782  |   0.173 us    |          complete_walk();
 4) open2-3801782  |               |          may_open() {
 4) open2-3801782  |               |            inode_permission() {
 4) open2-3801782  |   0.160 us    |              make_vfsuid();
 4) open2-3801782  |   0.161 us    |              make_vfsgid();
 4) open2-3801782  |               |              generic_permission() {
 4) open2-3801782  |   0.162 us    |                make_vfsuid();
 4) open2-3801782  |               |                capable_wrt_inode_uidgid() {
 4) open2-3801782  |               |                  security_capable() {
 4) open2-3801782  |   0.251 us    |                    cap_capable();
 4) open2-3801782  |               |                    selinux_capable() {
 4) open2-3801782  |               |                      cred_has_capability.isra.0() {
 4) open2-3801782  |   0.429 us    |                        __rcu_read_lock();
 4) open2-3801782  |   0.320 us    |                        avc_lookup();
 4) open2-3801782  |   0.164 us    |                        __rcu_read_unlock();
 4) open2-3801782  |   1.650 us    |                      }
 4) open2-3801782  |   1.995 us    |                    }
 4) open2-3801782  |   0.164 us    |                    bpf_lsm_capable();
 4) open2-3801782  |   3.371 us    |                  }
 4) open2-3801782  |               |                  privileged_wrt_inode_uidgid() {
 4) open2-3801782  |   0.165 us    |                    make_vfsuid();
 4) open2-3801782  |               |                    from_kuid() {
 4) open2-3801782  |   0.206 us    |                      map_id_up();
 4) open2-3801782  |   0.542 us    |                    }
 4) open2-3801782  |   0.165 us    |                    make_vfsgid();
 4) open2-3801782  |               |                    from_kgid() {
 4) open2-3801782  |   0.182 us    |                      map_id_up();
 4) open2-3801782  |   0.497 us    |                    }
 4) open2-3801782  |   2.161 us    |                  }
 4) open2-3801782  |   6.076 us    |                }
 4) open2-3801782  |   6.890 us    |              }
 4) open2-3801782  |               |              security_inode_permission() {
 4) open2-3801782  |               |                selinux_inode_permission() {
 4) open2-3801782  |               |                  __inode_security_revalidate() {
 4) open2-3801782  |   0.163 us    |                    __cond_resched();
 4) open2-3801782  |   0.488 us    |                  }
 4) open2-3801782  |   0.166 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.274 us    |                  avc_lookup();
 4) open2-3801782  |   0.161 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   2.106 us    |                }
 4) open2-3801782  |   0.173 us    |                bpf_lsm_inode_permission();
 4) open2-3801782  |   2.801 us    |              }
 4) open2-3801782  | + 11.250 us   |            }
 4) open2-3801782  | + 11.688 us   |          }
 4) open2-3801782  |               |          vfs_open() {
 4) open2-3801782  |               |            do_dentry_open() {
 4) open2-3801782  |               |              path_get() {
 4) open2-3801782  |   0.174 us    |                mntget();
 4) open2-3801782  |   0.551 us    |              }
 4) open2-3801782  |   0.166 us    |              __mnt_want_write();
 4) open2-3801782  |   0.158 us    |              try_module_get();
 4) open2-3801782  |               |              security_file_open() {
 4) open2-3801782  |               |                selinux_file_open() {
 4) open2-3801782  |               |                  inode_security() {
 4) open2-3801782  |   0.160 us    |                    __cond_resched();
 4) open2-3801782  |   0.481 us    |                  }
 4) open2-3801782  |   0.166 us    |                  avc_policy_seqno();
 4) open2-3801782  |               |                  inode_has_perm() {
 4) open2-3801782  |               |                    avc_has_perm() {
 4) open2-3801782  |   0.164 us    |                      __rcu_read_lock();
 4) open2-3801782  |   0.163 us    |                      avc_lookup();
 4) open2-3801782  |   0.160 us    |                      __rcu_read_unlock();
 4) open2-3801782  |   1.107 us    |                    }
 4) open2-3801782  |   1.421 us    |                  }
 4) open2-3801782  |   2.737 us    |                }
 4) open2-3801782  |               |                bpf_lsm_file_open() {
 4) open2-3801782  |   0.174 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.167 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   0.169 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.166 us    |                  migrate_disable();
 4) open2-3801782  |               |                  copy_from_kernel_nofault() {
 4) open2-3801782  |   0.177 us    |                    copy_from_kernel_nofault_allowed();
 4) open2-3801782  |   0.598 us    |                  }
 4) open2-3801782  |               |                  copy_from_kernel_nofault() {
 4) open2-3801782  |   0.166 us    |                    copy_from_kernel_nofault_allowed();
 4) open2-3801782  |   0.470 us    |                  }
 4) open2-3801782  |               |                  copy_from_kernel_nofault() {
 4) open2-3801782  |   0.168 us    |                    copy_from_kernel_nofault_allowed();
 4) open2-3801782  |   0.475 us    |                  }
 4) open2-3801782  |               |                  bpf_get_current_cgroup_id() {
 4) open2-3801782  |   0.161 us    |                    __rcu_read_lock();
 4) open2-3801782  |   0.172 us    |                    __rcu_read_unlock();
 4) open2-3801782  |   0.933 us    |                  }
 4) open2-3801782  |   0.406 us    |                  __htab_map_lookup_elem();
 4) open2-3801782  |   0.186 us    |                  migrate_enable();
 4) open2-3801782  |   0.162 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   0.168 us    |                  __rcu_read_lock();
 4) open2-3801782  |   0.180 us    |                  __rcu_read_unlock();
 4) open2-3801782  |   7.820 us    |                }
 4) open2-3801782  |   0.240 us    |                __fsnotify_parent();
 4) open2-3801782  | + 11.915 us   |              }
 4) open2-3801782  |               |              ext4_file_open() {
 4) open2-3801782  |   0.266 us    |                ext4_sample_last_mounted();
 4) open2-3801782  |   0.253 us    |                ext4_inode_attach_jinode();
 4) open2-3801782  |               |                dquot_file_open() {
 4) open2-3801782  |   0.186 us    |                  generic_file_open();
 4) open2-3801782  |   0.203 us    |                  __dquot_initialize();
 4) open2-3801782  |   1.107 us    |                }
 4) open2-3801782  |   2.446 us    |              }
 4) open2-3801782  |               |              file_ra_state_init() {
 4) open2-3801782  |   0.176 us    |                inode_to_bdi();
 4) open2-3801782  |   0.543 us    |              }
 4) open2-3801782  |   0.163 us    |              __fsnotify_parent();
 4) open2-3801782  | + 17.806 us   |            }
 4) open2-3801782  | + 18.161 us   |          }
 4) open2-3801782  | + 30.819 us   |        }
 4) open2-3801782  |               |        terminate_walk() {
 4) open2-3801782  |               |          dput() {
 4) open2-3801782  |   0.160 us    |            __cond_resched();
 4) open2-3801782  |   0.163 us    |            __rcu_read_lock();
 4) open2-3801782  |   0.163 us    |            __rcu_read_unlock();
 4) open2-3801782  |   1.129 us    |          }
 4) open2-3801782  |               |          mntput() {
 4) open2-3801782  |               |            mntput_no_expire() {
 4) open2-3801782  |   0.160 us    |              __rcu_read_lock();
 4) open2-3801782  |   0.161 us    |              __rcu_read_unlock();
 4) open2-3801782  |   0.821 us    |            }
 4) open2-3801782  |   1.174 us    |          }
 4) open2-3801782  |   2.815 us    |        }
 4) open2-3801782  | + 89.390 us   |      }
 4) open2-3801782  | + 89.857 us   |    }
 4) open2-3801782  |   0.204 us    |    fd_install();
 4) open2-3801782  |               |    putname() {
 4) open2-3801782  |   0.349 us    |      kmem_cache_free();
 4) open2-3801782  |   0.700 us    |    }
 4) open2-3801782  | ! 102.171 us  |  }
```