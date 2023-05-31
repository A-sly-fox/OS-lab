#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."
#define OS_SIZE_LOC1 0x1fc
#define OS_SIZE_LOC2 0x1fa


/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp);
static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr);
static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *first);
static void write_os_size(int nbytes, FILE * img, int first);

int main(int argc, char **argv)
{
    char *progname = argv[0];

    /* process command line options */
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
        char *option = &argv[1][2];

        if (strcmp(option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp(option, "extended") == 0) {
            options.extended = 1;
        } else {
            error("%s: invalid option\nusage: %s %s\n", progname,
                  progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
        error("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error("usage: %s %s\n", progname, ARGS);
    }
    create_image(argc - 1, argv + 1);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    int ph, nbytes = 0, first = 0, lastbytes = 0, increasebytes = 0;
    FILE *fp, *img;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    /* open the image file */
    img = fopen("image","wb");
    /* for each input file */
    while (nfiles-- > 0) {

        /* open input file */
        fp = fopen(*files, "rb");
        /* read ELF header */
        read_ehdr(&ehdr, fp);
        printf("0x%04lx: %s\n", ehdr.e_entry, *files);

        /* for each program header */
        for (ph = 0; ph < ehdr.e_phnum; ph++) {

            /* read program header */
            read_phdr(&phdr, fp, ph, ehdr);

            /* write segment to the image */
            write_segment(ehdr, phdr, fp, img, &nbytes, &first);
        }
        increasebytes = nbytes - lastbytes;
        if(first && options.extended)
            printf("os_size: %d sectors\n" ,increasebytes / 512);
        lastbytes = nbytes;
        write_os_size(increasebytes, img, first);
        first++;
        fclose(fp);
        files++;
    }
    
    fclose(img);
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
    fread(ehdr, sizeof(Elf64_Ehdr), 1, fp);
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{   
    if(options.extended){
        printf("\tsegment %d\n",ph);
    }
    fseek(fp, ehdr.e_phoff + ehdr.e_phentsize*ph, SEEK_SET);
    fread(phdr, ehdr.e_phentsize, 1, fp);
}

static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *first)
{
    int size = (!phdr.p_memsz%512)? phdr.p_memsz : (phdr.p_memsz-phdr.p_memsz%512+512);
    char a[size];
    fseek(fp, phdr.p_offset, SEEK_SET);
    fread(a, phdr.p_filesz, 1, fp);
    for(int i = phdr.p_filesz; i < size; i++)
        a[i] = 0;
    fseek(img, *nbytes, SEEK_SET);
    fwrite(a, size, sizeof(char), img);
    *nbytes += size;
    if(options.extended){
        printf("\t\toffset 0x%lx\tvaddr 0x%lx\n",phdr.p_offset,phdr.p_vaddr);
        printf("\t\tfilesz 0x%lx\tmemsz 0x%lx\n",phdr.p_filesz,phdr.p_memsz);
        if(size){
            printf("\t\twriting 0x%lx bytes\n",phdr.p_filesz);
            printf("\t\tpadding up to 0x%x bytes\n",*nbytes);
        }
    }
}

static void write_os_size(int nbytes, FILE * img, int first)
{
    nbytes = nbytes / 512;
    int location = (first == 1) ? OS_SIZE_LOC1 :OS_SIZE_LOC2;
    fseek(img, location, SEEK_SET);
    fwrite(&nbytes, 2, 1, img);
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}
