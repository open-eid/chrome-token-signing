/*
* SMARTCARDPP
* 
* This software is released under either the GNU Library General Public
* License (see LICENSE.LGPL) or the BSD License (see LICENSE.BSD).
* 
* Note that the only valid version of the LGPL license as far as this
* project is concerned is the original GNU Library General Public License
* Version 2.1, February 1999
*
*/




#include "common.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <string.h>
#endif

void * doAlloc(size_t n, void* /*hint*/) {
	void * ret = new unsigned char[n];
    if (!ret)
            throw std::bad_alloc();
#ifdef WIN32
	VirtualLock(ret,n);
#else
	mlock(ret,n);
#endif
	return ret;
	}

void doFree(void *pmem,size_t n) {
#ifdef WIN32
	VirtualUnlock(pmem,n);
#else
	munlock(pmem,n);
#endif
	memset(pmem,0,n);
	unsigned char * cp = (unsigned char *)pmem;
	delete [] cp;
	}
