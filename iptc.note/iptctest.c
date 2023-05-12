/* 
 * Example from http://wiki.tldp.org/iptc library HOWTO
 * 
 * Create iptables' equivalent rule: 
 * iptables -A INPUT -s 156.145.1.3 -d 168.220.1.9 -i eth0 -p tcp --sport 0:59136 --dport 0:51201 -m limit -limit 2000/s --limit-burst 10 -m physdev-in eth0 -j ACCEPT 
 *
 * To compile this code, use the following line:
 * gcc -g -o test test.c -liptc -liptables -ldl
 */

#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libiptc/libiptc.h>
#include <linux/netfilter/xt_limit.h>
#include <linux/netfilter/xt_physdev.h>
#include <iptables.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPT_ALIGN XT_ALIGN

const char *program_name = "p4";
const char *program_version = "NETFILTER_VERSION";

typedef struct iptc_handle * iptc_handle_t;
/*
226   const struct xt_mark_target_info_v1 *markinfo =                                                                                                     227     (const struct xt_mark_target_info_v1 *)target->data;
228   
229   switch (markinfo->mode) {
230   case XT_MARK_SET:
231     printf(" --set-mark");
232     break;
233   case XT_MARK_AND:
234     printf(" --and-mark");
235     break;
236   case XT_MARK_OR:
237     printf(" --or-mark");
238     break;
239   }
240   print_mark(markinfo->mark);
241 } 
*/

typedef struct  {
  unsigned long mark;
  uint8_t mode;
} xt_mark_target_info_v1;


int main(void)
{
	iptc_handle_t h;
	const ipt_chainlabel chain = "INPUT";
	const char * tablename = "mangle";
/*	
	const ipt_chainlabel chain = "INPUT";
	const char * tablename = "filter";
**/
	struct ipt_entry * e;
	struct ipt_entry_match * match_proto, * match_limit, * match_physdev;
	struct ipt_entry_target * target;

  xt_mark_target_info_v1 *markinfo = 0;
	struct ipt_tcp * tcpinfo;
	struct xt_rateinfo * rateinfo;
	struct xt_physdev_info * physdevinfo;
	unsigned int size_ipt_entry, size_ipt_entry_match, size_ipt_entry_target, size_ipt_tcp, size_rateinfo, size_physdevinfo, total_length;
	
	size_ipt_entry = IPT_ALIGN(sizeof(struct ipt_entry));
	size_ipt_entry_match = IPT_ALIGN(sizeof(struct ipt_entry_match));
	//size_ipt_entry_target = IPT_ALIGN(sizeof(struct ipt_entry_target) + sizeof(IPTC_LABEL_ACCEPT));
	//fprintf(stdout, "%d, %d, %d\n", size_ipt_entry_target, sizeof(struct ipt_entry_target), sizeof(IPTC_LABEL_ACCEPT));
	//size_ipt_entry_target = 1 + sizeof(struct ipt_entry_target) + sizeof(IPTC_LABEL_ACCEPT);
	//size_ipt_entry_target = IPT_ALIGN(sizeof(struct ipt_entry_target));
	size_ipt_entry_target = sizeof(struct ipt_entry_target);
	size_ipt_entry_target += sizeof(xt_mark_target_info_v1) + 1;
	size_ipt_tcp = IPT_ALIGN(sizeof(struct ipt_tcp));
	size_rateinfo = IPT_ALIGN(sizeof(struct xt_rateinfo));
	size_physdevinfo = IPT_ALIGN(sizeof(struct xt_physdev_info));
	total_length =  size_ipt_entry + size_ipt_entry_match * 3 + size_ipt_entry_target + size_ipt_tcp + size_rateinfo + size_physdevinfo;
	
	//memory allocation for all structs that represent the netfilter rule we want to insert
	e = calloc(1, total_length);
	if(e == NULL)
	{
		printf("malloc failure");
		exit(1);
	}
	
	//offsets to the other bits:
	//target struct begining
	e->target_offset = size_ipt_entry + size_ipt_entry_match * 3 + size_ipt_tcp + size_rateinfo + size_physdevinfo;
	//next "e" struct, end of the current one
	e->next_offset = total_length;
	
	//set up packet matching rules: ?-s 156.145.1.3 -d 168.220.1.9 -i eth0? part
	//of our desirable rule
	e->ip.src.s_addr = inet_addr("156.145.1.3");
	e->ip.smsk.s_addr= inet_addr("255.255.255.255");
	e->ip.dst.s_addr = inet_addr("168.220.1.9");
	e->ip.dmsk.s_addr= inet_addr("255.255.255.255");
	e->ip.proto = IPPROTO_TCP;
	e->nfcache = 0;
	strcpy(e->ip.iniface, "eth0");
	
	//match structs setting:
	//set match rule for the protocol to use
	//?-p tcp? part of our desirable rule
	match_proto = (struct ipt_entry_match *) e->elems;
	match_proto->u.match_size = size_ipt_entry_match + size_ipt_tcp;
	strcpy(match_proto->u.user.name, "tcp");//set name of the module, we will use in this match
	
	//set match rule for the packet number per time limitation - against DoS attacks
	//?-m limit? part of our desirable rule
	match_limit = (struct ipt_entry_match *) (e->elems + match_proto->u.match_size);
	match_limit->u.match_size = size_ipt_entry_match + size_rateinfo;
	strcpy(match_limit->u.user.name, "limit");//set name of the module, we will use in this match
	
	//set match rule for specific Ethernet card (interface)
	//?-m physdev? part of our desirable rule
	match_physdev = (struct ipt_entry_match *) (e->elems + match_proto->u.match_size + match_limit->u.match_size);
	match_physdev->u.match_size = size_ipt_entry_match + size_physdevinfo;
	strcpy(match_physdev->u.user.name, "physdev");//set name of the module, we will use in this match
	
	//tcp module - match extension
	//?--sport 0:59136 --dport 0:51201? part of our desirable rule
	tcpinfo = (struct ipt_tcp *)match_proto->data;
	tcpinfo->spts[0] = ntohs(0);
	tcpinfo->spts[1] = ntohs(0xE7);
	tcpinfo->dpts[0] = ntohs(0);
	tcpinfo->dpts[1] = ntohs(0x1C8);
	
	
	//limit module - match extension
	//?-limit 2000/s --limit-burst 10? part of our desirable rule
	rateinfo = (struct xt_rateinfo *)match_limit->data;
	rateinfo->avg = 5;
	rateinfo->burst = 10;
	
	//physdev module - match extension
	//?-in eth0? part of our desirable rule
	physdevinfo = (struct xt_physdev_info *)match_physdev->data;
	strcpy(physdevinfo->physindev, "eth0");
	memset(physdevinfo->in_mask, 0xFF, IFNAMSIZ);
	physdevinfo->bitmask = 1;
	
	//target struct
	//?-j ACCEPT? part of our desirable rule
	target = (struct ipt_entry_target *)(e->elems + size_ipt_entry_match * 3 + size_ipt_tcp + size_rateinfo + size_physdevinfo);
	target->u.target_size = size_ipt_entry_target;
	//strcpy(target->u.user.name, "ACCEPT");
	//strcpy(target->u.user.name, "DROP");
	//strcpy(target->u.user.name, "MARK");
	markinfo = target->data;
	markinfo->mark = 0x0;
	//markinfo->
	
	//All the functions, mentioned below could be found in "Querying libiptc HOWTO" manual
	h = iptc_init(tablename);
	if ( !h )
	{
		printf("Error initializing: %s\n", iptc_strerror(errno));
		exit(errno);
	}
	
	//analogous to ?iptables -A INPUT? part of our desirable rule + the rule itself         
	//inside of the e struct
	int x = iptc_append_entry(chain, e, h);
	if (!x)
	{
		printf("Error append_entry: %s\n", iptc_strerror(errno));
		exit(errno);
	}
	printf("%s", target->data);
	int y = iptc_commit(h);
	if (!y)
	{
		printf("Error commit: %s\n", iptc_strerror(errno));
		exit(errno);
	}
	
	exit(0);
	
}
