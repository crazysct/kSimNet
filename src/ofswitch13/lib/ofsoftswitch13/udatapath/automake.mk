#
# Build udatapath as binary
#

bin_PROGRAMS += udatapath/ofdatapath
man_MANS += udatapath/ofdatapath.8

udatapath_ofdatapath_SOURCES = \
	udatapath/action_set.c \
	udatapath/action_set.h \
	udatapath/crc32.c \
	udatapath/crc32.h \
	udatapath/datapath.c \
	udatapath/datapath.h \
	udatapath/dp_actions.c \
	udatapath/dp_actions.h \
	udatapath/dp_buffers.c \
	udatapath/dp_buffers.h \
	udatapath/dp_control.c \
	udatapath/dp_control.h \
	udatapath/dp_exp.c \
	udatapath/dp_exp.h \
	udatapath/dp_ports.c \
	udatapath/dp_ports.h \
	udatapath/flow_table.c \
	udatapath/flow_table.h \
	udatapath/flow_entry.c \
	udatapath/flow_entry.h \
	udatapath/group_table.c \
	udatapath/group_table.h \
	udatapath/group_entry.c \
	udatapath/group_entry.h \
	udatapath/match_std.c \
    udatapath/match_std.h \
	udatapath/meter_entry.c \
	udatapath/meter_entry.h \
	udatapath/meter_table.c \
	udatapath/meter_table.h \	
	udatapath/packet.c \
	udatapath/packet.h \
	udatapath/packet_handle_std.c \
    udatapath/packet_handle_std.h \
	udatapath/pipeline.c \
	udatapath/pipeline.h \
	udatapath/udatapath.c

udatapath_ofdatapath_LDADD = lib/libopenflow.a oflib/liboflib.a oflib-exp/liboflib_exp.a nbee_link/libnbee_link.a $(SSL_LIBS) $(FAULT_LIBS)
udatapath_ofdatapath_CPPFLAGS = $(AM_CPPFLAGS)
nodist_EXTRA_udatapath_ofdatapath_SOURCES = dummy.cxx

EXTRA_DIST += udatapath/ofdatapath.8.in
DISTCLEANFILES += udatapath/ofdatapath.8

if BUILD_HW_LIBS

# Options for each platform
if NF2
udatapath_ofdatapath_CPPFLAGS += -DOF_HW_PLAT -DUSE_NETDEV -g
endif

endif

if BUILD_HW_LIBS
#
# Build udatapath as a library
#

noinst_LIBRARIES += udatapath/libudatapath.a

udatapath_libudatapath_a_SOURCES = \
	udatapath/action_set.c \
	udatapath/action_set.h \
	udatapath/crc32.c \
	udatapath/crc32.h \
	udatapath/datapath.c \
	udatapath/datapath.h \
	udatapath/dp_actions.c \
	udatapath/dp_actions.h \
	udatapath/dp_buffers.c \
	udatapath/dp_buffers.h \
	udatapath/dp_control.c \
	udatapath/dp_control.h \
	udatapath/dp_exp.c \
	udatapath/dp_exp.h \
	udatapath/flow_table.c \
	udatapath/flow_table.h \
	udatapath/flow_entry.c \
	udatapath/flow_entry.h \
	udatapath/group_table.c \
	udatapath/group_table.h \
	udatapath/group_entry.c \
	udatapath/group_entry.h \
	udatapath/match_std.c \
	udatapath/match_std.h \
	udatapath/packet.c \
	udatapath/packet.h \
	udatapath/packet_handle_std.c \
	udatapath/packet_handle_std.h \
	udatapath/pipeline.c \
	udatapath/pipeline.h \
	udatapath/udatapath.c

udatapath_libudatapath_a_CPPFLAGS = $(AM_CPPFLAGS)
udatapath_libudatapath_a_CPPFLAGS += -DOF_HW_PLAT -DUDATAPATH_AS_LIB -g -lnbee_link

endif

if BUILD_NS3_LIBS

#
# Build udatapath as a ns3 library
#

noinst_LIBRARIES += udatapath/libns3ofswitch13.a

udatapath_libns3ofswitch13_a_SOURCES = \
	udatapath/action_set.c \
	udatapath/action_set.h \
	udatapath/crc32.c \
	udatapath/crc32.h \
	udatapath/datapath.c \
	udatapath/datapath.h \
	udatapath/dp_actions.c \
	udatapath/dp_actions.h \
	udatapath/dp_buffers.c \
	udatapath/dp_buffers.h \
	udatapath/dp_control.c \
	udatapath/dp_control.h \
	udatapath/dp_exp.c \
	udatapath/dp_exp.h \
	udatapath/dp_ports.c \
	udatapath/dp_ports.h \
	udatapath/flow_table.c \
	udatapath/flow_table.h \
	udatapath/flow_entry.c \
	udatapath/flow_entry.h \
	udatapath/group_table.c \
	udatapath/group_table.h \
	udatapath/group_entry.c \
	udatapath/group_entry.h \
	udatapath/match_std.c \
	udatapath/match_std.h \
	udatapath/meter_entry.c \
	udatapath/meter_entry.h \
	udatapath/meter_table.c \
	udatapath/meter_table.h \
	udatapath/packet.c \
	udatapath/packet.h \
	udatapath/packet_handle_std.c \
	udatapath/packet_handle_std.h \
	udatapath/pipeline.c \
	udatapath/pipeline.h \
	udatapath/udatapath.c \
	utilities/dpctl.h \
	utilities/dpctl.c \
	lib/timeval.h \
	lib/timeval.c \
	lib/vlog.h \
	lib/vlog.c \
	nbee_link/nbee_link.h \
	nbee_link/nbee_link.cpp

udatapath_libns3ofswitch13_a_LIBADD = \
	oflib/ofl-actions.o \
	oflib/ofl-actions-pack.o \
	oflib/ofl-actions-print.o \
	oflib/ofl-actions-unpack.o \
	oflib/ofl-messages.o \
	oflib/ofl-messages-pack.o \
	oflib/ofl-messages-print.o \
	oflib/ofl-messages-unpack.o \
	oflib/ofl-structs.o \
	oflib/ofl-structs-match.o \
	oflib/ofl-structs-pack.o \
	oflib/ofl-structs-print.o \
	oflib/ofl-structs-unpack.o \
	oflib/oxm-match.o \
	oflib/ofl-print.o

udatapath_libns3ofswitch13_a_LIBADD += \
	oflib-exp/ofl-exp.o \
	oflib-exp/ofl-exp-nicira.o \
	oflib-exp/ofl-exp-openflow.o

udatapath_libns3ofswitch13_a_LIBADD += \
	lib/backtrace.o \
	lib/command-line.o \
	lib/csum.o \
	lib/daemon.o \
	lib/dhcp-client.o \
	lib/dhcp.o \
	lib/dirs.o \
	lib/dynamic-string.o \
	lib/fatal-signal.o \
	lib/fault.o \
	lib/flow.o \
	lib/hash.o \
	lib/hmap.o \
	lib/ipv6_util.o \
	lib/list.o \
	lib/mac-learning.o \
	lib/netdev.o \
	lib/ofpbuf.o \
	lib/ofp.o \
	lib/pcap.o \
	lib/poll-loop.o \
	lib/port-array.o \
	lib/process.o \
	lib/queue.o \
	lib/random.o \
	lib/rconn.o \
	lib/shash.o \
	lib/signals.o \
	lib/socket-util.o \
	lib/stp.o \
	lib/svec.o \
	lib/tag.o \
	lib/util.o \
	lib/vconn.o \
	lib/vconn-stream.o \
	lib/vconn-tcp.o \
	lib/vconn-unix.o \
	lib/vlog-socket.o

if HAVE_NETLINK
udatapath_libns3ofswitch13_a_LIBADD += \
	lib/dpif.o \
	lib/netlink.o \
	lib/vconn-netlink.o
endif

if HAVE_OPENSSL
udatapath_libns3ofswitch13_a_LIBADD += \
	lib/vconn-ssl.o
endif

udatapath_libns3ofswitch13_a_CPPFLAGS = $(AM_CPPFLAGS)
udatapath_libns3ofswitch13_a_CPPFLAGS += -DUDATAPATH_AS_LIB -DDPCTL_AS_LIB -DNS3_OFSWITCH13 -DNETPDLDIR='"$(shell pwd)"' -g -lnbee_link

endif
