#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <fcntl.h> 
#include <time.h> 
#include <sys/mman.h>
#include <stdint.h>
#include <arpa/inet.h>

#define ENTRY_SIZE 64

// Super block
struct superblock_t {
    uint8_t     fs_id[8];
    uint16_t    blocksize;
    uint32_t    fs_block_count;
    uint32_t    table_start;
    uint32_t    table_block_count;
    uint32_t    root_start_block;
    uint32_t    root_block_count;
} ;
typedef struct superblock_t superblock;

struct entry_time {
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
} ;
typedef struct entry_time entry_time;

struct directory_entry {
    uint8_t     status;
    uint32_t    start_block;
    uint32_t    block_count;
    uint32_t    size;
    char      filename[31];
    uint8_t     unused[6];
    struct entry_time modified_time;
    struct entry_time created_time;
};
typedef struct directory_entry directory_entry;


int getBlockSize(char * mmap)
{
    int *tmp1 = malloc(sizeof(int));
    int *tmp2 = malloc(sizeof(int));
    int retVal;
    * tmp1 = mmap[8];
    * tmp2 = mmap[9];
    retVal = ((*tmp1)<<8)+(*tmp2);
    free(tmp1);
    tmp1 = NULL;
    free(tmp2);
    tmp2 = NULL;
    return retVal;
};


int get_data(char * mmap, int offset, int length)
{
        int i = 0 , retVal = 0;
        unsigned char *tmp = (unsigned char *)malloc(sizeof(unsigned char) * length);
        memcpy(tmp, mmap+offset, length);
 
        for(i=0; i < length; i++){
                retVal += ((int)tmp[i]<<(8*(-i+length - 1)));
                //("%d\n", retVal);
        }

        free(tmp);
        return retVal;
};

int get_data2(char * mmap, int offset, int length){
    char bin; int dec = 0;
    int n = 0;

    for (n=0; n<length; n++) { 
        scanf("%c",&bin); 
        if (bin == '1') dec = dec * 2 + 1; 
        else if (bin == '0') dec *= 2; 
    } 

    printf("%d\n", dec); 

    return dec;
}




int getFATinfo(char * mmap, int fat_start, int fat_len){

    int i = fat_start * 512;
    int available_blocks = 0;
    int reserved_blocks = 0;
    int assigned_blocks = 0;

    int counter = 0;
    int result = 0;
    for(i; i < fat_len*512; i+=4){
        counter ++;
        result = get_data(mmap, i, 4);
        if (result == 0){
            available_blocks ++;
        } else if(result == 1){
            reserved_blocks ++;
        } else{
            assigned_blocks ++;
        }
    }
    printf("\nFAT Information:\n");
    printf("Free Blocks: %d\n", available_blocks);
    printf("Reserved Blocks: %d\n", reserved_blocks);
    printf("Allocated Blocks: %d\n", assigned_blocks);

};

int getsuperinfo(char * p, superblock * super){

    super->blocksize = getBlockSize(p);
    super->fs_block_count = get_data(p, 10, 4);
    super->table_start = get_data(p, 14, 4);
    super->table_block_count = get_data(p, 18, 4);
    super->root_start_block = get_data(p, 22, 4);
    super->root_block_count = get_data(p, 26, 4);

    printf("Super Block Information:\n");
    printf("Block size: %d\n", super->blocksize);
    printf("Block count: %d\n", super->fs_block_count);
    printf("FAT start: %d\n", super->table_start);
    printf("FAT blocks: %d\n", super->table_block_count);
    printf("Root directory start: %d\n", super->root_start_block);
    printf("Root directory blocks: %d\n", super->root_block_count); 

    getFATinfo(p, super->table_start, super->table_block_count);
    return(0);

};


void makefile(directory_entry * entry, char * p, superblock * super, char * arg3){
    int totalblocks = entry->block_count;
    int fullsize = entry->size;
    int offset=0;
    int fat = super->table_start;
    int next_read = entry->start_block;

    //unsigned int *tempp = malloc(sizeof(unsigned int));
    unsigned char *file_content = (unsigned char *)malloc(sizeof(unsigned char) * fullsize+1); 


    do{
        memcpy(file_content+offset, p+(next_read*512), 512);
        printf("oldread:%d\n",next_read);
        next_read = get_data(p, fat*512 + next_read*4, 4);
        printf("next_read: %d \n",next_read );
        totalblocks--;
        offset += 512;
    } while(totalblocks > 1);

    FILE * fp = fopen(arg3, "w+");
    if (fp > 0 && fprintf(fp, "%s", file_content) > 0){
        printf("File written Successfully.\n");
    }
 
    //free(file_content);
    //file_content = NULL;
    
    exit(1);
}


void find_file(char * p, char * arg, int counter, void * voiditem, superblock * super, char * arg3)
{
    const char s[2] = "/";
    char * token;
    char list[10][31];
    int token_count = 0;
    int h;
    int file_found;
    int j;
    int i=0;
    int u=0;
    int next_start=0;
    directory_entry * entry = (directory_entry*)voiditem;

    token = strtok(arg, s);
    while(token != NULL){
        strcpy(list[token_count], token);
        token = strtok(NULL, s);
        token_count++;
    }

    // for as many subdirs in input file desired, check the directory entries for match.
    // if there is a match and more subdirs in input, read next directory and loop.
    for(j=0; j<c; j++){

        file_found = 0;
        next_start=0;
        for(h=0; h<counter; h++){
            if(strcmp(list[j], entry[h].filename)==0){
                //If file found...
                file_found = 1;
                next_start = h;
                printf("Found %s.\n", list[j]);
                printf("Total of %d blocks\n", entry[h].block_count);
                if(c-j == 1){
                    makefile(&entry[h], p, super, arg3);
                }
            } 
        }

        if (file_found==0){
            printf("\nFile Not Found. Please restart program with another filepath.\n");
            break;
        }


        printf("\nContents of %s:\n", entry[next_start].filename);
        for(u = 512*entry[next_start].start_block; u<512*(entry[next_start].start_block+entry[next_start].block_count); u+=ENTRY_SIZE){
            counter = read_directory(p, &entry[counter], entry[next_start].start_block, i, counter);
            i++;
        }

    }

}



// takes mmap, file pointer, directory entry array, start of directory, length of directory,
// number of directory blocks read so far in dir, num entries in directory_entries array.
int read_directory(char * p, void * voidentry, int root_start,  int counter, int num_entries)
{
    int i = root_start*super.blocksize;
    int parent = i;
    int base = 0;

    int place = 0;
    int k = 0;
    int result = 0;
    directory_entry *entry = (directory_entry *)voidentry;
    unsigned char *file_size_bytes = (unsigned char *)malloc(sizeof(unsigned char) * 4); 


    // Get basic numbers for status, start and count
    base = parent + 64*(counter);    
    entry->status = get_data(p, base, 1);
    entry->start_block = get_data(p, base+1, 4);
    entry->block_count = get_data(p, base+5, 4);


    //entry->size = file_size;
    entry->size = get_data(p, base+9, 4);

    //Get file name
    memcpy(entry->filename, p + base + 27, 31);

    //directory read, so increment counter
    counter++;

    //print file info
    if(strcmp(entry->filename, "No_file") != 0 && entry->size != 0){
        num_entries++;
        if(entry->status<4){
            printf("F %10d %30s\n", entry->size, entry->filename);
        } else
            printf("D %10d %30s\n", entry->size, entry->filename);
    }
    free(file_size_bytes);
    file_size_bytes = NULL;

    return num_entries;
}


 
int main(int argc, char* argv[]) 
{ 
    int fd = 0; 
    struct stat st; 
    char * p;
    superblock super;
    fd = open(argv[1], O_RDONLY); 
 
    //Check if open() was successful 
    if(-1 == fd) 
    { 
        printf("\n NULL File descriptor\n"); 
        return -1; 
    } 
 

    // set the errno to default value 
    errno = 0; 
    // the address of struct stat object is passed as the second argument 
    if(fstat(fd, &st)) 
    { 
        printf("\nfstat error: [%s]\n",strerror(errno)); 
        close(fd); 
        return -1; 
    } 

    // prints/gets info about disk, puts it into super.
    p = mmap(NULL,st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    getsuperinfo(p, &super);


    // Reads entire directory 
    directory_entry entries[128];
    int ceiling = super.blocksize*(super.root_block_count + super.root_start_block);
    //keeps track of directories read
    int dir_counter = 0;
    int u;
    //num=entry index
    int num=0;
    printf("\nContents of root:\n");
    for(u = 512*super.root_start_block; u<ceiling; u+=ENTRY_SIZE){
        num = read_directory(p, &entries[num], super.root_start_block, dir_counter, num);
        dir_counter++;

    }

    find_file(p, argv[2], num, &entries, &super, argv[3]);
    //Execute mmap
    void* mmappedData = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    //Write the mmapped data to stdout (= FD #1)
   // write(1, mmappedData, st.st_size);
    close(fd);
 
    return 0; 
}
