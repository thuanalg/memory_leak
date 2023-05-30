/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  27/08/2020 09:48:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sean Kim
 *        Company:  Skylab Innogram
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libiptc/libiptc.h>
#include <linux/netfilter/xt_mark.h>
//https://cboard.cprogramming.com/c-programming/145268-calling-libiptc-api-separate-thread-c-program-throws-segmentation-fault.html
//git://git.netfilter.org/iptables
//https://stackoverflow.com/questions/34302445/libiptc-adding-nat-rule-with-mark-based-match
//
//https://stackoverflow.com/questions/34302445/libiptc-adding-nat-rule-with-mark-based-match

int addChainToBuiltin(const char *_pChain, const char *_pTarget);

int main(int argc, char *argv[])
{
  addChainToBuiltin("PREROUTING", "DROP");
  //addChainToBuiltin("PREROUTING", "MARK");
  return EXIT_SUCCESS;
}
//
//static struct ipt_entry_match* get_mark_target() {
//
//    struct ipt_entry_match *match;
//    struct xt_mark_info *m;
//    size_t size;
//    size =   IPT_ALIGN(sizeof(struct ipt_entry_match))
//        + IPT_ALIGN(sizeof(struct xt_mark_info));
//
//    match = calloc(1, size);
//    match->u.match_size = size;
//    strncpy(match->u.user.name, "mark", sizeof(match->u.user.name));
//    m = (struct xt_mark_info*)match->data;
//    m->mark = m->mask = 0xff;
//    return match;
//}

int addChainToBuiltin(const char *_pChain, const char *_pTarget)
{
    /*iptables -A OUTPUT  -j <chain>*/
    int res = 0;
    int i = 0;
    const char *pTable    = "raw";
    struct ipt_tcp *info = 0;
    struct ipt_entry_match *match_limit = 0;
    //struct ipt_tcp *info = (struct ipt_tcp *) match_limit->data;

    struct xtc_handle         *pHandle = 0;
    struct ipt_entry          *pEntry = 0;
    struct xt_standard_target *pTarget = 0;
    struct xt_mark_tginfo2 *infomark = 0;

//    size_t entrySize  = XT_ALIGN(sizeof(struct ipt_entry));
//    size_t targetSize = XT_ALIGN(sizeof(struct xt_standard_target));

    size_t entrySize  = (sizeof(struct ipt_entry));
    size_t targetSize = (sizeof(struct xt_standard_target));
    pHandle = iptc_init(pTable);
    if (! pHandle)
    {
        return errno;
    }
    do
    {
      //pEntry  = calloc(1, entrySize + targetSize);
      pEntry  = calloc(1, 216);
      if(!pEntry)
      {
        break;
      }
      pTarget = (struct xt_entry_target *)((char*)pEntry + entrySize);
      infomark = ((struct xt_entry_target *)pTarget)->data;
      //xt_mark_info  
      if(infomark)
      {
        infomark->mark = 0xff; 
        infomark->mask = 0xff;
      }
      match_limit = (struct ipt_entry_match *) pEntry->elems;
      info = (struct ipt_tcp *) match_limit->data;
      info->dpts[0] = info->dpts[1] = (10000 + i);

      strncpy(pTarget->target.u.user.name, _pTarget, sizeof(pTarget->target.u.user.name));
      pTarget->target.u.target_size      = targetSize;
      pTarget->target.u.user.target_size = targetSize;
      pTarget->target.u.user.revision = 0;

      pEntry->target_offset = entrySize;
      pEntry->next_offset   = entrySize + targetSize;

      pEntry->ip.src.s_addr  = inet_addr("103.102.166.224");
      //pEntry->ip.src.s_addr  = INADDR_ANY;
      pEntry->ip.smsk.s_addr = 0xFFFFFFFF;
      //pEntry->ip.dst.s_addr  = inet_addr("103.102.166.224");
      pEntry->ip.dst.s_addr  = INADDR_ANY;
      pEntry->ip.dmsk.s_addr = 0;
      //:IPPROTO_TCP
      pEntry->ip.proto    = 0; 
      pEntry->ip.flags    = 0;
      pEntry->ip.invflags = 0;
      //pEntry->nfcache     = NFC_UNKNOWN; //NFC_IP_DST_PT
      //pEntry->nfcache     = NFC_IP_DST_PT;
      pEntry->nfcache     = NFC_IP_PROTO;
      //pEntry->nfcache     = NFC_UNKNOWN;
      //match_limit->nfcache     = NFC_IP_DST_PT;
  //
      //fprintf(stdout , "Adding: %s to %s\n", pTarget->target.u.user.name, _pChain);
      //res = iptc_append_entry(_pChain, pEntry, pHandle);
      res = iptc_append_entry("OUTPUT", pEntry, pHandle);
      //res = iptc_insert_entry(_pChain, pEntry, 0,pHandle);
        fprintf(stdout, "iptc_append_entry: %d\n", res);
      if(res != 1)
      {
        fprintf(stdout, "iptc_append_entry: %d\n", res);
        fprintf(stdout, "errror: %s\n", iptc_strerror(errno));
      }
  //
  //    int res = applyRule(Append, _pChain, pEntry, pHandle);
  //    if (res == 0)
  //        res = commitAndFree(pHandle);
  //
      free(pEntry);
      pEntry = 0;
      ++i;
    }
    while(i < 1);
    //fprintf(stdout, "errror: %s\n", iptc_strerror(errno));
    if(pHandle)
    {
      res = iptc_commit(pHandle);
      if(res != 1)
      {
        fprintf(stdout, "iptc_commit: %d\n", res);
        fprintf(stdout, "errror: %s\n", iptc_strerror(errno));
      }
        fprintf(stdout, "iptc_commit: %d\n", res);
      iptc_free(pHandle);
    }
    return res;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 root@sta-mu:/data/zz# iptables-save 
# Generated by iptables-save v1.6.1 on Fri Aug 28 10:22:07 2020
*raw
:PREROUTING ACCEPT [305:19717]
:OUTPUT ACCEPT [18393:7684602]
:sta_firewall - [0:0]
:sta_ignore_network - [0:0]
-A PREROUTING -m mark --mark 0x7 -j ACCEPT
-A PREROUTING -j sta_firewall
-A sta_firewall -d 8.8.8.8/32 -j MARK --set-xmark 0x5/0xffffffff
-A sta_firewall -d 8.8.8.8/32 -j RETURN
-A sta_firewall -j sta_ignore_network
-A sta_ignore_network -d 192.168.0.0/16 -j MARK --set-xmark 0x2/0xffffffff
-A sta_ignore_network -j ACCEPT
COMMIT
# Completed on Fri Aug 28 10:22:07 2020
# Generated by iptables-save v1.6.1 on Fri Aug 28 10:22:07 2020
*nat
:PREROUTING ACCEPT [109056:28012958]
:INPUT ACCEPT [20113:1214530]
:OUTPUT ACCEPT [44228:8589434]
:POSTROUTING ACCEPT [8945:1216612]
:DOCKER - [0:0]
:sta_dnat - [0:0]
:sta_nat_prerouting - [0:0]
:sta_tcp_acceleration - [0:0]
:sta_tcp_egress_acceleration - [0:0]
-A PREROUTING -j sta_nat_prerouting
-A PREROUTING -m addrtype --dst-type LOCAL -j DOCKER
-A OUTPUT -p tcp -j sta_tcp_egress_acceleration
-A OUTPUT ! -d 127.0.0.0/8 -m addrtype --dst-type LOCAL -j DOCKER
-A POSTROUTING -s 172.17.0.0/16 ! -o docker0 -j MASQUERADE
-A POSTROUTING -o enp1s0 -j MASQUERADE
-A POSTROUTING -o ppp0 -j MASQUERADE
-A DOCKER -i docker0 -j RETURN
-A sta_dnat -p tcp -j sta_tcp_acceleration
-A sta_nat_prerouting -m mark --mark 0x5 -j RETURN
-A sta_nat_prerouting -m mark --mark 0x4 -j RETURN
-A sta_nat_prerouting -j sta_dnat
-A sta_tcp_acceleration -m mark --mark 0x2 -j RETURN
-A sta_tcp_egress_acceleration -m mark --mark 0x2 -j RETURN
COMMIT
# Completed on Fri Aug 28 10:22:07 2020
# Generated by iptables-save v1.6.1 on Fri Aug 28 10:22:07 2020
*mangle
:PREROUTING ACCEPT [2691032:1567376933]
:INPUT ACCEPT [844506:193693170]
:FORWARD ACCEPT [1814059:1367448867]
:OUTPUT ACCEPT [772079:126940370]
:POSTROUTING ACCEPT [2583517:1494218877]
:sta_egress_acceleration - [0:0]
:sta_egress_ignore_network - [0:0]
:sta_mangle_prerouting - [0:0]
:sta_udp_acceleration - [0:0]
:sta_udp_egress_acceleration - [0:0]
-A PREROUTING -j sta_mangle_prerouting
-A OUTPUT -j sta_egress_ignore_network
-A sta_egress_acceleration -m mark --mark 0x2 -j RETURN
-A sta_egress_ignore_network -j sta_egress_acceleration
-A sta_mangle_prerouting -m mark --mark 0x5 -j RETURN
-A sta_mangle_prerouting -m mark --mark 0x4 -j RETURN
-A sta_mangle_prerouting -p udp -j sta_udp_acceleration
-A sta_udp_acceleration -m mark --mark 0x7 -j sta_udp_egress_acceleration
-A sta_udp_acceleration -m mark --mark 0x2 -j RETURN
-A sta_udp_egress_acceleration -j ACCEPT
COMMIT
# Completed on Fri Aug 28 10:22:07 2020
# Generated by iptables-save v1.6.1 on Fri Aug 28 10:22:07 2020
*filter
:INPUT DROP [371:145723]
:FORWARD DROP [0:0]
:OUTPUT ACCEPT [18400:7687394]
:DOCKER - [0:0]
:DOCKER-ISOLATION - [0:0]
:sta_filter_forward - [0:0]
:sta_filter_input - [0:0]
-A INPUT -j sta_filter_input
-A FORWARD -j DOCKER-ISOLATION
-A FORWARD -o docker0 -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
-A FORWARD -o docker0 -j DOCKER
-A FORWARD -i docker0 ! -o docker0 -j ACCEPT
-A FORWARD -i docker0 -o docker0 -j ACCEPT
-A FORWARD -j sta_filter_forward
-A DOCKER-ISOLATION -j RETURN
-A sta_filter_forward -m conntrack --ctstate RELATED,ESTABLISHED,SNAT,DNAT -j ACCEPT
-A sta_filter_forward -m mark --mark 0x5 -j DROP
-A sta_filter_forward -m mark --mark 0x4 -j REJECT --reject-with icmp-port-unreachable
-A sta_filter_forward -m mark --mark 0x3 -j ACCEPT
-A sta_filter_forward -i br0 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i br0 -o ppp0 -j ACCEPT
-A sta_filter_forward -i br0 -o br0 -j ACCEPT
-A sta_filter_forward -i br0 -o brvlan101 -j DROP
-A sta_filter_forward -i br0 -o brvlan102 -j DROP
-A sta_filter_forward -i br0 -o brvlan103 -j DROP
-A sta_filter_forward -i br0 -o brvlan104 -j DROP
-A sta_filter_forward -i br0 -o brvlan105 -j DROP
-A sta_filter_forward -i brvlan101 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i brvlan101 -o ppp0 -j ACCEPT
-A sta_filter_forward -i brvlan101 -o brvlan101 -j ACCEPT
-A sta_filter_forward -i brvlan101 -o br0 -j DROP
-A sta_filter_forward -i brvlan101 -o brvlan102 -j DROP
-A sta_filter_forward -i brvlan101 -o brvlan103 -j DROP
-A sta_filter_forward -i brvlan101 -o brvlan104 -j DROP
-A sta_filter_forward -i brvlan101 -o brvlan105 -j DROP
-A sta_filter_forward -i brvlan102 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i brvlan102 -o ppp0 -j ACCEPT
-A sta_filter_forward -i brvlan102 -o brvlan102 -j ACCEPT
-A sta_filter_forward -i brvlan102 -o br0 -j DROP
-A sta_filter_forward -i brvlan102 -o brvlan101 -j DROP
-A sta_filter_forward -i brvlan102 -o brvlan103 -j DROP
-A sta_filter_forward -i brvlan102 -o brvlan104 -j DROP
-A sta_filter_forward -i brvlan102 -o brvlan105 -j DROP
-A sta_filter_forward -i brvlan103 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i brvlan103 -o ppp0 -j ACCEPT
-A sta_filter_forward -i brvlan103 -o brvlan103 -j ACCEPT
-A sta_filter_forward -i brvlan103 -o br0 -j DROP
-A sta_filter_forward -i brvlan103 -o brvlan101 -j DROP
-A sta_filter_forward -i brvlan103 -o brvlan102 -j DROP
-A sta_filter_forward -i brvlan103 -o brvlan104 -j DROP
-A sta_filter_forward -i brvlan103 -o brvlan105 -j DROP
-A sta_filter_forward -i brvlan104 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i brvlan104 -o ppp0 -j ACCEPT
-A sta_filter_forward -i brvlan104 -o brvlan104 -j ACCEPT
-A sta_filter_forward -i brvlan104 -o br0 -j DROP
-A sta_filter_forward -i brvlan104 -o brvlan101 -j DROP
-A sta_filter_forward -i brvlan104 -o brvlan102 -j DROP
-A sta_filter_forward -i brvlan104 -o brvlan103 -j DROP
-A sta_filter_forward -i brvlan104 -o brvlan105 -j DROP
-A sta_filter_forward -i brvlan105 -o enp1s0 -j ACCEPT
-A sta_filter_forward -i brvlan105 -o ppp0 -j ACCEPT
-A sta_filter_forward -i brvlan105 -o brvlan105 -j ACCEPT
-A sta_filter_forward -i brvlan105 -o br0 -j DROP
-A sta_filter_forward -i brvlan105 -o brvlan101 -j DROP
-A sta_filter_forward -i brvlan105 -o brvlan102 -j DROP
-A sta_filter_forward -i brvlan105 -o brvlan103 -j DROP
-A sta_filter_forward -i brvlan105 -o brvlan104 -j DROP
-A sta_filter_input -d 127.0.0.1/32 -j ACCEPT
-A sta_filter_input -i cc0 -j ACCEPT
-A sta_filter_input -p udp -m mark --mark 0x7 -j ACCEPT
-A sta_filter_input -m conntrack --ctstate RELATED,ESTABLISHED,SNAT,DNAT -j ACCEPT
-A sta_filter_input -m mark --mark 0x5 -j DROP
-A sta_filter_input -m mark --mark 0x4 -j REJECT --reject-with icmp-port-unreachable
-A sta_filter_input -m mark --mark 0x3 -j ACCEPT
-A sta_filter_input -p udp -m mark --mark 0x1 -j ACCEPT
-A sta_filter_input -d 192.168.75.1/32 -i br0 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.75.1/32 -i br0 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.75.1/32 -i br0 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.75.1/32 -i br0 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.75.1/32 -i br0 -p tcp -m tcp --dport 3000 -j ACCEPT
-A sta_filter_input -d 192.168.101.1/32 -i brvlan101 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.101.1/32 -i brvlan101 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.101.1/32 -i brvlan101 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.101.1/32 -i brvlan101 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.101.1/32 -i brvlan101 -p tcp -m tcp --dport 3000 -j ACCEPT
-A sta_filter_input -d 192.168.102.1/32 -i brvlan102 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.102.1/32 -i brvlan102 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.102.1/32 -i brvlan102 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.102.1/32 -i brvlan102 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.102.1/32 -i brvlan102 -p tcp -m tcp --dport 3000 -j ACCEPT
-A sta_filter_input -d 192.168.103.1/32 -i brvlan103 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.103.1/32 -i brvlan103 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.103.1/32 -i brvlan103 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.103.1/32 -i brvlan103 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.103.1/32 -i brvlan103 -p tcp -m tcp --dport 3000 -j ACCEPT
-A sta_filter_input -d 192.168.104.1/32 -i brvlan104 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.104.1/32 -i brvlan104 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.104.1/32 -i brvlan104 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.104.1/32 -i brvlan104 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.104.1/32 -i brvlan104 -p tcp -m tcp --dport 3000 -j ACCEPT
-A sta_filter_input -d 192.168.105.1/32 -i brvlan105 -p tcp -m tcp --dport 22 -j ACCEPT
-A sta_filter_input -d 192.168.105.1/32 -i brvlan105 -p tcp -m tcp --dport 9393 -j ACCEPT
-A sta_filter_input -d 192.168.105.1/32 -i brvlan105 -p icmp -m icmp --icmp-type any -j ACCEPT
-A sta_filter_input -d 192.168.105.1/32 -i brvlan105 -p udp -m udp --dport 4000:4009 -j ACCEPT
-A sta_filter_input -d 192.168.105.1/32 -i brvlan105 -p tcp -m tcp --dport 3000 -j ACCEPT
COMMIT
# Completed on Fri Aug 28 10:22:07 2020
root@sta-mu:/data/zz# iptables-save  | grep MARK
-A sta_firewall -d 8.8.8.8/32 -j MARK --set-xmark 0x5/0xffffffff
-A sta_ignore_network -d 192.168.0.0/16 -j MARK --set-xmark 0x2/0xffffffff
root@sta-mu:/data/zz# 

 *
 *
 */
