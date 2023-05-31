#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

void *ioremap(unsigned long phys_addr, unsigned long size)
{
    // map phys_addr to a virtual address
    // then return the virtual address
    uint64_t pgdir = pa2kva(PGDIR_PA);
    uint64_t vaddr = io_base;
    while (size)
    {
        ptr_t new;
        uint64_t va = io_base & VA_MASK;
        uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
        uint64_t vpn1 = (vpn2 << PPN_BITS) ^ (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
        uint64_t vpn0 = ((vpn2 << (PPN_BITS + PPN_BITS)) | (vpn1 << PPN_BITS)) ^ (va >> NORMAL_PAGE_SHIFT);

        PTE *pgdir_vpn2 = (PTE *)pgdir;
        if (pgdir_vpn2[vpn2] % 2 == 0)
        {
            new = allocPage(1);
            set_pfn(&pgdir_vpn2[vpn2], kva2pa(new) >> NORMAL_PAGE_SHIFT);
            set_attribute(&pgdir_vpn2[vpn2], _PAGE_PRESENT);
        }
        else
        {
            new = pa2kva(pgdir_vpn2[vpn2] >> 10 << 12);
        }

        PTE *pgdir_vpn1 = (PTE *)new;
        if (pgdir_vpn1[vpn1] % 2 == 0)
        {
            new = allocPage(1);
            set_pfn(&pgdir_vpn1[vpn1], kva2pa(new) >> NORMAL_PAGE_SHIFT);
            set_attribute(&pgdir_vpn1[vpn1], _PAGE_PRESENT);
        }
        else
        {
            new = pa2kva(pgdir_vpn1[vpn1] >> 10 << 12);
        }

        PTE *pgdir_vpn0 = (PTE *)new;
        set_pfn(&pgdir_vpn0[vpn0], phys_addr >> 12);
        set_attribute(&pgdir_vpn0[vpn0], _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_PRESENT | _PAGE_DIRTY);

        io_base = io_base + NORMAL_PAGE_SIZE;
        phys_addr = phys_addr + NORMAL_PAGE_SIZE;
        size = size - NORMAL_PAGE_SIZE;
    }
    local_flush_tlb_all();
    return vaddr;
}

void iounmap(void *io_addr)
{
    // TODO: a very naive iounmap() is OK
    // maybe no one would call this function?
    // *pte = 0;
}
