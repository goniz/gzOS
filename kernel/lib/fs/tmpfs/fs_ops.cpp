#include <platform/drivers.h>
#include <platform/kprintf.h>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <lib/primitives/interrupts_mutex.h>
#include <lib/primitives/hexdump.h>
#include <cassert>
#include "fs_ops.h"

class TmpfsFileDescriptor : public MemoryBackedFileDescriptor {
public:
    TmpfsFileDescriptor(const TmpNode* node)
        : MemoryBackedFileDescriptor((uintptr_t) node->file->data.get(),
                                     (uintptr_t) ((uintptr_t)node->file->data.get() + node->file->size)),
          _node(node)
    {

    }

    ~TmpfsFileDescriptor(void) = default;

private:
    const TmpNode* _node;
};

void ls(VirtualFileSystem &vfs, const char* path)
{
    struct DirEntry dirent;
    int ret;
    auto readdirfd = vfs.readdir(path);
    if (nullptr == readdirfd) {
        printf("failed to create readdir handle\n");
        return;
    }

    printf("ls %s\n", path);
    while (0 < (ret = readdirfd->read(&dirent, sizeof(dirent)))) {
        printf("* %s%s\n", dirent.name, dirent.type == DirEntryType::DIRENT_DIR ? "/" : "");
    }

    printf("ret: %d\n", ret);
    readdirfd->close();
}

static int fs_tmpfs_init(void) {
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("tmpfs", [](const char *source) {
        return std::unique_ptr<FileSystem>(new TmpFileSystem(source));
    });

    return 0;
}

DECLARE_DRIVER(fs_tmpfs, fs_tmpfs_init, STAGE_SECOND + 1);

TmpFileSystem::TmpFileSystem(const char *sourceDevice)
    : _rootNode("/", std::make_unique<TmpDirectory>())
{

}

class TmpReaddirFileDescriptor : public ReaddirFileDescriptor {
public:
    TmpReaddirFileDescriptor(const TmpNode* node)
        : _node(node),
          _index(0)
    {

    }

private:
    bool getNextEntry(struct DirEntry &dirEntry) override {
        InterruptsMutex mutex(true);
        if (_index >= _node->directory->nodes.size()) {
            return false;
        }

        const auto& nodes = _node->directory->nodes;
        strncpy(dirEntry.name, nodes[_index].name.c_str(), sizeof(dirEntry.name));
        dirEntry.type = nodes[_index].type == TmpNode::Type::Directory ? DIRENT_DIR : DIRENT_REG;

        _index++;
        return true;
    }

    const TmpNode* _node;
    size_t _index;
};

std::unique_ptr<FileDescriptor> TmpFileSystem::readdir(const char* path) {
    const TmpNode *node = nullptr;

    this->findNode(&_rootNode, path, &node, nullptr);
    if (!node) {
        return nullptr;
    }

    if (node->type != TmpNode::Type::Directory) {
        return nullptr;
    }

    return std::make_unique<TmpReaddirFileDescriptor>(node);
}

std::unique_ptr<FileDescriptor> TmpFileSystem::open(const char *path, int flags) {

    const TmpNode* node = nullptr;
    const TmpNode* parentDir = nullptr;

    this->findNode(&_rootNode, path, &node, &parentDir);
    if (!node) {
        if (parentDir && (flags & O_CREAT)) {
            assert(parentDir->type == TmpNode::Type::Directory);
            parentDir->directory->nodes.emplace_back(
                    Path(path).filename(),
                    std::make_unique<TmpFile>(0)
            );

            return std::make_unique<TmpfsFileDescriptor>(&parentDir->directory->nodes.back());
        }

        return nullptr;
    }

    if (node->type != TmpNode::Type::File) {
        return nullptr;
    }

    return std::make_unique<TmpfsFileDescriptor>(node);
}

/*
 * /tmp/dir/file
 *
 * / -> tmp/ -> dir/ -> file
 * dir/ -> file
 */
int TmpFileSystem::findNode(const TmpNode* baseNode, const char* path, const TmpNode** outNode, const TmpNode** outParentNode) const {
    // let start from the baseNode provided
    const TmpNode* node = baseNode;
    const TmpNode* prevNode = nullptr;

    // split the given path to its segments
    Path desiredPath(path);
    const auto segments = desiredPath.split();

    // iterate over its segments and try to walk the nodes until we find the requested node, or fail
    for (size_t i = 0; i < segments.size(); i++) {
        const auto& segment = segments[i];
        const bool is_last = (segments.size() - 1) == i;

        // the current node can only be a non-directory if its the last segment (destination segment)
        // because if its not, we wont be able to traverse it..
        if (is_last && segment.segment == node->name) {
            if (outNode) {
                *outNode = node;
            }

            if (outParentNode) {
                *outParentNode = prevNode;
            }

            return 0;
        }

        // if its a file, and the segment is not last, we cannot find this path..
        if (TmpNode::Type::Directory != node->type) {
            if (outNode) {
                *outNode = nullptr;
            }

            if (outParentNode) {
                *outParentNode = prevNode;
            }

            return 0;
        }

        for (const auto& currentNode: node->directory->nodes) {
            if (is_last && currentNode.name == segment.segment) {
                if (outNode) {
                    *outNode = &currentNode;
                }

                if (outParentNode) {
                    *outParentNode = node;
                }

                return 0;
            }

            if (currentNode.name == segment.segment && TmpNode::Type::Directory == currentNode.type) {
                prevNode = node;
                node = &currentNode;
                break;
            }
        }
    }

    return 0;
}
