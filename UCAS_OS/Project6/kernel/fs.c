#include <os/fs.h>
#include <sbi.h>
#include <os/stdio.h>
#include <screen.h>
#include <os/time.h>
#include <os/string.h>
#include <pgtable.h>

int do_mkfs()
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    // if(superblock->magic_number == MAGIC){
    //     prints("[FS] FS has existed!\n");
    //     int a = do_statfs();
    //     return 1 + a;
    // }
    screen_reflush();
    prints("[FS] Start initialize filesystem!\n");

    prints("[FS] Setting superblock...\n");
    kbzero(superblock, sizeof(superblock_t));
    superblock->magic_number = MAGIC;

    superblock->fs_size = FS_SIZE;  
    superblock->fs_start = FIRST_SECTOR;

    superblock->sectormap_size = SECTORMAP_SIZE;
    superblock->sectormap_start = SECTORMAP_SECTOR_OFFSET;

    superblock->inodemap_size = INODEMAP_SIZE;
    superblock->inodemap_start = INODEMAP_SECTOR_OFFSET;

    superblock->inode_size = INODEBLOCK_SIZE;
    superblock->inode_start = INODEBLOCK_SECTOR_OFFSET;

    superblock->datablock_size = DATABLOCK_SIZE;
    superblock->datablock_start = DATABLOCK_SECTOR_OFFSET;

    prints("magic : 0x%x\n", superblock->magic_number);
    prints("num sector : %d, start_sector : %d\n",superblock->fs_size,superblock->fs_start);
    prints("block map offset : %d (%d)\n",superblock->sectormap_start,superblock->sectormap_size);
    prints("inode map offset : %d (%d)\n",superblock->inodemap_start,superblock->inodemap_size);
    prints("data offset : %d (%d)\n",superblock->datablock_start,superblock->datablock_size);
    prints("inode entry size : %dB, dir entry size : %dB\n", INODE_SIZE, DENTRY_SIZE);
    sbi_sd_write(kva2pa(superblock), 1, FIRST_SECTOR);

    kbzero(p, SECTOR_SIZE);
    prints("[FS] Setting inode-map...\n");
    p[0] = 1;
    sbi_sd_write(kva2pa(p), INODEMAP_SIZE, FIRST_SECTOR + INODEMAP_SECTOR_OFFSET);

    uint8_t *q = (uint8_t *)kmalloc(SECTOR_SIZE * SECTORMAP_SIZE);
    kbzero(q, SECTOR_SIZE * SECTORMAP_SIZE);
    prints("[FS] Setting sector-map...\n");
    sbi_sd_write(kva2pa(q), SECTORMAP_SIZE, FIRST_SECTOR + SECTORMAP_SECTOR_OFFSET);

    prints("[FS] Setting inode...\n");
    inode_t *inode = (inode_t *)p;
    inode->ino = 0;
    inode->mode = DIR;
    inode->type = O_RDWR;
    inode->link = 0;
    inode->size = SECTOR_SIZE;
    inode->valid_size = 2 * DENTRY_SIZE;
    inode->modified_time = get_timer();
    inode->identry[0].ino = 0;
    inode->identry[1].ino = 0;
    inode->identry[0].valid = VALID;
    inode->identry[1].valid = VALID;
    inode->identry[0].mode = DIR;
    inode->identry[1].mode = DIR;
    kbzero(inode->identry[0].name, MAX_FILE_LENGTH);
    kbzero(inode->identry[1].name, MAX_FILE_LENGTH);
    kmemcpy(inode->identry[0].name, ".", 1);
    kmemcpy(inode->identry[1].name, "..", 2);
    for(int i = 2; i < DENTRY_NUM; i++)
        inode->identry[i].valid = INVALID;

    for(int i = 0; i < DIRECT_NUM; i++)
        inode->direct_block[i] = 0;
    for(int i = 0; i < INDIRECT_NUM; i++)
        inode->indirect_block[i] = 0;
    for(int i = 0; i < D_INDIRECT_NUM; i++)
        inode->d_indirect_block[i] = 0;
    inode->t_indirect_block = 0;
    write_inode(inode, 0);

    for(int i = 0; i < MAX_FILE; i++){
        file[i].inode = 0;
    }

    prints("[FS] Initialize filesystem finished!\n");
    user_location = 0;
    return 12;
}

int do_statfs()
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);

    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return 1;
    }

    uint8_t *q = (uint8_t *)kmalloc(SECTOR_SIZE * SECTORMAP_SIZE);
    sbi_sd_read(kva2pa(q), SECTORMAP_SIZE, FIRST_SECTOR + SECTORMAP_SECTOR_OFFSET);
    int sector_count = 0;
    for (int i = 0; i < SECTOR_SIZE * SECTORMAP_SIZE; i++){
        for(int j = 0; j < 8 ; j++){
            if(q[i] % 2){
                sector_count++;
            }
            q[i] /= 2;
        }
    }

    uint8_t *r = (uint8_t *)kmalloc(SECTOR_SIZE);
    sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + INODEMAP_SECTOR_OFFSET);
    int inode_count = 0;
    for (int i = 0; i < SECTOR_SIZE; i++){
        for(int j = 0; j < 8 ; j++){
            if(r[i] % 2){
                inode_count++;
            }
            r[i] /= 2;
        }
    }

    prints("magic: 0x%x (KFS).\n", superblock->magic_number);
    prints("used sector: %d/%d ", sector_count, superblock->fs_size);
    prints("start sector: %d\n",superblock->fs_start);
    prints("sector map offset : %d, occupied sector : %d\n",superblock->sectormap_start, superblock->sectormap_size);
    prints("inode map offset : %d, occupied sector : %d\n",superblock->inodemap_start, superblock->inodemap_size);
    prints("inode offset: %d, occupied sector : %d, used : %d\n",superblock->inode_start, superblock->inode_size, inode_count);
    prints("block offset: %d, occupied sector : %d\n",superblock->datablock_start, superblock->datablock_size);
    
    return 6;
}

void do_mkdir(char* argv)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }
    inode_t *inode = get_inode(user_location);
    inode_t *inode2 = relocation(inode, argv, MKDIR);
    if(inode2 == NULL)
        prints("No Such Dir Or Dir Has Existed!\n");
    else
        prints("Mkdir Success!\n");
}

void do_cd(char* argv)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }

    inode_t *inode = get_inode(user_location);
    inode_t *inode2 = relocation(inode, argv, NOT_MKDIR);
    if(inode2 == NULL){
        prints("No Such Path\n");
    }else{
        user_location = inode2->ino;
        prints("CD Success!\n");
    }
}

void do_fs_ls()
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }

    inode_t *inode = get_inode(user_location);
    int size = inode->valid_size / DENTRY_SIZE;
    for (int i = 0; i < DENTRY_NUM; i++)
        if(inode->identry[i].valid == VALID)
            prints("%s\t", inode->identry[i].name);
}

int do_fs_ls_l()
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }

    int row = 0;
    inode_t *inode = get_inode(user_location);
    inode_t *new_inode;
    int size = inode->valid_size / DENTRY_SIZE;
    for (int i = 0; i < DENTRY_NUM; i++){
        if(inode->identry[i].valid == VALID){
            new_inode = get_inode(inode->identry[i].ino);
            prints("name:%s\tinode:%d\tmode:%d\tlink:%d\tsize:%d\tmodified time:%d\n", inode->identry[i].name, new_inode->ino, new_inode->mode, new_inode->link, new_inode->valid_size, new_inode->modified_time);
            row ++;
        }
    }
    return row;
}

void do_rmdir(char* argv)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }

    inode_t *inode = get_inode(user_location);
    inode_t *inode2 = relocation(inode, argv, NOT_MKDIR);
    if(inode2 == NULL){
        prints("No Such Dir\n");
        return;
    }

    int del_inode = inode2->ino;
    change_inode_bitmap(del_inode, 0);
    int parent_inode = inode2->identry[0].ino;

    kbzero(inode2,sizeof(inode_t));
    write_inode(inode2, del_inode);

    inode2 = get_inode(parent_inode);
    inode2->modified_time = get_timer();
    inode2->valid_size -= DENTRY_SIZE;
    for(int i = 0; i < DENTRY_NUM; i++){
        if(inode2->identry[i].ino == del_inode){
            inode2->identry[i].valid = INVALID;
        }
    }
    write_inode(inode2, parent_inode);
}

void do_touch(char* argv)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return;
    }

    inode_t *new_inode = get_inode(user_location);
    for(int i = 0; i < DENTRY_NUM; i++)
        if(!strcmp(argv, new_inode->identry[i].name) && new_inode->identry[i].valid == VALID){
            prints("The File Has Existed!\n");
            return;
        }
    
    int location;
    int inode_number = get_inode_number();
    int parent_inode = new_inode->ino;

    for(location = 0; location < DENTRY_NUM; location++)
        if(new_inode->identry[location].valid == INVALID)
            break;
    if(location == DENTRY_NUM){
        prints("Touch Error!\n");
        return;
    }

    new_inode->valid_size += DENTRY_SIZE;
    new_inode->identry[location].ino = inode_number;
    new_inode->identry[location].mode = FILE;
    new_inode->identry[location].valid = VALID;
    new_inode->modified_time = get_timer();
    kmemcpy(new_inode->identry[location].name, argv, kstrlen(argv));
    write_inode(new_inode, parent_inode);

    new_inode = get_inode(inode_number);
    new_inode->ino = inode_number;
    new_inode->mode = FILE;
    new_inode->type = O_RDWR;
    new_inode->link = 1;
    new_inode->size = SECTOR_SIZE;
    new_inode->valid_size = 0;
    new_inode->modified_time = get_timer();
    new_inode->identry[0].ino = parent_inode;
    new_inode->identry[1].ino = inode_number;
    new_inode->identry[0].valid = VALID;
    new_inode->identry[1].valid = VALID;
    new_inode->identry[0].mode = DIR;
    new_inode->identry[1].mode = FILE;
    for(int i = 0; i < DIRECT_NUM; i++){
        new_inode->direct_block[i] = 0;
    }
    for(int i = 0; i < INDIRECT_NUM; i++){
        new_inode->indirect_block[i] = 0;
    }
    for(int i = 0; i < D_INDIRECT_NUM; i++){
        new_inode->d_indirect_block[i] = 0;
    }
    new_inode->t_indirect_block = 0;
    
    kbzero(new_inode->identry[0].name, MAX_FILE_LENGTH);
    kbzero(new_inode->identry[1].name, MAX_FILE_LENGTH);
    kmemcpy(new_inode->identry[0].name, ".", 1);
    kmemcpy(new_inode->identry[1].name, "..", 2);
    for(int i = 2; i < DENTRY_NUM; i++)
        new_inode->identry[i].valid = INVALID;
    write_inode(new_inode, inode_number);
    change_inode_bitmap(inode_number, 1);
}

int do_fopen(char* argv, int access)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    superblock_t *superblock = (superblock_t *)p;
    sbi_sd_read(kva2pa(superblock), 1, FIRST_SECTOR);
    if(superblock->magic_number != MAGIC){
        prints("No FS Existed!\n");
        return 0;
    }

    int location;
    inode_t *new_inode = get_inode(user_location);
    for(location = 0; location < DENTRY_NUM; location++)
        if(!strcmp(argv, new_inode->identry[location].name) && new_inode->identry[location].valid == VALID)
            break;
    if(location == DENTRY_NUM){
        do_touch(argv);
        new_inode = get_inode(user_location);
        for(location = 0; location < DENTRY_NUM; location++){
            if(!strcmp(argv, new_inode->identry[location].name) && new_inode->identry[location].valid == VALID)
                break;
        }
        if(location == DENTRY_NUM)
            return -1;
    }
    new_inode = get_inode(new_inode->identry[location].ino);

    int file_number = get_file_number();
    if(file_number == MAX_FILE){
        prints("Too Many Files!\n");
        return MAX_FILE;
    }

    file[file_number].inode = new_inode->ino;
    file[file_number].type = access;
    file[file_number].read_point = 0;
    file[file_number].write_point = 0;

    return file_number;
}

int do_fread(int fd, char *buff, int size)
{
    int read_size = 0;
    int file_number = fd;
    if(file[file_number].inode == 0 ){
        prints("The File Is Not Opened!\n");
        return read_size;
    }else if(file[file_number].type == O_WRONLY){
        prints("The File Can Not Be Read!\n");
        return read_size;
    }

    inode_t *new_inode = get_inode(file[file_number].inode);

    //7 direct_block; 3 indirect_block; 2 d_indirect_block; 1 t_indirect_block
    uint32_t read_point =  file[fd].read_point;
    uint32_t new_readpoint = read_point + size;
    uint32_t sector_number = read_point / SECTOR_SIZE;
    uint32_t sector_offset = read_point % SECTOR_SIZE;
    uint32_t data_location, data_location2, data_location3;
    uint32_t indirect, indirect2;
    uint32_t d_indirect, d_indirect2, d_indirect3;
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    uint32_t *q = (uint32_t *)kmalloc(SECTOR_SIZE);
    uint32_t *r = (uint32_t *)kmalloc(SECTOR_SIZE);

    if(new_readpoint <= DIRECT_NUM * SECTOR_SIZE){
        for(int i = sector_number; i < DIRECT_NUM; i++){
            data_location = new_inode->direct_block[i];
            sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            if(size - read_size > SECTOR_SIZE - sector_offset){
                kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                read_size += SECTOR_SIZE - sector_offset;
                sector_offset = 0;
            }else{
                kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                read_size += size - read_size;
                break;
            }
        }
    }

    else if(new_readpoint <= INDIRECT){
        if(sector_number < DIRECT_NUM){
            for(int i = sector_number; i < DIRECT_NUM; i++){
                data_location = new_inode->direct_block[i];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                if(size - read_size > SECTOR_SIZE - sector_offset){
                    kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                    read_size += SECTOR_SIZE - sector_offset;
                    sector_offset = 0;
                }else{
                    kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                    read_size += size - read_size;
                }
            }
            sector_number = DIRECT_NUM;
            sector_offset = 0;
        }

        if(sector_number >= DIRECT_NUM){
            indirect = (sector_number - DIRECT_NUM) / BLOCK_NUM;
            indirect2 = (sector_number - DIRECT_NUM) % BLOCK_NUM;
            data_location = new_inode->indirect_block[indirect];
            sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            while(read_size < size){
                data_location2 = q[indirect2];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                if(size - read_size > SECTOR_SIZE - sector_offset){
                    kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                    read_size += SECTOR_SIZE - sector_offset;
                    sector_offset = 0;
                }else{
                    kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                    read_size += size - read_size;
                }

                indirect2++;
                if(indirect2 == BLOCK_NUM){
                    indirect++;
                    indirect2 = 0;
                    data_location = new_inode->indirect_block[indirect];
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                }
            }
        }
    }

    else if(new_readpoint <= D_INDIRECT){
        if(sector_number < DIRECT_NUM){
            for(int i = sector_number; i < DIRECT_NUM; i++){
                data_location = new_inode->direct_block[i];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                if(size - read_size > SECTOR_SIZE - sector_offset){
                    kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                    read_size += SECTOR_SIZE - sector_offset;
                    sector_offset = 0;
                }else{
                    kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                    read_size += size - read_size;
                }
            }
            sector_number = DIRECT_NUM;
            sector_offset = 0;
        }

        if(sector_number < INDIRECT / SECTOR_SIZE){
            indirect = (sector_number - DIRECT_NUM) / BLOCK_NUM;
            indirect2 = (sector_number - DIRECT_NUM) % BLOCK_NUM;
            for(int i = indirect; i < INDIRECT_NUM; i++){
                data_location = new_inode->indirect_block[i];
                sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                for(int j = indirect2; j < BLOCK_NUM; j++){
                    data_location2 = q[indirect2];
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    if(size - read_size > SECTOR_SIZE - sector_offset){
                        kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                        read_size += SECTOR_SIZE - sector_offset;
                        sector_offset = 0;
                    }else{
                        kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                        read_size += size - read_size;
                    }
                }
                indirect2 = 0;
            }
            sector_number = INDIRECT / SECTOR_SIZE;
            sector_offset = 0;
        }

        if(sector_number >= INDIRECT / SECTOR_SIZE){
            d_indirect = (sector_number - INDIRECT / SECTOR_SIZE) / (BLOCK_NUM * BLOCK_NUM);
            d_indirect2 = (sector_number - INDIRECT / SECTOR_SIZE - d_indirect * BLOCK_NUM * BLOCK_NUM) / BLOCK_NUM;
            d_indirect3 = (sector_number - INDIRECT / SECTOR_SIZE - d_indirect * BLOCK_NUM * BLOCK_NUM) % BLOCK_NUM;

            data_location = new_inode->d_indirect_block[d_indirect];
            sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            while(read_size < size){
                data_location2 = q[d_indirect2];
                sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                while(read_size < size){
                    data_location3 = r[d_indirect3];
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location3);
                    if(size - read_size > SECTOR_SIZE - sector_offset){
                        kmemcpy(&buff[read_size], &p[sector_offset], SECTOR_SIZE - sector_offset);
                        read_size += SECTOR_SIZE - sector_offset;
                        sector_offset = 0;
                    }else{
                        kmemcpy(&buff[read_size], &p[sector_offset], size - read_size);
                        read_size += size - read_size;
                    }

                    d_indirect3++;
                    if(d_indirect3 == BLOCK_NUM){
                        d_indirect3 = 0;
                        d_indirect2++;
                        if(d_indirect2 == BLOCK_NUM){
                            d_indirect2 = 0;
                            d_indirect++;
                            data_location = new_inode->d_indirect_block[d_indirect];
                            sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                        }
                        data_location2 = q[d_indirect2];
                        sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    }
                }
            }
        }
    }

    file[fd].read_point += read_size;
    return read_size;
}

int do_fwrite(int fd, char *buff, int size)
{
    int write_size = 0;
    int file_number = fd;
    if(file[file_number].inode == 0){
        prints("The File Is Not Opened!\n");
        return write_size;
    }else if(file[file_number].type == O_RDONLY){
        prints("The File Can Not Be Read!\n");
        return write_size;
    }

    inode_t *new_inode = get_inode(file[file_number].inode);

    //7 direct_block; 3 indirect_block; 2 d_indirect_block; 1 t_indirect_block
    uint32_t write_point =  file[fd].write_point;
    uint32_t new_writepoint = write_point + size;
    uint32_t sector_number = write_point / SECTOR_SIZE;
    uint32_t sector_offset = write_point % SECTOR_SIZE;
    uint32_t new_sector_number = new_writepoint / SECTOR_SIZE;
    uint32_t data_location, data_location2, data_location3;
    uint32_t indirect, indirect2, d_indirect, d_indirect2, d_indirect3;
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    uint32_t *q = (uint32_t *)kmalloc(SECTOR_SIZE);
    uint32_t *r = (uint32_t *)kmalloc(SECTOR_SIZE);

    if(new_writepoint <= DIRECT){
        for(int i = sector_number; i < DIRECT_NUM; i++){
            if(sector_offset != 0){
                data_location = new_inode->direct_block[i];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }else{
                data_location = get_datablock_number();
                new_inode->direct_block[i] = data_location;
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                kbzero(p, SECTOR_SIZE);
                change_sector_bitmap(data_location, 1);
            }

            if(size - write_size > SECTOR_SIZE - sector_offset){
                kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                write_size += SECTOR_SIZE - sector_offset;
                sector_offset = 0;
                sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }else{
                kmemcpy(&p[sector_offset], &buff[write_size], size - write_size);
                write_size += size - write_size;
                sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                break;
            }
        }
    }
    else if(new_writepoint <= INDIRECT){
        if(sector_number < DIRECT_NUM){
            for(int i = sector_number; i < DIRECT_NUM; i++){
                if(sector_offset != 0){
                    data_location = new_inode->direct_block[i];
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                }else{
                    data_location = get_datablock_number();
                    new_inode->direct_block[i] = data_location;
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    kbzero(p, SECTOR_SIZE);
                    change_sector_bitmap(data_location, 1);
                }
                kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                write_size += SECTOR_SIZE - sector_offset;
                sector_offset = 0;
                sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }
            sector_number = DIRECT_NUM;
            sector_offset = 0;
        }

        if(sector_number >= DIRECT_NUM){
            indirect = (sector_number - DIRECT_NUM) / BLOCK_NUM;
            indirect2 = (sector_number - DIRECT_NUM) % BLOCK_NUM;
            if(new_inode->indirect_block[indirect] == 0){
                data_location = get_datablock_number();
                new_inode->indirect_block[indirect] = data_location;
                change_sector_bitmap(data_location, 1);
                sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                kbzero(q, SECTOR_SIZE);
            }else{
                data_location = new_inode->indirect_block[indirect];
                sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }
            
            while(write_size < size){
                if(sector_offset != 0){
                    data_location2 = q[indirect2];
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                }else{
                    data_location2 = get_datablock_number();
                    q[indirect2] = data_location2;
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    kbzero(p, SECTOR_SIZE);
                    change_sector_bitmap(data_location2, 1);
                }

                if(size - write_size > SECTOR_SIZE - sector_offset){
                    kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                    write_size += SECTOR_SIZE - sector_offset;
                    sector_offset = 0;
                    sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                }else{
                    kmemcpy(&p[sector_offset], &buff[write_size], size - write_size);
                    write_size += size - write_size;
                    sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                }

                indirect2++;
                if(indirect2 == BLOCK_NUM){
                    sbi_sd_write(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    indirect++;
                    indirect2 = 0;
                    // if(new_inode->valid_size <= DIRECT + indirect*BLOCK_NUM*SECTOR_SIZE){
                        data_location = get_datablock_number();
                        new_inode->indirect_block[indirect] = data_location;
                        change_sector_bitmap(data_location, 1);
                        sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                        kbzero(q, SECTOR_SIZE);
                    // }else{
                        // data_location = new_inode->indirect_block[indirect];
                        // sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    // }
                    
                }
            }
            sbi_sd_write(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
        }
    }
    else if(new_writepoint <= D_INDIRECT){
        if(sector_number < DIRECT_NUM){
            for(int i = sector_number; i < DIRECT_NUM; i++){
                if(sector_offset != 0){
                    data_location = new_inode->direct_block[i];
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                }else{
                    data_location = get_datablock_number();
                    new_inode->direct_block[i] = data_location;
                    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    kbzero(p, SECTOR_SIZE);
                    change_sector_bitmap(data_location, 1);
                }
                kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                write_size += SECTOR_SIZE - sector_offset;
                sector_offset = 0;
                sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }
            sector_number = DIRECT_NUM;
            sector_offset = 0;
        }

        if(sector_number < INDIRECT / SECTOR_SIZE){
            indirect = (sector_number - DIRECT_NUM) / BLOCK_NUM;
            indirect2 = (sector_number - DIRECT_NUM) % BLOCK_NUM;
            for(int ii = indirect; ii < INDIRECT_NUM; ii++){
                if(indirect2 == 0 && sector_offset == 0){
                    data_location = get_datablock_number();
                    new_inode->indirect_block[indirect] = data_location;
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                    kbzero(p, SECTOR_SIZE);
                    change_sector_bitmap(data_location, 1);
                }else{
                    data_location = new_inode->indirect_block[indirect];
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                }


                for(int jj = indirect2; jj < BLOCK_NUM; jj++){
                    if(sector_offset != 0){
                        data_location2 = q[jj];
                        sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    }else{
                        data_location2 = get_datablock_number();
                        q[jj] = data_location;
                        sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                        kbzero(p, SECTOR_SIZE);
                        change_sector_bitmap(data_location2, 1);
                    }
                    kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                    write_size += SECTOR_SIZE - sector_offset;
                    sector_offset = 0;
                    sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                }
                indirect2 = 0;
                sbi_sd_write(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }
            sector_number = INDIRECT / SECTOR_SIZE;
            sector_offset = 0;
        }

        if(sector_number >= INDIRECT / SECTOR_SIZE){
            d_indirect = (sector_number - INDIRECT / SECTOR_SIZE) / (BLOCK_NUM * BLOCK_NUM);
            d_indirect2 = (sector_number - INDIRECT / SECTOR_SIZE - d_indirect * BLOCK_NUM * BLOCK_NUM) / BLOCK_NUM;
            d_indirect3 = (sector_number - INDIRECT / SECTOR_SIZE - d_indirect * BLOCK_NUM * BLOCK_NUM) % BLOCK_NUM;
            
            if(new_inode->d_indirect_block[d_indirect] == 0){
                data_location = get_datablock_number();
                new_inode->d_indirect_block[d_indirect] = data_location;
                change_sector_bitmap(data_location, 1);
                sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                kbzero(q, SECTOR_SIZE);
            }else{
                data_location = new_inode->d_indirect_block[d_indirect];
                sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
            }
            
            while(write_size < size){
                if(d_indirect3 != 0){
                    data_location2 = q[d_indirect2];
                    sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                }else{
                    data_location2 = get_datablock_number();
                    q[d_indirect2] = data_location2;
                    sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    kbzero(r, SECTOR_SIZE);
                    change_sector_bitmap(data_location2, 1);
                }

                while(write_size < size){
                    if(sector_offset != 0){
                        data_location3 = r[d_indirect3];
                        sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location3);
                    }else{
                        data_location3 = get_datablock_number();
                        q[d_indirect3] = data_location3;
                        sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location3);
                        kbzero(p, SECTOR_SIZE);
                        change_sector_bitmap(data_location3, 1);
                    }

                    if(size - write_size > SECTOR_SIZE - sector_offset){
                        kmemcpy(&p[sector_offset], &buff[write_size], SECTOR_SIZE - sector_offset);
                        write_size += SECTOR_SIZE - sector_offset;
                        sector_offset = 0;
                        sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    }else{
                        kmemcpy(&p[sector_offset], &buff[write_size], size - write_size);
                        write_size += size - write_size;
                        sbi_sd_write(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                    }

                    d_indirect3++;
                    if(d_indirect3 == BLOCK_NUM){
                        sbi_sd_write(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                        d_indirect2++;
                        if(d_indirect2 == BLOCK_NUM){
                            sbi_sd_write(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                            d_indirect++;
                            d_indirect2 = 0;
                            data_location = get_datablock_number();
                            new_inode->d_indirect_block[d_indirect] = data_location;
                            change_sector_bitmap(data_location, 1);
                            sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
                            kbzero(p, SECTOR_SIZE);
                        }
                        d_indirect3 = 0;
                        data_location2 = get_datablock_number();
                        q[d_indirect2] = data_location2;
                        change_sector_bitmap(data_location2, 1);
                        sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location2);
                        kbzero(r, SECTOR_SIZE);
                    }

                }
            }
            sbi_sd_write(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + data_location);
        }
    }

    new_inode->valid_size += write_size;
    new_inode->modified_time = get_timer();
    write_inode(new_inode, new_inode->ino);

    file[fd].write_point += write_size;
    return write_size;
}

void do_close(int fd)
{
    file[fd].inode = 0;
}

void do_cat(char* argv)
{
    int fd = do_fopen(argv, O_RDWR);
    inode_t *new_inode = get_inode(file[fd].inode);
    int size = new_inode->valid_size;
    prints("size : %d\n", new_inode->valid_size);
    char *buff = (char *)kmalloc(size);
    int read = do_fread(fd, buff, size);
    for(int i = 0; i < read; i++)
        prints("%c", buff[i]);
    do_close(fd);
}

void do_ln(char* argv1, char* argv2)
{
    inode_t *dir = get_inode(user_location);
    int location, location2;

    for(location = 0; location < DENTRY_NUM; location++){
        if(!strcmp(argv1, dir->identry[location].name) && dir->identry[location].valid == VALID){
            break;
        }
    }
    if(location == DENTRY_NUM){
        prints("No File Named %s\n", argv1);
        return;
    }

    for(location2 = 0; location2 < DENTRY_NUM; location2++){
        if(!strcmp(argv2, dir->identry[location2].name) && dir->identry[location2].valid == VALID){
            prints("%s Has Existed!\n", argv2);
            return;
        }
    }
    for(location2 = 0; location2 < DENTRY_NUM; location2++){
        if(dir->identry[location2].valid == INVALID){
            break;
        }
    }
    
    inode_t* old_file = get_inode(dir->identry[location].ino);
    int inode = old_file->ino;

    dir->valid_size += DENTRY_SIZE;
    dir->identry[location2].ino = inode;
    dir->identry[location2].mode = FILE;
    dir->identry[location2].valid = VALID;
    dir->modified_time = get_timer();
    kmemcpy(dir->identry[location2].name, argv2, kstrlen(argv2));
    write_inode(dir, dir->ino);

    old_file->link++;
    old_file->modified_time = get_timer();
    write_inode(old_file, old_file->ino);
}

void do_rm(char* argv)
{
    inode_t *dir = get_inode(user_location);
    int location;

    for(location = 0; location < DENTRY_NUM; location++){
        if(!strcmp(argv, dir->identry[location].name) && dir->identry[location].valid == VALID){
            break;
        }
    }
    if(location == DENTRY_NUM){
        prints("No File Named %s\n", argv);
        return;
    }
    
    inode_t* del_file = get_inode(dir->identry[location].ino);
    int inode = del_file->ino;
    int size = del_file->valid_size;
    int i = 0, j = 0, k = 0;
    uint32_t *p = (uint32_t *)kmalloc(SECTOR_SIZE);
    uint32_t *q = (uint32_t *)kmalloc(SECTOR_SIZE);
    uint32_t *r = (uint32_t *)kmalloc(SECTOR_SIZE);
    uint32_t indirect, number, number2, number3, d_indirect, t_indirect;

    if(del_file->link > 1){
        del_file->link--;
        del_file->modified_time = get_timer();
        write_inode(del_file, del_file->ino);
    }else{
        if(size <= DIRECT){
            while(size > 0){
                change_sector_bitmap(del_file->direct_block[i], 0);
                size -= SECTOR_SIZE;
                i++;
            }
        }
        else if(size <= INDIRECT){
            for(int ii = 0; ii < DIRECT_NUM; ii++){
                change_sector_bitmap(del_file->direct_block[ii], 0);
                size -= SECTOR_SIZE;
            }
            while(size > 0){
                indirect = del_file->indirect_block[i];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + indirect);
                j = 0;
                while(size > 0 && j < SECTOR_SIZE / 4){
                    number = p[j];
                    change_sector_bitmap(number, 0);
                    size -= SECTOR_SIZE;
                }
                change_sector_bitmap(indirect, 0);
                i++;
            }
        }
        else if(size <= D_INDIRECT){
            for(int ii = 0; ii < DIRECT_NUM; ii++){
                change_sector_bitmap(del_file->direct_block[ii], 0);
                size -= SECTOR_SIZE;
            }
            for(int ii = 0; ii < INDIRECT_NUM; ii++){
                indirect = del_file->indirect_block[ii];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + indirect);
                for(int jj = 0; jj < SECTOR_SIZE / 4; jj++){
                    number = p[j];
                    change_sector_bitmap(number, 0);
                    size -= SECTOR_SIZE;
                }
                change_sector_bitmap(indirect, 0);
            }
            while(size > 0){
                d_indirect = del_file->d_indirect_block[i];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + d_indirect);
                j = 0;
                while(size > 0 && j < SECTOR_SIZE / 4){
                    number = p[j];
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + number);
                    k = 0;
                    while(size > 0 && k < SECTOR_SIZE / 4){
                        number2 = q[k];
                        change_sector_bitmap(number2, 0);
                        size -= SECTOR_SIZE;
                    }
                    change_sector_bitmap(number, 0);
                    j++;
                }
                change_sector_bitmap(d_indirect, 0);
                i++;
            }
        }
        else if(size <= T_INDIRECT){
            for(int ii = 0; ii < DIRECT_NUM; ii++){
                change_sector_bitmap(del_file->direct_block[ii], 0);
                size -= SECTOR_SIZE;
            }
            for(int ii = 0; ii < INDIRECT_NUM; ii++){
                indirect = del_file->indirect_block[ii];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + indirect);
                for(int jj = 0; jj < SECTOR_SIZE / 4; jj++){
                    number = p[jj];
                    change_sector_bitmap(number, 0);
                    size -= SECTOR_SIZE;
                }
                change_sector_bitmap(indirect, 0);
            }
            for(int ii = 0; ii < D_INDIRECT_NUM; ii++){
                d_indirect = del_file->d_indirect_block[ii];
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + indirect);
                for(int jj = 0; jj < SECTOR_SIZE / 4; jj++){
                    number = p[jj];
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + number);
                    for(int kk = 0; kk < SECTOR_SIZE / 4; kk++){
                        number2 = q[kk];
                        change_sector_bitmap(number2, 0);
                        size -= SECTOR_SIZE;
                    }
                    change_sector_bitmap(number, 0);
                }
                change_sector_bitmap(d_indirect, 0);
            }
            while(size > 0){
                t_indirect = del_file->t_indirect_block;
                sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + t_indirect);
                j = 0;
                while(size > 0 && j < SECTOR_SIZE / 4){
                    number = p[j];
                    sbi_sd_read(kva2pa(q), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + number);
                    k = 0;
                    while(size > 0 && k < SECTOR_SIZE / 4){
                        number2 = q[k];
                        sbi_sd_read(kva2pa(r), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + number2);
                        i = 0;
                        while(size > 0 && i < SECTOR_SIZE / 4){
                            number3 = r[i];
                            change_sector_bitmap(number3, 0);
                            size -= SECTOR_SIZE;
                        }
                        change_sector_bitmap(number2, 0);
                    }
                    change_sector_bitmap(number, 0);
                    j++;
                }
                change_sector_bitmap(t_indirect, 0);
            }
        }
    }

    dir->modified_time = get_timer();
    kbzero(dir->identry[location].name, MAX_FILE_LENGTH);
    dir->identry[location].valid = INVALID;
    write_inode(dir, dir->ino);
}

int do_lseek(int fd,int offset,int whence)
{
    inode_t* new_inode = get_inode(file[fd].inode);
    int size = new_inode->valid_size;
    if(whence == 0){
        file[fd].read_point = offset;
        file[fd].write_point = offset;
    }else if(whence == 1){
        file[fd].read_point += offset;
        file[fd].write_point += offset;
    }else if(whence == 2){
        file[fd].read_point = size + offset;
        file[fd].write_point = size + offset;
    }
    int max = (file[fd].read_point > file[fd].write_point)? file[fd].read_point : file[fd].write_point;
    if(max > size){
        new_inode->valid_size = max;
        write_inode(new_inode, new_inode->ino);
    }
    return 0;
}

inode_t* get_inode(int location)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    inode_t *inode = (inode_t *)p;
    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + INODEBLOCK_SECTOR_OFFSET + location);
    return inode;
}

int get_inode_number(void)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE * INODEMAP_SIZE);
    sbi_sd_read(kva2pa(p), INODEMAP_SIZE, FIRST_SECTOR + INODEMAP_SECTOR_OFFSET);
    int inode_number = 0;
    for (int i = 0; i < SECTOR_SIZE * INODEMAP_SIZE; i++){
        if(p[i] == 0xff)
            inode_number += 8;
        else{
            for(int j = 0; j < 8 ; j++){
                if(p[i] % 2){
                    inode_number++;
                }else{
                    return inode_number;
                }
                p[i] /= 2;
            }
        }
    }
    return -1;
}

int get_datablock_number(void)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE * SECTORMAP_SIZE);
    sbi_sd_read(kva2pa(p), SECTORMAP_SIZE, FIRST_SECTOR + SECTORMAP_SECTOR_OFFSET);
    int datablock_number = 0;
    for (int i = 0; i < SECTOR_SIZE * SECTORMAP_SIZE; i++){
        if(p[i] == 0xff)
            datablock_number += 8;
        else{
            for(int j = 0; j < 8 ; j++){
                if(p[i] % 2){
                    datablock_number++;
                }else{
                    return datablock_number;
                }
                p[i] /= 2;
            }
        }
    }
    return -1;
}

uint8_t* get_datablock(int location)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE);
    sbi_sd_read(kva2pa(p), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + location);
    return p;
}

int get_file_number(void)
{
    int i;
    for(i = 0; i < MAX_FILE; i++){
        if(file[i].inode == 0)
            break;
    }
    return i;
}

void write_inode(inode_t* inode, int location)
{
    sbi_sd_write(kva2pa(inode), 1, FIRST_SECTOR + INODEBLOCK_SECTOR_OFFSET + location);
}

void write_datablock(uint8_t* datablock, int location)
{
    sbi_sd_write(kva2pa(datablock), 1, FIRST_SECTOR + DATABLOCK_SECTOR_OFFSET + location);
}

void change_inode_bitmap(int location, int change_number)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE * INODEMAP_SIZE);
    sbi_sd_read(kva2pa(p), INODEMAP_SIZE, FIRST_SECTOR + INODEMAP_SECTOR_OFFSET);
    int a = location / 8;
    int b = location % 8;
    if(!change_number){
        uint8_t c = 1 << b;
        p[a] = p[a] & ~c;
    }else{
        uint8_t c = 1 << b;
        p[a] = c | p[a] & ~c;
    }
    sbi_sd_write(kva2pa(p), INODEMAP_SIZE, FIRST_SECTOR + INODEMAP_SECTOR_OFFSET);
}

void change_sector_bitmap(int location, int change_number)
{
    uint8_t *p = (uint8_t *)kmalloc(SECTOR_SIZE * SECTORMAP_SIZE);
    sbi_sd_read(kva2pa(p), SECTORMAP_SIZE, FIRST_SECTOR + SECTORMAP_SECTOR_OFFSET);
    int a = location / 8;
    int b = location % 8;
    if(!change_number){
        uint8_t c = 1 << b;
        p[a] = p[a] & ~c;
    }else{
        uint8_t c = 1 << b;
        p[a] = c | p[a] & ~c;
    }
    sbi_sd_write(kva2pa(p), SECTORMAP_SIZE, FIRST_SECTOR + SECTORMAP_SECTOR_OFFSET);
}

inode_t* relocation(inode_t* inode, char* argv, int mode)
{
    inode_t* new_inode = inode;
    char array[MAX_FILE_LENGTH];
    int size, p = 0, i, j;
    while(p != kstrlen(argv)){
        i = 0;
        size = new_inode->valid_size / DENTRY_SIZE;
        kbzero(array, MAX_FILE_LENGTH);
        while(argv[p] != '/' && argv[p] != '\0'){
            array[i] = argv[p];
            i++;p++;
        }
        if(argv[p] == '\0' && mode == MKDIR)
            break;
        if(argv[p] == '/')
            p++;
        for(j = 0; j < DENTRY_NUM; j++){
            if(!strcmp(array, new_inode->identry[j].name) && new_inode->identry[j].valid == VALID){
                new_inode = get_inode(new_inode->identry[j].ino);
                break;
            }
        }
        if(j == size)
            return NULL;
    }
    if(mode == MKDIR){
        for(j = 0; j < DENTRY_NUM; j++){
            if(!strcmp(array, new_inode->identry[j].name) && new_inode->identry[j].valid == VALID){
                return NULL;
            }
        }
        int inode_number = get_inode_number();
        int parent_inode = new_inode->ino;
        for(size = 0; size < DENTRY_NUM; size++)
            if(new_inode->identry[size].valid == INVALID)
                break;
        new_inode->valid_size += DENTRY_SIZE;
        new_inode->identry[size].ino = inode_number;
        new_inode->identry[size].mode = DIR;
        new_inode->identry[size].valid = VALID;
        new_inode->modified_time = get_timer();
        kmemcpy(new_inode->identry[size].name, array, i);
        write_inode(new_inode, new_inode->ino);
        new_inode = get_inode(inode_number);
        new_inode->ino = inode_number;
        new_inode->mode = DIR;
        new_inode->type = O_RDWR;
        new_inode->link = 0;
        new_inode->size = SECTOR_SIZE;
        new_inode->valid_size = 2 * DENTRY_SIZE;
        new_inode->modified_time = get_timer();
        new_inode->identry[0].ino = parent_inode;
        new_inode->identry[1].ino = inode_number;
        new_inode->identry[0].valid = VALID;
        new_inode->identry[1].valid = VALID;
        new_inode->identry[0].mode = DIR;
        new_inode->identry[1].mode = DIR;
        kbzero(new_inode->identry[0].name, MAX_FILE_LENGTH);
        kbzero(new_inode->identry[1].name, MAX_FILE_LENGTH);
        kmemcpy(new_inode->identry[0].name, ".", 1);
        kmemcpy(new_inode->identry[1].name, "..", 2);
        for(int i = 2; i < DENTRY_NUM; i++)
            new_inode->identry[i].valid = INVALID;
        write_inode(new_inode, new_inode->ino);
        change_inode_bitmap(inode_number, 1);
    }
    return new_inode;
}