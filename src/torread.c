/* $Id: torread.c 529 2009-09-11 08:44:53Z michael $ */
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

void Byte2Hex(uint8_t ch,FILE* fdo);
void PrintSpc(int num,FILE* fdo);
enum obj_type DetType(const uint8_t* d, size_t shift);
size_t FindSym(const uint8_t* d, uint8_t s, size_t shift, size_t len);
size_t PrintInt(const uint8_t* d, size_t shift, size_t len,FILE* fdo);
size_t PrintStr(const uint8_t* d, size_t shift, size_t len,FILE* fdo);
size_t PrintLst(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo);
size_t PrintDct(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo);
size_t PrintObj(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo);

void Byte2Hex(uint8_t ch,FILE* fdo)
{
    static const uint8_t cnv[]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    fwrite(cnv + (( ch & (15 << 4)) >> 4), 1, 1, fdo);
    fwrite(cnv + (ch & 15), 1, 1, fdo);
}

void PrintSpc(int num,FILE* fdo)
{
    int i;
    for(i = 0; i < num; i++) fwrite(&sp, 1, 1, fdo);
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

size_t PrintObj(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo)
{
    switch(DetType(d, shift))
    {
    case(OBJ_NUL):
        exit(2);
    case(OBJ_INT):
        return PrintInt(d, shift, len, fdo);
    case(OBJ_STR):
        return PrintStr(d, shift, len, fdo);
    case(OBJ_LST):
        return PrintLst(d, shift, len, skip, fdo);
    case(OBJ_DCT):
        return PrintDct(d, shift, len, skip, fdo);
    }
    exit(2);
}

size_t PrintInt(const uint8_t* d, size_t shift, size_t len,FILE* fdo)
{
    size_t e = FindSym(d, 'e', shift, len);
    fwrite(d + shift + 1, e - shift - 1, 1, fdo);
    return e+1;
}

size_t PrintStr(const uint8_t* d, size_t shift, size_t len,FILE* fdo)
{
    size_t e = FindSym(d, ':', shift, len);
    int l = atoi((const char*)(d + shift));
    if(e + l >= len) exit(2);
    size_t p;

    fwrite(&quo, 1, 1, fdo);
    for(p = e + 1; p <= e + l; p++)
    {
        if(d[p] == 127 || d[p] < 32)
        {
            fwrite(&scr, 1, 1, fdo);
            fwrite(&hxp, 1, 1, fdo);
            Byte2Hex(d[p], fdo);
        }
        else if(d[p] == '\\')
        {
            fwrite(&scr, 1, 1, fdo);
            fwrite(&scr, 1, 1, fdo);
        }
        else if(d[p] == '\"')
        {
            fwrite(&scr, 1, 1, fdo);
            fwrite(&quo, 1, 1, fdo);
        }
        else fwrite(d + p, 1, 1, fdo);
    }

    fwrite(&quo, 1, 1, fdo);
    return e + l + 1;
}

size_t PrintLst(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo)
{
    size_t ishift=shift+1;

    fwrite(&blst, 1, 1, fdo);
    fwrite(&ret, 1, 1, fdo);

    while(d[ishift] != 'e')
    {
        PrintSpc(skip + 1, fdo);
        ishift = PrintObj(d, ishift, len, skip + 1, fdo);
        fwrite(&ret, 1, 1, fdo);
        if(ishift >= len) exit(2);
    }
    PrintSpc(skip, fdo);
    fwrite(&elst, 1, 1, fdo);
    return ishift + 1;
}

size_t PrintDct(const uint8_t* d, size_t shift, size_t len, int skip,FILE* fdo)
{
    size_t ishift = shift + 1;

    fwrite(&bdct, 1, 1, fdo);
    fwrite(&ret, 1, 1, fdo);

    while(d[ishift] != 'e')
    {
        PrintSpc(skip + 1, fdo);
        if(DetType(d, ishift) != OBJ_STR) exit(2);
        ishift = PrintStr(d, ishift, len, fdo);
        fwrite(&sp, 1, 1, fdo);
        fwrite(&eqv, 1, 1, fdo);
        fwrite(&sp, 1, 1, fdo);
        ishift = PrintObj(d, ishift, len, skip + 1, fdo);
        fwrite(&ret, 1, 1, fdo);
        if(ishift >= len) exit(2);
    }
    PrintSpc(skip, fdo);
    fwrite(&edct, 1, 1, fdo);
    return ishift + 1;
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s file.torrent file.txt\n", argv[0]);
        return 1;
    }

    FILE *fd, *fdo;
    uint8_t *pdata;
    size_t flen = 0;

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

    PrintDct(pdata, 0, flen, 0, fdo);
    fwrite(&ret, 1, 1, fdo);
    free(pdata);
    fclose(fd);
    fclose(fdo);

    return 0;
}
