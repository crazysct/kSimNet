/* Copyright (c) 2011, TrafficLab, Ericsson Research, Hungary
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Ericsson Research nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef DPCTL_H
#define DPCTL_H 1

#include "openflow/openflow.h"

#include "vconn.h"
#include "oflib/ofl-messages.h"

struct names8 {
    uint8_t    code;
    const char *name;
};

struct names16 {
    uint16_t   code;
    const char *name;
};

struct names32 {
    uint32_t   code;
    const char *name;
};

#define FLOW_MOD_COMMAND       "cmd"
#define FLOW_MOD_COOKIE        "cookie"
#define FLOW_MOD_COOKIE_MASK   "cookie_mask"
#define FLOW_MOD_TABLE_ID      "table"
#define FLOW_MOD_IDLE          "idle"
#define FLOW_MOD_HARD          "hard"
#define FLOW_MOD_PRIO          "prio"
#define FLOW_MOD_BUFFER        "buffer"
#define FLOW_MOD_OUT_PORT      "out_port"
#define FLOW_MOD_OUT_GROUP     "out_group"
#define FLOW_MOD_FLAGS         "flags"
#define FLOW_MOD_MATCH         "match"


#define MATCH_IN_PORT        "in_port"
#define MATCH_DL_SRC         "eth_src"
#define MATCH_DL_SRC_MASK    "eth_src_mask"
#define MATCH_DL_DST         "eth_dst"
#define MATCH_DL_DST_MASK    "eth_dst_mask"
#define MATCH_DL_VLAN        "vlan_vid"
#define MATCH_IP_DSCP        "ip_dscp"
#define MATCH_IP_ECN         "ip_ecn"
#define MATCH_DL_VLAN_PCP    "vlan_pcp"
#define MATCH_DL_TYPE        "eth_type"
#define MATCH_NW_PROTO       "ip_proto"
#define MATCH_NW_SRC         "ip_src"
#define MATCH_NW_SRC_MASK    "nw_src_mask"
#define MATCH_NW_DST         "ip_dst"
#define MATCH_NW_DST_MASK    "ipv4_dst_mask"
#define MATCH_TP_SRC         "tcp_src"
#define MATCH_TP_DST         "tcp_dst"
#define MATCH_UDP_SRC        "udp_src"
#define MATCH_UDP_DST        "udp_dst"
#define MATCH_SCTP_SRC       "sctp_src"
#define MATCH_SCTP_DST       "sctp_dst"
#define MATCH_ICMPV4_CODE    "icmp_code"
#define MATCH_ICMPV4_TYPE    "icmp_type"
#define MATCH_ARP_OP         "arp_op"
#define MATCH_ARP_SHA        "arp_sha"
#define MATCH_ARP_THA        "arp_tha"
#define MATCH_ARP_SPA        "arp_spa"
#define MATCH_ARP_TPA        "arp_tpa"
#define MATCH_NW_SRC_IPV6    "ipv6_src"
#define MATCH_NW_DST_IPV6    "ipv6_dst"
#define MATCH_ICMPV6_CODE    "icmpv6_code"
#define MATCH_ICMPV6_TYPE    "icmpv6_type"
#define MATCH_IPV6_FLABEL    "ipv6_flabel"
#define MATCH_IPV6_ND_TARGET "ipv6_nd_target"
#define MATCH_IPV6_ND_SLL    "ipv6_nd_sll"
#define MATCH_IPV6_ND_TLL    "ipv6_nd_tll"
#define MATCH_MPLS_LABEL     "mpls_label"
#define MATCH_MPLS_TC        "mpls_tc"
#define MATCH_MPLS_BOS       "mpls_bos"
#define MATCH_METADATA       "meta"
#define MATCH_METADATA_MASK  "meta_mask"
#define MATCH_PBB_ISID       "pbb_isid"
#define MATCH_TUNNEL_ID      "tunn_id"    
#define MATCH_EXT_HDR        "ext_hdr"

#define GROUP_MOD_COMMAND "cmd"
#define GROUP_MOD_TYPE    "type"
#define GROUP_MOD_GROUP   "group"

#define BUCKET_WEIGHT       "weight"
#define BUCKET_WATCH_PORT   "port"
#define BUCKET_WATCH_GROUP  "group"

#define METER_MOD_COMMAND "cmd"
#define METER_MOD_FLAGS   "flags"
#define METER_MOD_METER   "meter"

#define BAND_RATE "rate"
#define BAND_BURST_SIZE "burst"
#define BAND_PREC_LEVEL "prec_level"

#define CONFIG_FLAGS "flags"
#define CONFIG_MISS  "miss"


#define PORT_MOD_PORT      "port"
#define PORT_MOD_HW_ADDR   "addr"
#define PORT_MOD_HW_CONFIG "conf"
#define PORT_MOD_MASK      "mask"
#define PORT_MOD_ADVERTISE "adv"


#define TABLE_MOD_TABLE  "table"
#define TABLE_MOD_CONFIG "conf"

#define KEY_VAL    "="
#define KEY_VAL2   ":"
#define KEY_SEP    ","
#define MASK_SEP   "/"

#define ADD   "+"
#define WILDCARD_SUB   '-'



#define NUM_ELEMS( x )   (sizeof(x) / sizeof(x[0]))

#ifdef NS3_OFSWITCH13
/* Enable ns3 openflow controller to execute dpctl commands. */
int dpctl_exec_ns3_command (void *ctrl, int argc, char *argv[]);

/* These signature is necessary here only when compiling the ns3 library. */
void dpctl_send_and_print(struct vconn *vconn, struct ofl_msg_header *msg);
void dpctl_transact_and_print(struct vconn *vconn, struct ofl_msg_header *req,
                              struct ofl_msg_header **repl);
#endif

#endif /* DPCTL_H */
