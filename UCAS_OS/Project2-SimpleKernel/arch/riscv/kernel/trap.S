#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TODO: 
   * save exception_handler_entry into STVEC
   * enable global exceptions */
  la t0, exception_handler_entry 
  csrw stvec, t0
  li t0, SIE_SEIE
  addi t0, t0, SIE_STIE
  addi t0, t0, SIE_SSIE
  csrw sie, t0
  jr ra

ENDPROC(setup_exception)
