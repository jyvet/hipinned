libhipinned.so:
	hipcc -O3 -D__HIP_PLATFORM_AMD__ -fpic -shared intercept.c -ldl -o libhipinned.so

all: libhipinned.so

clean:
	rm libhipinned.so

