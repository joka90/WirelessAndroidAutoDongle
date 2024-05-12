################################################################################
#
# bpftrace
#
################################################################################

BPFTRACE_CUSTOM_VERSION = 0.19.1
BPFTRACE_CUSTOM_SITE = $(call github,iovisor,bpftrace,v$(BPFTRACE_CUSTOM_VERSION))
BPFTRACE_CUSTOM_LICENSE = Apache-2.0
BPFTRACE_CUSTOM_LICENSE_FILES = LICENSE
BPFTRACE_CUSTOM_DEPENDENCIES = \
	bcc-backport \
	bzip2 \
	cereal \
	elfutils \
	host-bison \
	host-flex \
	libbpf-custom \
	llvm \
	xz

# libbfd, libopcodes
ifeq ($(BR2_PACKAGE_BINUTILS),y)
BPFTRACE_CUSTOM_DEPENDENCIES += binutils
endif

BPFTRACE_CUSTOM_CONF_OPTS += \
	-DBUILD_SHARED_LIBS:BOOL=OFF \
	-DBUILD_TESTING:BOOL=OFF \
	-DENABLE_MAN:BOOL=OFF \
	-DINSTALL_TOOL_DOCS:BOOL=OFF \
	-DUSE_SYSTEM_BPF_BCC:BOOL=ON

$(eval $(cmake-package))
