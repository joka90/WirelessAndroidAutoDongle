################################################################################
#
# bpftool
#
################################################################################

BPFTOOL_CUSTOM_VERSION = v7.4.0
BPFTOOL_CUSTOM_SITE = https://github.com/libbpf/bpftool
BPFTOOL_CUSTOM_SITE_METHOD = git
BPFTOOL_CUSTOM_GIT_SUBMODULES = YES
BPFTOOL_CUSTOM_LICENSE = GPL-2.0, BSD-2-Clause
BPFTOOL_CUSTOM_LICENSE_FILES = LICENSE LICENSE.BSD-2-Clause LICENSE.GPL-2.0
BPFTOOL_CUSTOM_DEPENDENCIES = binutils elfutils
HOST_BPFTOOL_CUSTOM_DEPENDENCIES = host-elfutils host-pkgconf host-zlib

ifeq ($(BR2_PACKAGE_LIBCAP),y)
BPFTOOL_CUSTOM_DEPENDENCIES += libcap
endif

ifeq ($(BR2_PACKAGE_ZLIB),y)
BPFTOOL_CUSTOM_DEPENDENCIES += zlib
endif

define BPFTOOL_CUSTOM_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) \
		-C $(@D)/src
endef

define HOST_BPFTOOL_CUSTOM_BUILD_CMDS
	$(HOST_MAKE_ENV) $(HOST_CONFIGURE_OPTS) $(MAKE) \
		-C $(@D)/src
endef

define BPFTOOL_CUSTOM_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) \
		-C $(@D)/src install-bin DESTDIR="$(TARGET_DIR)" prefix=/usr
endef

define HOST_BPFTOOL_CUSTOM_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(HOST_CONFIGURE_OPTS) $(MAKE) \
		-C $(@D)/src install-bin DESTDIR="$(HOST_DIR)" prefix=
endef

$(eval $(generic-package))
$(eval $(host-generic-package))
