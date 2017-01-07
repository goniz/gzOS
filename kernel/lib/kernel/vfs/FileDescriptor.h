#ifndef GZOS_FILEDESCRIPTOR_H
#define GZOS_FILEDESCRIPTOR_H

#include <cstdlib>
#include <cstdint>
#include <sys/stat.h>
#include <vector>
#include <memory>

class FileDescriptor;
class InvalidFileDescriptor;
class NullFileDescriptor;

class FileDescriptor {
public:
    virtual ~FileDescriptor(void) = default;

    virtual int read(void* buffer, size_t size) = 0;
    virtual int write(const void* buffer, size_t size) = 0;
    virtual int seek(int where, int whence) = 0;
    virtual int stat(struct stat *stat) = 0;
    virtual void close(void) = 0;

    virtual int ioctl(int cmd, void* buffer, size_t size);

    int get_offset(void) const;
    bool is_valid(void) const;

protected:
    int offset = 0;
};

class NullFileDescriptor : public FileDescriptor {
public:
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
