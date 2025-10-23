/*
 * An example using splice syscall which avoids copying to/from user space buffers to kernel space
 * and uses the pipe buffers allocated in kernel space as an intermediate to directly xfer from one file to another
 *
 * gcc -o splice splice.c -g
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define __IO_BUFSIZE (4096) /*reasonable buffer size based on pipe buffers*/

#ifdef HAVE_SPLICE
#define do_copy spliced_copy
#else
#define do_copy std_copy
#endif

/*
 * Read from in and write to out without using user space buffers.
 * Directly splice using the pipe buffer.
 */
#ifdef HAVE_SPLICE
static int spliced_copy(int in_fd, int out_fd)
{
    loff_t in_off = 0;
    loff_t out_off = 0;
    static int buf_size = __IO_BUFSIZE;
    off_t len;
    int filedes[2];
    int err = -1;
    struct stat stbuf;
    if(pipe(filedes) < 0)
    {
        perror("pipe:");
        goto out;
    }
    if(fstat(in_fd, &stbuf) < 0)
    {
        perror("fstat:");
        goto out_close;
    }
    len = stbuf.st_size;
    while(len > 0)
    {
        if(buf_size > len) buf_size = len;
        /*
         * move to pipe buffer.
         */
        err = splice(in_fd, &in_off, filedes[1], NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if(err < 0)
        {
            perror("splice:");
            goto out_close;
        }
        /*
         * move from pipe buffer to out_fd
         */
        err = splice(filedes[0], NULL, out_fd, &out_off, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if(err < 0)
        {
            perror("splice2:");
            goto out_close;
        }
        len -= buf_size;
    }
    err = 0;

    out_close:
    close(filedes[0]);
    close(filedes[1]);
    
    out:
    return err;
}
#else
static int std_copy(int in_fd, int out_fd)
{
    char buf[__IO_BUFSIZE];
    int err = -1;
    int bytes;
    while( (bytes = read(in_fd, buf, sizeof(buf))) > 0 )
    {
        if(write(out_fd, buf, bytes) != bytes)
        {
            perror("write:");
            goto out;
        }
    }
    err = 0;
    out:
    return err;
}
#endif

int main(int argc, char **argv)
{
    char infile[0xff+1], outfile[0xff+1];
    int in_fd = -1, out_fd = -1;
    int err = -1;

    if(argc != 3) 
    {
        fprintf(stderr, "%s infile outfile\n", argv[0]);
        goto out;
    }

    infile[0] = 0, outfile[0] = 0;
    strncat(infile, argv[1], sizeof(infile)-1);
    strncat(outfile, argv[2], sizeof(outfile)-1);
    in_fd = open(infile, O_RDONLY);
    if(in_fd < 0)
    {
        perror("open:");
        goto out;
    }
    out_fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if(out_fd < 0)
    {
        perror("open2:");
        goto out_close;
    }
    if((err = do_copy(in_fd, out_fd)) < 0)
    {
        printf("Error copying input file [%s] to output file [%s]\n", infile, outfile);
    }

    close(out_fd);

    out_close:
    close(in_fd);

    out:
    return err;
}
