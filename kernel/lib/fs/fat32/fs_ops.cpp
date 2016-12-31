#include <platform/drivers.h>
#include <platform/kprintf.h>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <lib/primitives/interrupts_mutex.h>
#include <cstdio>
#include "fs_ops.h"
#include "ff.h"

static int fs_fat32_init(void) {
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("fat32", [](const char *source) {
        return std::unique_ptr<FileSystem>(new Fat32FileSystem(source));
    });

    return 0;
}

DECLARE_DRIVER(fs_fat32, fs_fat32_init, STAGE_SECOND + 1);

Fat32FileSystem::Fat32FileSystem(const char *sourceDevice)
        : _sourceFd()
{
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    _sourceFd = vfs.open(sourceDevice, O_RDONLY);
    if (!_sourceFd) {
        kprintf("Could NOT open file \"%s\"\n", sourceDevice);
        throw std::exception();
    }

    f_mount(&_fs, "PFLASH", 1);
}

class Fat32ReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    Fat32ReaddirFileDescriptor(const char* path)
    {
        auto result = f_opendir(&_dir, path);
        if (FR_OK != result) {
            kprintf("failed to open dir: %d\n", result);
            throw std::exception();
        }
    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        FILINFO finfo;

        memset(&finfo, 0, sizeof(finfo));
        auto result = f_readdir(&_dir, &finfo);
        if (FR_OK != result || finfo.fname[0] == 0) {
            return false;
        }

        strncpy(dirEntry.name, finfo.fname, sizeof(dirEntry.name));
        if (finfo.fattrib & AM_DIR) {
            dirEntry.type = DIRENT_DIR;
        } else {
            dirEntry.type = DIRENT_REG;
        }

        return true;
    }

    DIR _dir;
};

std::unique_ptr<FileDescriptor> Fat32FileSystem::readdir(const char* path) {
    return std::make_unique<Fat32ReaddirFileDescriptor>(path);
}

class Fat32FileDescriptor : public FileDescriptor {
public:
    Fat32FileDescriptor(const char* name, int flags)
        : _filename(name),
          _fp()
    {
        if (FR_OK != f_open(&_fp, name, this->translateFlags(flags))) {
            throw std::exception();
        }
    }

    virtual ~Fat32FileDescriptor(void) override {
        this->close();
    }

    virtual int read(void *buffer, size_t size) override {
        unsigned int bytesRead = 0;
        int result = f_read(&_fp, buffer, size, &bytesRead);
        if (FR_OK == result) {
            return bytesRead;
        }

        return -1;
    }

    virtual int write(const void *buffer, size_t size) override {
#if !defined(_FS_READONLY)
        UINT bytesWritten = 0;
        int result = f_write(&_fp, buffer, size, &bytesWritten);
        if (FR_OK == result) {
            return bytesWritten;
        }
#endif
        return -1;
    }

    virtual int seek(int where, int whence) override {
        if (SEEK_SET != whence) {
            return -1;
        }

        int result = f_lseek(&_fp, (FSIZE_t) where);
        if (FR_OK != result) {
            return -1;
        }

        return where;
    }

    virtual int stat(struct stat *stat) override {
        FILINFO info;
        if (FR_OK != f_stat(_filename.c_str(), &info)) {
            return -1;
        }

        stat->st_size = info.fsize;
        return 0;
    }

    virtual void close(void) override {
        f_close(&_fp);
    }

private:
    uint8_t translateFlags(int flags) const {
        uint8_t result = 0;

        if (flags == O_RDONLY) {
            result = FA_READ;
        } else if (flags == O_WRONLY) {
            result = FA_WRITE;
        } else if (flags == O_RDWR) {
            result = FA_READ | FA_WRITE;
        }

        if (flags & O_CREAT && flags & O_TRUNC) {
            result |= FA_CREATE_ALWAYS;
        }

        return result;
    }

    std::string _filename;
    FIL _fp;
};

std::unique_ptr<FileDescriptor> Fat32FileSystem::open(const char *path, int flags) {
    return std::make_unique<Fat32FileDescriptor>(path, flags);
}