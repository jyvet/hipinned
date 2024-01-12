libhipinned.so:
	hipcc -O3 -D__HIP_PLATFORM_AMD__ -fpic -shared hipinned.c -ldl -o libhipinned.so

all: libhipinned.so

clean:
	rm libhipinned.so

