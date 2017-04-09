#ifndef GZOS_CPIOEXTRACTOR_H
#define GZOS_CPIOEXTRACTOR_H

#ifdef __cplusplus

#include <cstdint>
#include <cstddef>
#include <sched.h>
#include <string>

struct cpio_newc_header {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
};

enum class CpioMode {
    Directory = 0040000,
    RegularFile = 0100000
};

struct CpioFile {
    mode_t mode;
    int uid;
    int gid;
    time_t mtime;
    size_t name_size;
    std::string name;
    uint8_t* file_data;
    size_t file_size;
    uint32_t checksum;
    void* nextPosition;
};


class CpioExtractor
{
public:
    CpioExtractor(void* startAddress, unsigned long size);

    bool extract(const char* outputRootPath);

private:
    uint8_t* _archiveStart;
    size_t _archiveSize;

    CpioFile parseCpioFile(cpio_newc_header* header);

    uint32_t parseAsciiUnsignedInt(const char* value, size_t valueSize);

    void writeFile(const char* basePath, const CpioFile& file);

    void createDirectory(const char* basePath, const char* filename);
};


#endif //cplusplus
#endif //GZOS_CPIOEXTRACTOR_H
