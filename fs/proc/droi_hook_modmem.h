#ifndef __PROCFS_DROI_MEM_H__
#define __PROCFS_DROI_MEM_H__
struct modram {
    unsigned long   fake_ram;
    unsigned long   used_ram_plus;
    unsigned long   slab_plus;
    unsigned long   pagetables_plus;
    unsigned long   cached_plus;
    unsigned long   free_plus;
    unsigned long   ram_added;
};

struct droiram {
    struct mutex    lock;
    unsigned long   fake_ram;
    unsigned long   used_ram_plus;
};

extern void droi_hook_ram(struct modram *dram);

#endif /* __PROCFS_DROI_MEM_H__*/
