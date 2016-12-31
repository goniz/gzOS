#ifndef GZOS_TMPFS_FS_H
#define GZOS_TMPFS_FS_H

#include <lib/kernel/vfs/VirtualFileSystem.h>
#include <lib/primitives/sys/tree.h>
#include <initializer_list>

#ifdef __cplusplus

class TmpNode;

class TmpFile {
public:
    TmpFile(std::unique_ptr<uint8_t[]> data, size_t size)
        : data(std::move(data)),
          size(size)
    {

    }

    TmpFile(size_t size)
        : data(std::make_unique<uint8_t[]>(size)),
          size(size)
    {

    }

    std::unique_ptr<uint8_t[]> data;
    size_t size;
};

class TmpDirectory {
public:
    TmpDirectory(void) = default;
    TmpDirectory(std::vector<TmpNode>&& initialNodes)
            : nodes(std::move(initialNodes))
    {

    }

    std::vector<TmpNode> nodes;
};

class TmpNode {
public:
    TmpNode(std::string&& name, std::unique_ptr<TmpFile> file)
        : name(std::move(name)),
          type(Type::File),
          file(std::move(file))
    {

    }

    TmpNode(std::string&& name, std::unique_ptr<TmpDirectory> dir)
            : name(std::move(name)),
              type(Type::Directory),
              directory(std::move(dir))
    {

    }

    std::string name;

    enum class Type {
        File,
        Directory
    } type;

    std::unique_ptr<TmpFile> file;
    std::unique_ptr<TmpDirectory> directory;
};

class TmpFileSystem : public FileSystem
{
    friend class TmpReaddirFileDescriptor;

public:
    TmpFileSystem(const char* sourceDevice);
    virtual std::unique_ptr<FileDescriptor> open(const char *path, int flags) override;

    std::unique_ptr<FileDescriptor> readdir(const char* path) override;

private:
    int findNode(const TmpNode* baseNode, const char* path, const TmpNode** outNode, const TmpNode** outParentNode) const;

    TmpNode _rootNode;
};

#endif //cplusplus
#endif //GZOS_TMPFS_FS_H
