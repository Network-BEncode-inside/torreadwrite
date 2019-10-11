/* $Id: torwrite.c 529 2009-09-11 08:44:53Z michael $ */
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const uint8_t sp=' ';
const uint8_t ret='\n';
const uint8_t quo='\"';
const uint8_t scr='\\';
const uint8_t hxp='x';
const uint8_t blst='[';
const uint8_t elst=']';
const uint8_t bdct='(';
const uint8_t edct=')';
const uint8_t eqv='=';

enum obj_type {OBJ_NUL, OBJ_INT, OBJ_STR, OBJ_LST, OBJ_DCT};

size_t IntLen(const uint8_t* d, size_t shift, size_t len)
{
    size_t l = 1;

    while(d[shift + l] >= 48 && d[shift + l] <= 57)
    {
        l++;
        if(shift + l == len) exit(2);
    }
    return l;
}

size_t StrLen(const uint8_t* d, size_t shift, size_t len)
{
    size_t l = 0;
    size_t sh = shift + 1;

    while(1)
    {
        if(d[sh] == quo) break;
        if(d[sh] == scr)
        {
            if(d[sh + 1] == 'x') sh += 3;
            else sh++;
        }
        sh++;
        l++;
        if(sh >= len) exit(2);
    }
    return l;
}

void PrintDec(size_t n)
{
    size_t num = n, rem;
    size_t len = 0;
    uint8_t *out, *pout;

    while(num != 0)
    {
        num /= 10;
        len++;
    }

    out = malloc(len);
    pout = out + len - 1;
    num = n;

    while(num != 0)
    {
        rem = num % 10;
        num /= 10;
        *pout --= rem + '0';
    }
    fwrite(out, len, 1, stdout);
    free(out);
}

uint8_t Hex2N(uint8_t h)
{
    if(h >= '0' && h <= '9') return h - '0';
    if(h >= 'A' && h <= 'F') return h - 'A' + 10;
    if(h >= 'a' && h <= 'f') return h - 'a' + 10;
    exit(2);
}

uint8_t Hex2Sym(const uint8_t *d, size_t sh)
{
    return (Hex2N(d[sh]) << 4) | Hex2N(d[sh + 1]);
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s file.torrent.txt > file.torrent", argv[0]);
        return 1;
    }

    int fd;
    struct stat st;
    uint8_t *pdata;
    uint8_t ch, c;
    size_t shift = 0, len;

    fd=open(argv[1], O_RDONLY);
    if(fd == -1)
    {
        fprintf(stderr, "ERROR: Can't open %s", argv[1]);
        return 1;
    }
    if(fstat(fd, &st) != 0)
    {
        fprintf(stderr, "ERROR: Can't read %s", argv[1]);
        return 1;
    }
    if(st.st_size == 0)
    {
        fprintf(stderr, "ERROR: File %s empty", argv[1]);
        return 1;
    }
    pdata=(uint8_t*) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

    while(shift < st.st_size)
    {
        c = pdata[shift];
        if(c == bdct)
        {
            ch = 'd';
            fwrite(&ch, 1, 1, stdout);
        }
        if(c == blst)
        {
            ch = 'l';
            fwrite(&ch, 1, 1, stdout);
        }
        if(c == edct || pdata[shift] == elst)
        {
            ch = 'e';
            fwrite(&ch, 1, 1, stdout);
        }
        if((c >= 48 && c <= 57) || c == '+' || c == '-')
        {
            len = IntLen(pdata, shift, st.st_size);
            ch = 'i';
            fwrite(&ch, 1, 1, stdout);
            fwrite(pdata + shift, len, 1, stdout);
            ch = 'e';
            fwrite(&ch, 1, 1, stdout);
            shift += len - 1;
        }
        if(c == quo)
        {
            len = StrLen(pdata, shift, st.st_size);
            PrintDec(len);
            ch = ':';
            fwrite(&ch, 1, 1, stdout);
            shift++;
            while(1)
            {
                c = pdata[shift];
                if(c == quo) break;
                if(c == scr)
                {
                    shift++;
                    c = pdata[shift];
                    if(c == 'x')
                    {
                        shift++;
                        ch = Hex2Sym(pdata, shift);
                        shift++;
                    }
                    else
                    {
                        switch(c)
                        {
                        case('n'):
                        {
                            ch = '\n';
                            break;
                        }
                        case('a'):
                        {
                            ch = '\a';
                            break;
                        }
                        case('b'):
                        {
                            ch = '\b';
                            break;
                        }
                        case('e'):
                        {
                            ch = '\e';
                            break;
                        }
                        case('f'):
                        {
                            ch = '\f';
                            break;
                        }
                        case('r'):
                        {
                            ch = '\r';
                            break;
                        }
                        case('t'):
                        {
                            ch = '\t';
                            break;
                        }
                        case('v'):
                        {
                            ch = '\v';
                            break;
                        }
                        default:
                            ch = c;
                        }
                    }
                }
                else ch = c;
                fwrite(&ch, 1, 1, stdout);
                shift++;
            }
        }
        shift++;
    }

    munmap(pdata, st.st_size);
    close(fd);
    return 0;
}
