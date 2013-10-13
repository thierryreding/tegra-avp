#ifndef LINKAGE_H
#define LINKAGE_H

#ifndef asmlinkage
#define asmlinkage
#endif

#ifndef ALIGN
#define ALIGN			\
  .align 0
#endif

#ifndef ENTRY
#define ENTRY(name)		\
  .globl name;			\
  ALIGN;			\
  name:
#endif

#ifndef END
#define END(name)		\
  .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name)		\
  .type name, %function;	\
  END(name)
#endif

#endif
