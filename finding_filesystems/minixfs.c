/**
 * finding_filesystems
 * CS 241 - Fall 2020
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DATA_BLOCK_SIZE (16 * KILOBYTE)

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
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }
    node->mode = new_permissions;
    node->mode |= (node->mode >> RWX_BITS_NUMBER) << RWX_BITS_NUMBER;

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
    inode_number first_unused_in = first_unused_inode(fs);

    if (first_unused_in== -1) return NULL;

    char* filename;
    inode* parent_in = parent_directory(fs, path, (const char**) &filename);

    if (!valid_filename(filename)) return NULL;

    if ((parent_in->mode & 0700) != 0700) return NULL;

    size_t size = FILE_NAME_LENGTH + INODE_SIZE;

    char dir_string[size];

    minixfs_dirent dirent;
    dirent.name = filename;
    dirent.inode_num = first_unused_inode(fs);

    make_string_from_dirent(dir_string,dirent);

    size_t total_size = parent_in->size + size;
    size_t total_direct_block_size = NUM_DIRECT_BLOCKS * DATA_BLOCK_SIZE;
    size_t total_indirect_block_size = NUM_INDIRECT_BLOCKS * DATA_BLOCK_SIZE;



    if (total_size <= total_direct_block_size) {

        int num_data_blocks = parent_in->size / (int) sizeof(data_block);

        char* data = (char*) (fs->data_root + parent_in->direct[num_data_blocks]);

        int any_remaining = parent_in->size % (int) sizeof(data_block);
        if (!any_remaining) {
            data_block_number first_data = first_unused_data(fs);

            if (first_data == -1) return NULL;

            parent_in->direct[num_data_blocks] = first_data;
            set_data_used(fs, first_data, true);

            data = (char*) (fs->data_root + first_data);
            memcpy(data, dir_string, size);
        
        } else {
            memcpy(data + any_remaining, dir_string,size);  
        }
        parent_in->size += size;

        inode* new_in = fs->inode_root + first_unused_in;
        init_inode(parent_in, new_in);
        return new_in;


    } else if (total_size <= total_indirect_block_size + total_direct_block_size) {

        data_block_number block_num = (parent_in->size / (int) sizeof(data_block)) - NUM_DIRECT_BLOCKS;

        if (parent_in->indirect == -1) {
            data_block_number first_data = first_unused_data(fs);
            
            if (first_data == -1) return NULL;

            parent_in->indirect = first_data;

            set_data_used(fs, first_data, true);

            data_block_number* block_ptr = (data_block_number*) (fs->data_root + first_data);
            data_block_number next_data = first_unused_data(fs);
            
            if (next_data == -1) return NULL;

            set_data_used(fs, next_data, true);
            block_ptr[0] = next_data;

            memcpy((char*)(fs->data_root + next_data), dir_string, size);

        } else {
            data_block_number* block_ptr = (data_block_number*)(fs->data_root + parent_in->indirect);

            int rem = parent_in->size % (int)sizeof(data_block);
            if (!rem) {
                memcpy((char*) (fs->data_root + block_ptr[block_num]), dir_string, size);

            } else { 
                data_block_number next_data = first_unused_data(fs);
                
                if (next_data == -1) return NULL;
                
                set_data_used(fs,next_data, true);
                block_ptr[block_num] = next_data;

                memcpy((char*)(fs->data_root + next_data), dir_string,size);
            }
        }

        parent_in->size += size;

        inode* new_in = fs->inode_root + first_unused_in;
        init_inode(parent_in, new_in);
        return new_in;

    }
    
    return NULL;

}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {

    if (!strcmp(path, "info")) {
        size_t used_datablocks = 0;

        for (int i = 0; i < DATA_NUMBER; i++) {
          if (get_data_used(fs,i)) {
            used_datablocks++;
          }
        }

        char* info_str = block_info_string(used_datablocks);

        size_t len = strlen(info_str) - *off;
        size_t min_len = count < len ? count : len;

        memcpy(buf, info_str + *off, min_len);
        *off += min_len;

        return min_len; 

    } else {    // for my own sake lol
        errno = ENOENT;
        return -1;
    }
}

void set_times(inode* node) {
    clock_gettime(CLOCK_REALTIME, &node->mtim);
    clock_gettime(CLOCK_REALTIME, &node->atim);
}


ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    if (*off + count > sizeof(data_block) * (NUM_INDIRECT_BLOCKS + NUM_DIRECT_BLOCKS)) {
        errno=ENOSPC;
        return -1;
    }

    inode* node = get_inode(fs, path);
    
    if (!node) {
        node = minixfs_create_inode_for_path(fs,path);
        if(!node) {
            return -1;
        }
    }

    clock_gettime(CLOCK_REALTIME, &node->mtim);    
    
    if (node->size < *off + count) {
        int block_count = (*off + count) / (int) sizeof(data_block);

        if ((*off + count) % (int) sizeof(data_block)) {
            block_count++;
        }

        if (minixfs_min_blockcount(fs, path, block_count) == -1) return -1;
    }

    size_t block_index = *off / sizeof(data_block);
    int offset = *off - block_index * (int) sizeof(data_block);
    int remaining = count;
    char* block_ptr = NULL;

    char* bc = (char*) buf;

    size_t min_size = remaining < (int) sizeof(data_block) ? remaining : sizeof(data_block);

    if (block_index < NUM_DIRECT_BLOCKS) {
        block_ptr = (char*) fs->data_root + node->direct[block_index];

        if (count + offset <= (int) sizeof(data_block)) {
            *off += count;
            node->size = *off;

            memcpy(block_ptr + offset, buf, count);
            set_times(node);

            return count;
        } 

        int rem = sizeof(data_block) - offset;
        memcpy(block_ptr + offset, buf, rem);
        remaining -= rem;
        bc += rem;
        block_index++;

        while (remaining > 0 && block_index < NUM_DIRECT_BLOCKS) {
            block_ptr = (char*) fs->data_root+node->direct[block_index];

            memcpy(block_ptr, bc, min_size);
            bc += min_size;
            remaining -= min_size;

            block_index++;
        }

        if (remaining <= 0) {
            *off += count;
            node->size = *off;
            set_times(node);


            return count;
        }

        data_block_number* block_num = (int*)(fs->data_root+node->indirect);
        while (remaining > 0) {
            block_ptr = (char*) fs->data_root + *block_num;
            memcpy(block_ptr, bc, min_size);
            bc += min_size;
            remaining -= min_size;
            block_num++;
        }
        *off += count;
        node->size = *off;
        set_times(node);


        return count;
      
    } else {
        data_block_number* block_num = (data_block_number*) (fs->data_root + node->indirect);
        int db_offset = block_index - NUM_DIRECT_BLOCKS;
        block_num += db_offset;
        block_ptr = (char*) (fs->data_root + *block_num);
        if (count + offset <= (int) sizeof(data_block)) {
            memcpy(block_ptr + offset, buf, count);
            *off += count;
            node->size = *off;
            set_times(node);


            return count;
        } 
        memcpy(block_ptr + offset, buf, sizeof(data_block) - offset);
        remaining -= sizeof(data_block) - offset;
        block_index += sizeof(data_block) - offset;  
        
        block_num++;

        while (remaining > 0) {
            block_ptr = (char*) (fs->data_root + *block_num);
            memcpy(block_ptr, bc, min_size);
            block_index += min_size;
            remaining -= min_size;

            block_num++;
        }

        *off += count;
        node->size = *off;
        set_times(node);

        return count;
        
    }

}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    
    if (!path) {
        errno = ENOENT; 
        return -1;
    }

    inode* node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT; 
        return -1;
    }

    size_t bytes_read = 0;
    size_t start_index = *off / sizeof(data_block);

    if (start_index > NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS ) {
        return 0;
    }

    int offset = *off - start_index * (int)sizeof(data_block);
    void* block_ptr = NULL;

    if (*off >= (long) node->size) {
      return 0;
    }
    
    if (*off + count >= node->size) {
        count = node->size - *off;
    } 

    size_t min_size = count < sizeof(data_block) ? count : (int) sizeof(data_block);

    if (start_index < NUM_DIRECT_BLOCKS) {

        block_ptr = (char*) (fs->data_root + node->direct[start_index]);
        char* bc = buf;

        if (count  <= sizeof(data_block) - offset) {
            memcpy((char*)buf, block_ptr + offset, count);
            bytes_read += count;
            *off = *off + bytes_read;
            clock_gettime(CLOCK_REALTIME, &node->atim);

            return bytes_read;

        } else {
            memcpy((char*)buf, block_ptr + offset, (int) sizeof(data_block) - offset);
            
            count -= sizeof(data_block) - offset;
            bc += (int) sizeof(data_block) - offset;
            bytes_read += (int) sizeof(data_block) - offset;

            start_index++;


        }

        while (count > 0 && start_index < NUM_DIRECT_BLOCKS) {
            block_ptr = (char*) (fs->data_root + node->direct[start_index]);

            memcpy(bc, block_ptr, min_size);
            bc += min_size;
            count -= min_size;
            start_index++;
            bytes_read += min_size;
        }

        if (count <= 0) {
            *off = *off + bytes_read;
            clock_gettime(CLOCK_REALTIME, &node->atim);
            return bytes_read;
        }

        data_block_number* datablock = (int*) (fs->data_root + node->indirect);
        while (count != 0) {
            block_ptr = (char*) (fs->data_root + *datablock);
            memcpy(bc, block_ptr, min_size);

            bc += min_size;
            bytes_read += min_size;
            count -= min_size;

            datablock++;
        }
        *off = *off + bytes_read;
        clock_gettime(CLOCK_REALTIME, &node->atim);
        return bytes_read;
      
    } else {
        data_block_number* datablock = (int*)(fs->data_root + node->indirect);
        int db_offset = start_index - NUM_DIRECT_BLOCKS;
        char* bc = buf;

        datablock += db_offset;
        block_ptr = (char*)(fs->data_root + *datablock);

        if (count + offset <= sizeof(data_block)) {
            *off = *off + count;
            memcpy(bc, block_ptr + offset, count);

            clock_gettime(CLOCK_REALTIME, &node->atim);
            return count;
        }

        memcpy(buf, block_ptr + offset, (int) sizeof(data_block) - offset);
        
        bc += (int) sizeof(data_block) - offset;  
        bytes_read += (int) sizeof(data_block) - offset;
        count -= (int) sizeof(data_block) - offset;

        datablock++;
      
      
        while (count > 0) {
            block_ptr = (char*)(fs->data_root + *datablock);

            memcpy(bc,block_ptr,min_size);

            bytes_read += min_size;
            bc += min_size;
            count -= min_size;

            datablock++;

        }
        *off = *off + bytes_read;
        clock_gettime(CLOCK_REALTIME, &node->atim);

        return bytes_read;
      
    }
  

}
