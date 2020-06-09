/* connector for sbrk */

#include <reent.h>
#include <unistd.h>

extern void *_sbrk_r (struct _reent *, size_t);
extern void *_sbrk (size_t);

void *
sbrk (incr)
     size_t incr;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
//  Log("_sbrk_r\n");
  return _sbrk_r (_REENT, incr);
#else
//  Log("sbrk\n");
  return _sbrk (incr);
#endif
}
