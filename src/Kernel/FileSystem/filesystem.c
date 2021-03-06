#include "BalrogOS/FileSystem/filesystem.h"
#include "BalrogOS/FileSystem/fs_cache.h"
#include "BalrogOS/Drivers/disk/ata.h"
#include "BalrogOS/FileSystem/ext2/ext2.h"
#include "BalrogOS/Memory/memory.h"
#include "BalrogOS/Memory/kheap.h"
#include "BalrogOS/Memory/vmm.h"
#include "BalrogOS/Memory/pmm.h"
#include "BalrogOS/Tasking/elf/elf.h"
#include <string.h>
#include "lib/IO/kprint.h"

fs_device dev;

int fs_get_file(const char* name, fs_file* file)
{
    fs_fd fd;
    dev.fs->open(&dev, name, &fd);
    fs_file* tmp = fs_cache_get_file(fd.ftable_idx);
    *file = *tmp;
    return 0;
}

int fs_open(const char* name, fs_fd* fd)
{
    pthread_mutex_lock(&dev.lock);
    int ret = dev.fs->open(&dev, name, fd);
    pthread_mutex_unlock(&dev.lock);
    return ret;
}

int fs_read(uint8_t* buffer, uint64_t len, fs_fd* fd)
{
    pthread_mutex_lock(&dev.lock);
    int ret = dev.fs->read(&dev, buffer, len, fd);
    pthread_mutex_unlock(&dev.lock);
    return ret;
}

int fs_close(fs_fd* fd)
{
    pthread_mutex_lock(&dev.lock);
    int ret = dev.fs->close(&dev, fd);
    pthread_mutex_unlock(&dev.lock);
    return ret;
}

int fs_fstat(fs_fd* fd, fs_file_stat* stat)
{
    pthread_mutex_lock(&dev.lock);
    int ret = dev.fs->stat(&dev, fd, stat);
    pthread_mutex_unlock(&dev.lock);
    return ret;
}

void init_file_system()
{
    init_ata();
    pthread_mutex_init(&dev.lock, NULL);
    pthread_mutex_lock(&dev.lock);
    ata_get_boot_device(&dev);
    ext2_probe(&dev);
    pthread_mutex_unlock(&dev.lock);
}