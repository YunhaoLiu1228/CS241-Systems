/**
 * finding_filesystems
 * CS 241 - Fall 2020
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    inode* node = get_inode(fs, path);

    // If this functions fails because path doesn't exist, set errno to ENOENT
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    new_permissions |= 0777;
    node->mode |= new_permissions;
    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    inode* node = get_inode(fs, path);

    // If this functions fails because path doesn't exist, set errno to ENOENT
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    //  If owner is ((uid_t)-1), then don't change the node's uid.
    if (owner != ((uid_t) - 1)) {
        node->uid = owner;
    }
    //  If group is ((gid_t)-1), then don't change the node's gid.
    if (group != ((gid_t) - 1)) {
        node->gid = group;
    }

    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // return NULL if inode already exists ...
    if (get_inode(fs, path) != NULL) return NULL;

    // or cannot be created (no available inodes)
    if (first_unused_inode(fs) == -1) return NULL;

    char* filename;
    inode* parent_in = parent_directory(fs, path, (const char**) &filename);

    if (!valid_filename(filename)) return NULL;

    if ((parent_in->mode & 0700) != 0700) return NULL;

    return NULL;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    return -1;
}
