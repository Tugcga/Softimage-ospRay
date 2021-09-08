
#ifndef OSPRAY_TESTING_EXPORT_H
#define OSPRAY_TESTING_EXPORT_H

#ifdef OSPRAY_TESTING_STATIC_DEFINE
#  define OSPRAY_TESTING_EXPORT
#  define OSPRAY_TESTING_NO_EXPORT
#else
#  ifndef OSPRAY_TESTING_EXPORT
#    ifdef ospray_testing_EXPORTS
        /* We are building this library */
#      define OSPRAY_TESTING_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define OSPRAY_TESTING_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef OSPRAY_TESTING_NO_EXPORT
#    define OSPRAY_TESTING_NO_EXPORT 
#  endif
#endif

#ifndef OSPRAY_TESTING_DEPRECATED
#  define OSPRAY_TESTING_DEPRECATED __declspec(deprecated)
#endif

#ifndef OSPRAY_TESTING_DEPRECATED_EXPORT
#  define OSPRAY_TESTING_DEPRECATED_EXPORT OSPRAY_TESTING_EXPORT OSPRAY_TESTING_DEPRECATED
#endif

#ifndef OSPRAY_TESTING_DEPRECATED_NO_EXPORT
#  define OSPRAY_TESTING_DEPRECATED_NO_EXPORT OSPRAY_TESTING_NO_EXPORT OSPRAY_TESTING_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef OSPRAY_TESTING_NO_DEPRECATED
#    define OSPRAY_TESTING_NO_DEPRECATED
#  endif
#endif

#endif /* OSPRAY_TESTING_EXPORT_H */
