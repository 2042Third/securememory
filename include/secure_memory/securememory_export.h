
#ifndef SECUREMEMORY_API_H
#define SECUREMEMORY_API_H

#ifdef SECUREMEMORY_STATIC_DEFINE
#  define SECUREMEMORY_API
#  define SECUREMEMORY_NO_EXPORT
#else
#  ifndef SECUREMEMORY_API
#    ifdef SecureMemory_EXPORTS
        /* We are building this library */
#      define SECUREMEMORY_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define SECUREMEMORY_API __declspec(dllimport)
#    endif
#  endif

#  ifndef SECUREMEMORY_NO_EXPORT
#    define SECUREMEMORY_NO_EXPORT 
#  endif
#endif

#ifndef SECUREMEMORY_DEPRECATED
#  define SECUREMEMORY_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SECUREMEMORY_DEPRECATED_EXPORT
#  define SECUREMEMORY_DEPRECATED_EXPORT SECUREMEMORY_API SECUREMEMORY_DEPRECATED
#endif

#ifndef SECUREMEMORY_DEPRECATED_NO_EXPORT
#  define SECUREMEMORY_DEPRECATED_NO_EXPORT SECUREMEMORY_NO_EXPORT SECUREMEMORY_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SECUREMEMORY_NO_DEPRECATED
#    define SECUREMEMORY_NO_DEPRECATED
#  endif
#endif

#endif /* SECUREMEMORY_API_H */
