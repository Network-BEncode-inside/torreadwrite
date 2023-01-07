/* $Id: torwrite.c 529 2009-09-11 08:44:53Z michael $ */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

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

void PrintDec(size_t n, FILE* fdo)
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
    fwrite(out, len, 1, fdo);
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
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s file.txt file.torrent\n", argv[0]);
        return 1;
    }

    FILE *fd, *fdo;
    uint8_t *pdata;
    uint8_t ch, c;
    size_t flen = 0, shift = 0, len;

    fd = fopen(argv[1],"rb");
    if(fd == NULL)
    {
        fprintf(stderr, "ERROR: Can't open %s", argv[1]);
        return 1;
    }
    fdo = fopen(argv[2],"wb");
    if(fdo == NULL)
    {
        fprintf(stderr, "ERROR: Can't write %s", argv[2]);
        return 1;
    }

    fseek(fd,0l,SEEK_END);
    flen = ftell(fd);
    if(flen == 0)
    {
        fprintf(stderr, "ERROR: File %s empty", argv[1]);
        return 1;
    }
    if (flen > 16777216)
    {
        printf("Torrent metadata file %s is %ld bytes long.\n",argv[1],flen);
        printf("That is unusually large for a torrent file. You may have specified an\n");
        printf("incorrect file. The metadata must be loaded into memory, so this may\n");
        printf("take some time or fail due to lack of memory.\n");
        printf("\n");
    }
    rewind(fd);

    pdata = malloc(flen);
    if (pdata == NULL)
    {
        printf("Unable to malloc %ld bytes for torrent file\n",flen);
        return 2;
    }
    flen = fread(pdata,1,flen,fd);
    if(flen == 0)
    {
        fprintf(stderr, "ERROR: File %s empty", argv[1]);
        return 1;
    }

    while(shift < flen)
    {
        c = pdata[shift];
        if(c == bdct)
        {
            ch = 'd';
            fwrite(&ch, 1, 1, fdo);
        }
        if(c == blst)
        {
            ch = 'l';
            fwrite(&ch, 1, 1, fdo);
        }
        if(c == edct || pdata[shift] == elst)
        {
            ch = 'e';
            fwrite(&ch, 1, 1, fdo);
        }
        if((c >= 48 && c <= 57) || c == '+' || c == '-')
        {
            len = IntLen(pdata, shift, flen);
            ch = 'i';
            fwrite(&ch, 1, 1, fdo);
            fwrite(pdata + shift, len, 1, fdo);
            ch = 'e';
            fwrite(&ch, 1, 1, fdo);
            shift += len - 1;
        }
        if(c == quo)
        {
            len = StrLen(pdata, shift, flen);
            PrintDec(len, fdo);
            ch = ':';
            fwrite(&ch, 1, 1, fdo);
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
                fwrite(&ch, 1, 1, fdo);
                shift++;
            }
        }
        shift++;
    }

    free(pdata);
    fclose(fd);
    fclose(fdo);

    return 0;
}
