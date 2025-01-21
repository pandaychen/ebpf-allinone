#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct linux_dirent {
	long           d_ino;
	off_t          d_off;
	unsigned short d_reclen;
	char           d_name[];
};

#define BUF_SIZE 33

/*
1、BUF_SIZE的值会影响输出（BUF_SIZE=33）

0 loop..--------------- nread=32 ---------------
i-node#  file type  d_reclen  d_off   d_name
  541933  directory    32 1970467830156897535  sysdig
1 loop..--------------- nread=32 ---------------
i-node#  file type  d_reclen  d_off   d_name
  674615  directory    32 2278887354544302234  wazuh-agent
2 loop..--------------- nread=32 ---------------
i-node#  file type  d_reclen  d_off   d_name
  661458  directory    32 3695880606158561910  notify
3 loop..--------------- nread=32 ---------------
i-node#  file type  d_reclen  d_off   d_name
  527846  directory    32 4860058639032155676  deepflow
4 loop..--------------- nread=24 ---------------
i-node#  file type  d_reclen  d_off   d_name
      30  directory    24 4897947189046076029  .
getdents: Invalid argument
5 loop..


2、BUF_SIZE = 128

[root@VM-X-X-centos tmp]# ./1
0 loop..--------------- nread=128 ---------------
i-node#  file type  d_reclen  d_off   d_name
  541933  directory    32 1970467830156897535  sysdig
  674615  directory    32 2278887354544302234  wazuh-agent
  661458  directory    32 3695880606158561910  notify
  527846  directory    32 4860058639032155676  deepflow
1 loop..--------------- nread=120 ---------------
i-node#  file type  d_reclen  d_off   d_name
      30  directory    24 4897947189046076029  .
  939290  directory    40 6087992280207792028  hsperfdata_root
     771  regular      32 7078790303714470523  getdents64.c
       2  directory    24 7230312651263208905  ..
2 loop..--------------- nread=56 ---------------
i-node#  file type  d_reclen  d_off   d_name
  524473  directory    24 7307003968302152876  libs
  656692  directory    32 7373424052242438578  rubish
3 loop..--------------- nread=120 ---------------
i-node#  file type  d_reclen  d_off   d_name
  524321  directory    96 7900318765878367783  systemd-private-70396c85cc3d4bc89420a5dd76153a40-chronyd.service-s6COth
     779  regular      24 9223372036854775807  1
4 loop..

3、BUF_SIZE=1024
[root@VM-X-X-centos tmp]# ./1
0 loop..--------------- nread=424 ---------------
i-node#  file type  d_reclen  d_off   d_name
  541933  directory    32 1970467830156897535  sysdig
  674615  directory    32 2278887354544302234  wazuh-agent
  661458  directory    32 3695880606158561910  notify
  527846  directory    32 4860058639032155676  deepflow
      30  directory    24 4897947189046076029  .
  939290  directory    40 6087992280207792028  hsperfdata_root
     771  regular      32 7078790303714470523  getdents64.c
       2  directory    24 7230312651263208905  ..
  524473  directory    24 7307003968302152876  libs
  656692  directory    32 7373424052242438578  rubish
  524321  directory    96 7900318765878367783  systemd-private-70396c85cc3d4bc89420a5dd76153a40-chronyd.service-s6COth
     779  regular      24 9223372036854775807  1
1 loop..

4、BUF_SIZE=31

getdents: Invalid argument
0 loop..

*/

int main(int argc, char **argv)
{
	int fd, nread;
	char buf[BUF_SIZE];
	struct linux_dirent *d;
	int bpos;
	char d_type;

	fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
	if(fd == -1)
		handle_error("open");
	int index=0;
	for( ; ; ) {
		printf("%d loop..",index);
		index++;
		nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
		if(nread == -1)
			handle_error("getdents");

		if(nread == 0)
			break;

		printf("--------------- nread=%d ---------------\n", nread);
		printf("i-node#  file type  d_reclen  d_off   d_name\n");

		for(bpos=0; bpos<nread; ) {
			d = (struct linux_dirent *)(buf+bpos);
			printf("%8ld  ", d->d_ino);
			d_type = *(buf + bpos + d->d_reclen - 1);
			printf("%-10s ", (d_type == DT_REG) ?  "regular" :
                             (d_type == DT_DIR) ?  "directory" :
							 (d_type == DT_FIFO) ? "FIFO" :
							 (d_type == DT_SOCK) ? "socket" :
							 (d_type == DT_LNK) ?  "symlink" :
							 (d_type == DT_BLK) ?  "block dev" :
							 (d_type == DT_CHR) ?  "char dev" : "???");
            printf("%4d %10lld  %s\n", d->d_reclen,
						(long long) d->d_off, d->d_name);
            bpos += d->d_reclen;
		}
	}

	return 0;
}
