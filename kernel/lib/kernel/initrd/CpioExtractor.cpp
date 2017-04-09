#include <cstring>
#include <cstdlib>
#include <lib/primitives/align.h>
#include <vfs/vfs_api.h>
#include <vfs/Path.h>
#include "CpioExtractor.h"

CpioExtractor::CpioExtractor(void* startAddress, unsigned long size)
    : _archiveStart((uint8_t*) startAddress),
      _archiveSize(size)
{

}

bool CpioExtractor::extract(const char* outputRootPath)
{
    cpio_newc_header* pos = (cpio_newc_header*) _archiveStart;

    do {
        if (0 != memcmp(pos->c_magic, "070702", sizeof(pos->c_magic))) {
            return false;
        }

        CpioFile cpioFile = this->parseCpioFile(pos);
        if ("TRAILER!!!" == cpioFile.name || "." == cpioFile.name) {
            break;
        }

        if (cpioFile.mode & (int)CpioMode::Directory) {
            this->createDirectory(outputRootPath, cpioFile.name.c_str());
        } else {
            this->writeFile(outputRootPath, cpioFile);
        }

        pos = (cpio_newc_header*) cpioFile.nextPosition;

    } while ((uint8_t*)pos < (_archiveStart + _archiveSize));

    return true;
}

CpioFile CpioExtractor::parseCpioFile(cpio_newc_header* header) {
    CpioFile file;

    file.mode = this->parseAsciiUnsignedInt(header->c_mode, sizeof(header->c_mode));
    file.file_size = this->parseAsciiUnsignedInt(header->c_filesize, sizeof(header->c_filesize));
    file.name_size = this->parseAsciiUnsignedInt(header->c_namesize, sizeof(header->c_namesize));
    file.name = std::string((const char*)(header) + sizeof(cpio_newc_header), 0, file.name_size);
    file.file_data = align((uint8_t*)(header) + sizeof(cpio_newc_header) + file.name_size, 4);
    file.nextPosition = align(file.file_data + file.file_size, 4);

    return file;
}

uint32_t CpioExtractor::parseAsciiUnsignedInt(const char* value, size_t valueSize) {
    char buffer[valueSize + 1] = {0};
    strncpy(buffer, value, valueSize);

    return strtoul(buffer, NULL, 16);
}

void CpioExtractor::createDirectory(const char* basePath, const char* filename) {
    Path rootPath(basePath);
    Path filePath = rootPath + Path(filename);

    auto segments = filePath.split();
    Path mkdirPath(rootPath);
    for (const auto& segment : segments) {
        mkdirPath.append(segment.segment);
        vfs_mkdir(mkdirPath.string().c_str());
    }
}

void CpioExtractor::writeFile(const char* basePath, const CpioFile& file) {
    Path rootPath(basePath);
    Path filePath = rootPath + Path(file.name);
    filePath.trim();

    Path dirPath(filePath.parent());
    this->createDirectory(basePath, dirPath.string().c_str());

    int fd = vfs_open(filePath.string().c_str(), O_WRONLY | O_CREAT);
    if (-1 == fd) {
        return;
    }

    vfs_write(fd, file.file_data, file.file_size);
    vfs_close(fd);
}
