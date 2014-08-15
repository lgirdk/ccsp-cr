AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += -D_COSA_SIM_
AM_LDFLAGS = -lccsp_common

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = CcspCrSsp
CcspCrSsp_CPPFLAGS = -I$(top_srcdir)/source/DslhManagementServer $(CPPFLAGS)
CcspCrSsp_SOURCES = CrSsp/ssp_main.c CrSsp/ssp_dbus.c CrSsp/ssp_cmd.c CCSP_CR/ccsp_cr_utility.c CCSP_CR/ccsp_cr_base.c CCSP_CR/ccsp_cr_exportDM.c CCSP_CR/ccsp_cr_operation.c CCSP_CR/ccsp_cr_session.c CCSP_CR/ccsp_cr_profile.c
CcspCrSsp_LDFLAGS =  -lccsp_common
