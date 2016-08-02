#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

int main(int argc, char** argv) {
    int fd; 
    struct stat sf;
    //Open file
    fd = open(argv[1], O_RDONLY, 0);
    fstat(fd, &sf);
    //Execute mmap
    void* mmappedData = mmap(NULL, sf.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    //Write the mmapped data to stdout (= FD #1)
    write(1, mmappedData, sf.st_size);
    close(fd);
}
