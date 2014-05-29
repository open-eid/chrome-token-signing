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

#include <cstddef>
#include <memory>

template <class T> class locked_allocator;

//void specialization
template <>class locked_allocator<void> {
public:
  typedef void*       pointer;
  typedef const void* const_pointer;
  // reference-to-void members are impossible
  typedef void        value_type;

  template <class U>
  struct rebind
  {
	 typedef locked_allocator<U> other;
  };
};

void * doAlloc(size_t num, void * hint);
void doFree(void * ptr,size_t num);

/// Allocator template that locks pages and cleans up free()'s
/// see http://www.ddj.com/cpp/184401646 for more complete version
template<class T>
class locked_allocator
{
public:
	typedef T			value_type;
	typedef T    *pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;
	pointer	adress(reference x) const
	{
		return &x;
	}
	const_pointer	adress(const_reference x) const
	{
		return &x;
	}
	void	construct(pointer p, const T &val)
	{
		new ((void*)p) T(val);
	}
	void	destroy(pointer p)
	{
		p->T::~T();
	}

	explicit locked_allocator() {}
	locked_allocator(const locked_allocator<T> &) {}
	template<class U> locked_allocator(const locked_allocator<U> &) {}
	~locked_allocator() {}
	pointer	allocate(size_type n, void* hint= NULL)
	{
		void * pmem = doAlloc(n * sizeof (T),hint);
		return static_cast<pointer>(pmem);
	}
	void deallocate(void *p, size_type n)
	{
		doFree(p,n);
		return;
	}

	template <class U>
	struct rebind {
		typedef locked_allocator<U> other;
	};

	size_type max_size () const
	{
		std::allocator<T> al;
		return al.max_size();
	}

	template <class U>
	locked_allocator<T>& operator=(const locked_allocator<U> &) { return *this; }
};

template <class T>
bool operator==(const locked_allocator<T> &,const locked_allocator<T> &) { return true;}
template <class T>
bool operator!=(const locked_allocator<T> &,const locked_allocator<T> &) { return false;}
