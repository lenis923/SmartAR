
# confirm if NDK_ROOT is specified
ifeq ($(NDK_ROOT),)
    $(error you should set NDK_ROOT. abort...)
endif

all : jni/Android.mk
	$(NDK_ROOT)/ndk-build V=0 -j 5 $@

clean :
	$(NDK_ROOT)/ndk-build V=0 clean
	rm -r obj
