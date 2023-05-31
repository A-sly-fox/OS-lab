#include <os/mm.h>
#include <pgtable.h>

ptr_t memCurr = FREEMEM;

ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE);
    memCurr = ret + numPage * PAGE_SIZE;
    return ret;
}

void* kmalloc(size_t size)
{
    ptr_t ret = ROUND(memCurr, 4);
    memCurr = ret + size;
    return (void*)ret;
}
uintptr_t shm_page_get(int key)
{
    // TODO(c-core):
}

void shm_page_dt(uintptr_t addr)
{
    // TODO(c-core):
}

/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO:
    uint64_t len = 0x1000;
    kmemcpy((uint8_t *)dest_pgdir, (uint8_t *)src_pgdir, len);
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page.
   */
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir)
{
    // TODO:
    ptr_t new;
    PTE *second_level_pgdir, *last_level_pgdir;
    uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
    uint64_t vpn1 = (vpn2 << PPN_BITS) ^ (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
    uint64_t vpn0 = ((vpn2 << (PPN_BITS + PPN_BITS)) | (vpn1 << PPN_BITS)) ^ (va >> NORMAL_PAGE_SHIFT);

    PTE *pgdir_vpn2 = (PTE *)pgdir;
    if (pgdir_vpn2[vpn2] % 2 == 0) {
        new = allocPage(1);
        // pgdir_vpn2[vpn2] = (kva2pa(second_level_pgdir) >> NORMAL_PAGE_SHIFT) << 10;
        set_pfn(&pgdir_vpn2[vpn2], kva2pa(new) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pgdir_vpn2[vpn2], _PAGE_PRESENT | _PAGE_USER);
    }else{
        new = pa2kva(pgdir_vpn2[vpn2] >> 10 << 12);
    }

    second_level_pgdir = (PTE *)new;
    if (second_level_pgdir[vpn1] % 2 == 0) {
        new = allocPage(1);
        // second_level_pgdir[vpn1] = (kva2pa(last_level_pgdir) >> NORMAL_PAGE_SHIFT) << 10;
        set_pfn(&second_level_pgdir[vpn1], kva2pa(new) >> NORMAL_PAGE_SHIFT);
        set_attribute(&second_level_pgdir[vpn1], _PAGE_PRESENT | _PAGE_USER);
    }else{
        new = pa2kva(second_level_pgdir[vpn1] >> 10 << 12);
    }

    last_level_pgdir = (PTE *)new;
    new = allocPage(1);
    set_pfn(&last_level_pgdir[vpn0], kva2pa(new) >> NORMAL_PAGE_SHIFT);
    set_attribute(&last_level_pgdir[vpn0], _PAGE_PRESENT | _PAGE_USER | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY);
    return new;
}
