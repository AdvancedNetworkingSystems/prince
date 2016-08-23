include $(TOPDIR)/rules.mk

PKG_NAME:=poprouting
PKG_VERSION:=v0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/poprouting-$(PKG_VERSION)
PKG_USE_MIPS16:=0 
PKG_SOURCE_PROTO:=git
PKG_SOURCE:=poprouting-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/gabri94/poprouting.git
PKG_SOURCE_VERSION:=master
PKG_SOURCE_SUBDIR:=poprouting-v0.1
include $(INCLUDE_DIR)/package.mk
define Package/poprouting
	SECTION :=net
	CATEGORY :=Network
	SUBMENU :=Routing and Redirection
	Mantainer :=Gabriele Gemmi <gabriel@autistici.org>
	TITLE :=PopRouting daemon
	MENU :=1
	DEPENDS := +boost-graph +libjson-c +libstdcpp
endef

define Package/poprouting/description
	TODO
endef

define Build/Prepare
        $(Build/Prepare/Default)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/graph-parser/build
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/graph-parser/build/lib
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/prince/build
endef


define Package/poprouting/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/graph-parser/build/lib/libgraphparser.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/prince/build/prince $(1)/usr/sbin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/prince/build/libprince_olsr.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/prince/build/libprince_oonf.so $(1)/usr/lib/
endef
$(eval $(call BuildPackage,poprouting))

