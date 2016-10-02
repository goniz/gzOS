#ifndef GZOS_FILEDESCRIPTOR_H
#define GZOS_FILEDESCRIPTOR_H

#include <cstdlib>

class FileDescriptor;
class InvalidFileDescriptor;
class NullFileDescriptor;

class FileDescriptor {
public:
    virtual ~FileDescriptor() {
        this->close();
    }

    virtual int read(void* buffer, size_t size) = 0;
    virtual int write(const void* buffer, size_t size) = 0;
    virtual int seek(int where, int whence) = 0;
    virtual void close(void) { }

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
    virtual void close(void) override;
};

#endif //GZOS_FILEDESCRIPTOR_H
