HIP Pinned Lib (HIPinned)
=========================

This library is designed to intercept memory allocations and automatically
register large regions (aka pinned memory) with HIP.


Dependencies
------------

* **AMD ROCm** (HIP)


How to build HIPinned
---------------------

    % module load rocm
    % make


How to run ECounter in standalone mode
--------------------------------------

    LD_PRELOAD=<absolute_path_to_libhipinned> ./<your_application>

By default all allocations lerger or equal to 1MB will be registered.
Set the HIPINNED_BYTES_THRES_ENV environment variable to adjust this threshold.
