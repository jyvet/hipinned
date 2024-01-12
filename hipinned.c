/**
* HIP Pinned (HIPinned): Intercept memory allocations and automatically
*                        register large regions (aka pinned memory) with HIP.
* URL       https://github.com/jyvet/hipinned
* License   MIT
* Copyright (c) 2024
******************************************************************************/
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <errno.h>
#include <hip/hip_runtime.h>

#define BYTES_THRES_DEFAULT 1048576 /* 1 MB */

static bool   is_init = false;
static size_t bytes_thres = BYTES_THRES_DEFAULT;
static void*  (*real_malloc)(size_t) = NULL;
static int    (*real_posix_memalign)(void **, size_t, size_t) = NULL;
static int    (*real_libc_start_main)(int (*main) (int,char **,char **),
                                      int argc, char **ubp_av,
                                      void (*init) (void),
                                      void (*fini)(void),
                                      void (*rtld_fini)(void),
                                      void (*stack_end)) = NULL;

static void _init(void)
{
    real_libc_start_main = dlsym(RTLD_NEXT, "__libc_start_main");
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");

    is_init = true;
}

static void _hip_register(void *ptr, size_t size)
{
    if (size < bytes_thres)
        return;

    hipError_t ret = hipHostRegister(ptr, size, hipHostRegisterMapped);
    if (ret != hipSuccess)
        fprintf(stderr,"Warning: hipHostRegister failed (%s)\n", hipGetErrorString(ret));
    else
        fprintf(stderr, "Registered region [%p, %ld bytes] as pinned memory\n", ptr, size);
}

void *malloc(size_t size)
{
    if (!is_init)
        _init();

    void *ret = real_malloc(size);
    if (size >= bytes_thres && ret != NULL)
        _hip_register(ret, size);

    return ret;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    if (!is_init)
        _init();

    int ret = real_posix_memalign(memptr, alignment, size);
    if (size >= bytes_thres && ret == 0)
        _hip_register(*memptr, size);

    return ret;
}

int __libc_start_main(int (*main) (int,char **,char **),
              int argc,char **ubp_av,
              void (*init) (void),
              void (*fini)(void),
              void (*rtld_fini)(void),
              void (*stack_end))
{
    if (!is_init)
        _init();

    char *env_tmp = getenv("HIPINNED_BYTES_THRES_ENV");
    if (env_tmp != NULL)
    {
        bytes_thres = strtol(env_tmp, NULL, 10);
        if (errno == EINVAL || errno == ERANGE || bytes_thres < 0)
        {
            fprintf(stderr, "Error: cannot parse HIPINNED_BYTES_THRES_ENV (%s). Exit.\n", env_tmp);
            exit(EXIT_FAILURE);
        }
    }

    printf("HIPinned loaded (threshold: %ld bytes)\n", bytes_thres);

    return real_libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

