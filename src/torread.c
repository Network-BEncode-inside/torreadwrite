/* $Id: torread.c 529 2009-09-11 08:44:53Z michael $ */
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

void Byte2Hex(uint8_t ch);
void PrintSpc(int num);
enum obj_type DetType(const uint8_t* d, size_t shift);
size_t FindSym(const uint8_t* d, uint8_t s, size_t shift, size_t len);
size_t PrintInt(const uint8_t* d, size_t shift, size_t len);
size_t PrintStr(const uint8_t* d, size_t shift, size_t len);
size_t PrintLst(const uint8_t* d, size_t shift, size_t len, int skip);
size_t PrintDct(const uint8_t* d, size_t shift, size_t len, int skip);
size_t PrintObj(const uint8_t* d, size_t shift, size_t len, int skip);

void Byte2Hex(uint8_t ch)
{
    static const uint8_t cnv[]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    fwrite(cnv + (( ch & (15 << 4)) >> 4), 1, 1, stdout);
    fwrite(cnv + (ch & 15), 1, 1, stdout);
}

void PrintSpc(int num)
{
    int i;
    for(i = 0; i < num; i++) fwrite(&sp, 1, 1, stdout);
}

enum obj_type DetType(const uint8_t* d, size_t shift)
{
// write(2,d+shift,1);
    if(d[shift] == 'i') return OBJ_INT;
    if(d[shift] == 'l') return OBJ_LST;
    if(d[shift] == 'd') return OBJ_DCT;
    if(d[shift] >= 49 && d[shift] <= 57) return OBJ_STR;
    return OBJ_NUL;
}

size_t FindSym(const uint8_t* d, uint8_t s, size_t shift, size_t len)
{
    size_t i;

    for(i = shift; i < len; i++) if(d[i] == s) return i;
    exit(2);
}

size_t PrintObj(const uint8_t* d, size_t shift, size_t len, int skip)
{
    switch(DetType(d, shift))
    {
    case(OBJ_NUL):
        exit(2);
    case(OBJ_INT):
        return PrintInt(d, shift, len);
    case(OBJ_STR):
        return PrintStr(d, shift, len);
    case(OBJ_LST):
        return PrintLst(d, shift, len, skip);
    case(OBJ_DCT):
        return PrintDct(d, shift, len, skip);
    }
    exit(2);
}

size_t PrintInt(const uint8_t* d, size_t shift, size_t len)
{
    size_t e = FindSym(d, 'e', shift, len);
    fwrite(d + shift + 1, e - shift - 1, 1, stdout);
    return e+1;
}

size_t PrintStr(const uint8_t* d, size_t shift, size_t len)
{
    size_t e = FindSym(d, ':', shift, len);
    int l = atoi((const char*)(d + shift));
    if(e + l >= len) exit(2);
    size_t p;

    fwrite(&quo, 1, 1, stdout);
    for(p = e + 1; p <= e + l; p++)
    {
        if(d[p] == 127 || d[p] < 32)
        {
            fwrite(&scr, 1, 1, stdout);
            fwrite(&hxp, 1, 1, stdout);
            Byte2Hex(d[p]);
        }
        else if(d[p] == '\\')
        {
            fwrite(&scr, 1, 1, stdout);
            fwrite(&scr, 1, 1, stdout);
        }
        else if(d[p] == '\"')
        {
            fwrite(&scr, 1, 1, stdout);
            fwrite(&quo, 1, 1, stdout);
        }
        else fwrite(d + p, 1, 1, stdout);
    }

    fwrite(&quo, 1, 1, stdout);
    return e + l + 1;
}

size_t PrintLst(const uint8_t* d, size_t shift, size_t len, int skip)
{
    size_t ishift=shift+1;

    fwrite(&blst, 1, 1, stdout);
    fwrite(&ret, 1, 1, stdout);

    while(d[ishift] != 'e')
    {
        PrintSpc(skip + 1);
        ishift = PrintObj(d, ishift, len, skip + 1);
        fwrite(&ret, 1, 1, stdout);
        if(ishift >= len) exit(2);
    }
    PrintSpc(skip);
    fwrite(&elst, 1, 1, stdout);
    return ishift + 1;
}

size_t PrintDct(const uint8_t* d, size_t shift, size_t len, int skip)
{
    size_t ishift = shift + 1;

    fwrite(&bdct, 1, 1, stdout);
    fwrite(&ret, 1, 1, stdout);

    while(d[ishift] != 'e')
    {
        PrintSpc(skip + 1);
        if(DetType(d, ishift) != OBJ_STR) exit(2);
        ishift = PrintStr(d, ishift, len);
        fwrite(&sp, 1, 1, stdout);
        fwrite(&eqv, 1, 1, stdout);
        fwrite(&sp, 1, 1, stdout);
        ishift = PrintObj(d, ishift, len, skip + 1);
        fwrite(&ret, 1, 1, stdout);
        if(ishift >= len) exit(2);
    }
    PrintSpc(skip);
    fwrite(&edct, 1, 1, stdout);
    return ishift + 1;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s file.torrent > file.torrent.txt", argv[0]);
        return 1;
    }

    int fd;
    struct stat st;
    uint8_t *pdata;

    fd = open(argv[1],O_RDONLY);
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
    pdata = (uint8_t*) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    PrintDct(pdata, 0, st.st_size, 0);
    fwrite(&ret, 1, 1, stdout);
    munmap(pdata, st.st_size);
    close(fd);

    return 0;
}
