# app makefile
MY_TARGETDIR := $(APPS_BUILD_DIR)/rld
MY_SRCDIR := $(APPS_DIR)/rld
MY_TARGET :=  $(MY_TARGETDIR)/rld.so
ifeq ($(call FINDINLIST,$(MY_TARGET),$(ALL)),1)

MY_SRCS := \
	rld0.c \
	rld.c \
	rldelf.c \
	rldunix.c \
	rldbeos.c \
	rldheap.c \
	rldaux.c

MY_INCLUDES := $(STDINCLUDE)
MY_CFLAGS := $(USER_CFLAGS)
MY_LIBS := $(LIBC_STATIC) $(LIBNEWOS_STATIC)
MY_LIBPATHS :=
MY_DEPS :=
MY_LINKSCRIPT := $(MY_SRCDIR)/arch/$(ARCH)/rld.ld
MY_GLUE :=

include templates/app.mk

endif

