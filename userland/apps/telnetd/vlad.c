#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <libc/select.h>
#include <stdio.h>
#include "vlad.h"

struct vlad_entry {
    TAILQ_ENTRY(vlad_entry) next;
    int fd;
    vlad_handler_func handler;
    void* arg;
};

struct vlad_handle {
    TAILQ_HEAD(, vlad_entry) entries;
};

vlad_handle_t vlad_init(void) {
    vlad_handle_t handle = malloc(sizeof(*handle));
    if (!handle) {
        return NULL;
    }

    memset(handle, 0, sizeof(*handle));

    TAILQ_INIT(&handle->entries);

    return handle;
}

static void vlad_free_entry(struct vlad_entry* entry) {
    if (!entry) {
        return;
    }

    // TODO: implement
    free(entry);
}

void vlad_free(vlad_handle_t handle) {
    if (!handle) {
        return;
    }

    while (!TAILQ_EMPTY(&handle->entries)) {
        struct vlad_entry* entry = TAILQ_FIRST(&handle->entries);
        TAILQ_REMOVE(&handle->entries, entry, next);

        vlad_free_entry(entry);
    }

    free(handle);
}

bool vlad_register_fd(vlad_handle_t handle, int fd, vlad_handler_func handler, void* arg) {
    if (!handle || 0 > fd || !handler) {
        return false;
    }

    struct vlad_entry* entry = malloc(sizeof(*entry));
    if (!entry) {
        return false;
    }

    memset(entry, 0, sizeof(*entry));

    entry->fd = fd;
    entry->handler = handler;
    entry->arg = arg;

    if (TAILQ_EMPTY(&handle->entries)) {
        TAILQ_INSERT_HEAD(&handle->entries, entry, next);
    } else {
        TAILQ_INSERT_TAIL(&handle->entries, entry, next);
    }

    return true;
}

void vlad_unregister_fd(vlad_handle_t handle, int fd) {
    if (!handle || 0 > fd) {
        return;
    }

    struct vlad_entry* found = NULL;
    struct vlad_entry* item = NULL;
    TAILQ_FOREACH(item, &handle->entries, next) {
        if (fd == item->fd) {
            found = item;
            break;
        }
    }

    if (!found) {
        return;
    }

    TAILQ_REMOVE(&handle->entries, found, next);
    vlad_free_entry(found);
}

int vlad_fill_fd_set(const vlad_handle_t handle, fd_set* read_fds) {
    int maxfd = -1;

    struct vlad_entry* item = NULL;
    TAILQ_FOREACH(item, &handle->entries, next) {
        FD_SET(item->fd, read_fds);

        if (maxfd < item->fd) {
            maxfd = item->fd;
        }
    }

    return maxfd;
}

void vlad_call_handlers(vlad_handle_t handle, fd_set* read_fds) {
    struct vlad_entry* item = NULL;
    struct vlad_entry* temp_item = NULL;
    TAILQ_FOREACH_SAFE(item, &handle->entries, next, temp_item) {
        if (!FD_ISSET(item->fd, read_fds)) {
            continue;
        }

        if (!item->handler(item->arg)) {
            TAILQ_REMOVE(&handle->entries, item, next);
            vlad_free_entry(item);
        }
    }
}

bool vlad_loop_once(vlad_handle_t handle)
{
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    int maxfd = vlad_fill_fd_set(handle, &read_fds);
    if (-1 == maxfd) {
        return false;
    }

    select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);

    vlad_call_handlers(handle, &read_fds);
    return true;
}

void vlad_loop(vlad_handle_t handle)
{
    while (true) {
        if (!vlad_loop_once(handle)) {
            break;
        }
    }
}
