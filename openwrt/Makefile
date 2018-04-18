include $(TOPDIR)/rules.mk

PKG_NAME:=prince
PKG_VERSION:=v0.3.3
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

# out of source build
CMAKE_BINARY_SUBDIR:=build

PKG_BUILD_DIR:=$(BUILD_DIR)/prince-$(PKG_VERSION)
PKG_INSTALL:=1
PKG_SOURCE:=prince-$(PKG_VERSION).tar.gz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=prince-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/AdvancedNetworking/poprouting.git
PKG_SOURCE_VERSION:=v0.3.3
PKG_USE_MIPS16:=0

define Package/prince
	SECTION :=net
	CATEGORY :=Network
	SUBMENU :=Routing and Redirection
	Mantainer :=Gabriele Gemmi <gabriel@autistici.org>
	TITLE :=PopRouting daemon
	URL :=https://github.com/gabri94/poprouting
	MENU :=0
	DEPENDS := +libjson-c +libpthread +ldl +lm
endef

define Package/prince/description
  Prince is an open source implementation of the PopRouting Algorithm.
  It has been developed as a Google Summer of Code Project in collaboration
  with Freifunk and the University of Trento.

  It work by fetching topology data from a Link State routing demon(OONF,
  OLSR, OSPF, etc), calculating the betweenness centrality using the topology,
  then using these data the timer's value are optimized. Finally the timers are
  pushed back to the routing demon.

  Currently it only supports OLSRd2 (aka OONF).
endef

define Build/prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r .. $(PKG_BUILD_DIR)
endef

$(eval $(call BuildPackage,prince))
