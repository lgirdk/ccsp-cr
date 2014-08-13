AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -D_COSA_SIM_
AM_LDFLAGS = -L$(top_srcdir)/../CcspCommonLibrary/lib
AM_LDFLAGS += -lccsp_common

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = CcspCrSsp
CcspCrSsp_CPPFLAGS = -I/var/tmp/pc-rdkb/include/dbus-1.0 -I/var/tmp/pc-rdkb/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/include -I$(top_srcdir)/../CcspCommonLibrary/source/debug_api/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/slap/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/MessageBusHelper/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/http/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/ansc/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/tls/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/web/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/stun/include -I$(top_srcdir)/source/DslhManagementServer -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/system/include $(CPPFLAGS)
CcspCrSsp_SOURCES = CrSsp/ssp_main.c CrSsp/ssp_dbus.c CrSsp/ssp_cmd.c CCSP_CR/ccsp_cr_utility.c CCSP_CR/ccsp_cr_base.c CCSP_CR/ccsp_cr_exportDM.c CCSP_CR/ccsp_cr_operation.c CCSP_CR/ccsp_cr_session.c CCSP_CR/ccsp_cr_profile.c
CcspCrSsp_LDFLAGS =  -L$(top_srcdir)/../CcspCommonLibrary/lib -lccsp_common
