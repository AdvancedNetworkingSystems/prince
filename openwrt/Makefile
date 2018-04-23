include $(TOPDIR)/rules.mk

PKG_NAME:=prince
PKG_VERSION:=v0.4alpha2
PKG_RELEASE:=1

# out of source build
CMAKE_BINARY_SUBDIR:=build

PKG_BUILD_DIR:=$(BUILD_DIR)/prince-$(PKG_VERSION)
PKG_INSTALL:=1
PKG_LICENSE:=MIT
PKG_SOURCE:=prince-$(PKG_VERSION).tar.xz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=prince-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/AdvancedNetworkingSystems/poprouting.git
PKG_SOURCE_VERSION:=v0.4alpha2
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/prince
	SECTION :=net
	CATEGORY :=Network
	SUBMENU :=Routing and Redirection
	Mantainer :=Gabriele Gemmi <gabriel@autistici.org>
	TITLE :=PopRouting daemon
	URL :=https://github.com/gabri94/poprouting
	MENU :=0
	DEPENDS := +libjson-c +libpthread
endef

define Package/prince/description
	Prince is an open source implementation of the PopRouting Algorithm.
	It has been developed as a Google Summer of Code Project in collaboration with Freifunk and the University of Trento.

	Prince fetches topology data from a Link State routing daemon(OONF, OLSR, OSPF, etc),
	calculates the betweenness centrality using the topology and then computes the
	the timers' optimal values using these data. At the end of the computation the timers are pushed back to the routing daemon.
	Since v.0.4.0 it supports OONF and OLSRd.
endef

CFLAGS += $(TARGET_CFLAGS) $(TARGET_CPPFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)

$(eval $(call BuildPackage,prince))
