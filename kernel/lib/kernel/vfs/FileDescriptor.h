#ifndef GZOS_FILEDESCRIPTOR_H
#define GZOS_FILEDESCRIPTOR_H

#include <cstdlib>
#include <cstdint>
#include <sys/stat.h>
#include <vector>
#include <memory>
#include <platform/kprintf.h>

class FileDescriptor;
class InvalidFileDescriptor;
class NullFileDescriptor;

using UniqueFd = std::unique_ptr<FileDescriptor>;
class FileDescriptor {
public:
    enum class IoctlCommands : int {
        GetBlocking = 0,
        SetBlocking = 1
    };

    FileDescriptor(void) = default;
    virtual ~FileDescriptor(void) {
        this->close();
    }

    virtual const char* type(void) const = 0;

    virtual int read(void* buffer, size_t size) = 0;
    virtual int write(const void* buffer, size_t size) = 0;
    virtual int seek(int where, int whence) = 0;
    virtual int stat(struct stat *stat) = 0;
    virtual int poll(bool* read_ready, bool* write_ready) { return -1; }
    virtual int set_blocking(int blocking_state) { return 0; }
    virtual void close(void) { }

    int ioctl(int cmd, ...);
    virtual int ioctl(int cmd, va_list args);

    int get_offset(void) const;
    bool is_valid(void) const;

protected:
    int offset = 0;
    int blocking_state = 1;
};

class DuplicatedFileDescriptor : public FileDescriptor {
public:
    DuplicatedFileDescriptor(std::unique_ptr<FileDescriptor>& dupFd);
    virtual ~DuplicatedFileDescriptor() = default;
    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual int poll(bool* read_ready, bool* write_ready) override;
    virtual void close(void) override;

private:
    std::unique_ptr<FileDescriptor>& _dupFd;
};

class NullFileDescriptor : public FileDescriptor {
public:
    virtual ~NullFileDescriptor() = default;
    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;
};


class InvalidFileDescriptor : public FileDescriptor {
public:
    InvalidFileDescriptor(void) {
        this->offset = -1;
    }

    virtual ~InvalidFileDescriptor(void) = default;

    virtual int read(void *buffer, size_t size) override;
    virtual int write(const void *buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;
};

class MemoryBackedFileDescriptor : public FileDescriptor {
public:
    MemoryBackedFileDescriptor(uintptr_t start, uintptr_t end);
    virtual ~MemoryBackedFileDescriptor() = default;

    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;

protected:
    size_t size_left(void) {
        return _end - (_start + this->offset);
    }

    uintptr_t _start;
    uintptr_t _end;
};

class VectorBackedFileDescriptor : public FileDescriptor
{
public:
    VectorBackedFileDescriptor(std::shared_ptr<std::vector<uint8_t>> vector);
    virtual ~VectorBackedFileDescriptor(void) = default;

    virtual const char* type(void) const override;
    virtual int read(void* buffer, size_t size) override;
    virtual int write(const void* buffer, size_t size) override;
    virtual int seek(int where, int whence) override;
    virtual int stat(struct stat *stat) override;
    virtual void close(void) override;

private:
    size_t size_left(void);

    std::shared_ptr<std::vector<uint8_t>> _data;
};

#endif //GZOS_FILEDESCRIPTOR_H
