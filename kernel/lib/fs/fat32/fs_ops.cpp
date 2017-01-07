#include <platform/drivers.h>
#include <platform/kprintf.h>
#include <lib/kernel/vfs/ReaddirFileDescriptor.h>
#include <cstdio>
#include <lib/kernel/vfs/VirtualFileSystem.h>
#include "fs_ops.h"
#include "Fat32FileDescriptor.h"
#include "ff.h"

static int fs_fat32_init(void) {
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    vfs.registerFilesystem("fat32", [](const char *source, const char* destName) {
        auto fatRootNode = std::make_shared<Fat32FileSystem>(source, std::string(destName));
        return std::static_pointer_cast<VFSNode>(fatRootNode);
    });

    return 0;
}

DECLARE_DRIVER(fs_fat32, fs_fat32_init, STAGE_SECOND + 1);

Fat32VFSNode::Fat32VFSNode(VFSNode::Type type, std::string&& fullPath, std::string&& segment)
        : BasicVFSNode(type, std::move(segment)),
          _parentFullPath(std::move(fullPath))
{
//    kprintf("_parentFullPath: %s\n", _parentFullPath.c_str());
//    kprintf("segment: %s\n", this->getPathSegment().c_str());
}

const std::vector<SharedNode>& Fat32VFSNode::childNodes(void) {
    if (VFSNode::Type::Directory != this->getType()) {
        return _nodes;
    }

    if (_nodes.empty()) {
        DIR dp;
        memset(&dp, 0, sizeof(dp));

        Path dirPath = std::move(this->getCurrentFatPath());
        auto dirString = dirPath.string();

        if (FR_OK != f_opendir(&dp, dirString.c_str())) {
            return _nodes;
        }

        while (true) {
            FILINFO fno;
            memset(&fno, 0, sizeof(fno));

            auto result = f_readdir(&dp, &fno);
            if (FR_OK != result || fno.fname[0] == 0) {
                break;
            }

            VFSNode::Type type(VFSNode::Type::File);
            if (fno.fattrib & AM_DIR) {
                type = VFSNode::Type::Directory;
            }

            auto fatnode = std::make_shared<Fat32VFSNode>(type, std::string(dirString), std::string(fno.fname));
            auto node = std::static_pointer_cast<VFSNode>(fatnode);
            _nodes.push_back(node);
        }

        f_closedir(&dp);
    }

    return _nodes;
}

std::unique_ptr<FileDescriptor> Fat32VFSNode::open(void) {
    Path dirPath = std::move(this->getCurrentFatPath());
    auto pathString = dirPath.string();
    return std::make_unique<Fat32FileDescriptor>(pathString.c_str(), O_RDONLY);
}

SharedNode Fat32VFSNode::createNode(VFSNode::Type type, std::string&& path) {
    if (this->getType() != VFSNode::Type::Directory) {
        return nullptr;
    }

    Path fullPath(_parentFullPath);
    fullPath.append(path);
    auto str  = fullPath.string();

    switch (type)
    {
        case Type::File:
            FIL fil;
            if (FR_OK != f_open(&fil, str.c_str(), FA_CREATE_ALWAYS)) {
                return nullptr;
            }

            break;

        case Type::Directory:
#if !defined(_FS_READONLY)
            if (FR_OK != f_mkdir(str.c_str())) {
                return nullptr;
            }
#else
            return nullptr;
#endif

            break;
    }

    auto fatnode = std::make_shared<Fat32VFSNode>(type, std::string(str), std::move(path));
    auto node = std::static_pointer_cast<VFSNode>(fatnode);
    _nodes.push_back(node);
    return node;
}

bool Fat32VFSNode::isRootNode(void) const {
    return false;
}

Path Fat32VFSNode::getCurrentFatPath(void) const {
    Path dirPath(_parentFullPath);
    if (!this->isRootNode()) {
        if ("" == dirPath.string()) {
            dirPath = Path(this->getPathSegment());
        } else {
            dirPath.append(this->getPathSegment());
        }
    }

    return dirPath;
}

// TODO: check this path thingy
Fat32FileSystem::Fat32FileSystem(const char* sourceDevice, std::string&& path)
        : Fat32VFSNode(VFSNode::Type::Directory, std::string(""), std::string(path)),
          _sourceFd()
{
    VirtualFileSystem &vfs = VirtualFileSystem::instance();

    _sourceFd = vfs.open(sourceDevice, O_RDONLY);
    if (!_sourceFd) {
        kprintf("Could NOT open file '%s'\n", sourceDevice);
        throw std::exception();
    }

    if (FR_OK != f_mount(&_fs, "PFLASH", 1)) {
        kprintf("Could NOT mount file system: '%s'\n", sourceDevice);
        throw std::exception();
    }
}

bool Fat32FileSystem::isRootNode(void) const {
    return true;
}

