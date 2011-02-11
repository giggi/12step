#ifndef _H_DEBUG_INCLUDED_
#define _H_DEBUG_INCLUDED_

#ifdef __ENABLE_DEBUG__

#define DBG(s) do{puts("DBG: ");puts(#s);puts("\n");}while(0)

#else/*__ENABLE_DEBUG__*/

#define DBG(s) do{}while(0)

#endif/*__ENABLE_DEBUG__*/

#endif /* _H_DEBUG_INCLUDED_ */
