#ifndef GZOS_FILEDESCRIPTOR_H
#define GZOS_FILEDESCRIPTOR_H

#include <cstdlib>
#include <cstdint>
#include <sys/stat.h>

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

private:
    size_t size_left(void) {
        return _end - (_start + this->offset);
    }

    uintptr_t _start;
    uintptr_t _end;
};

#endif //GZOS_FILEDESCRIPTOR_H
