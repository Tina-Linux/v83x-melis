/** @file
 @brief IPv4 related functions

 This is not to be included by the application.
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __IPV4_H
#define __IPV4_H

#include <zephyr/types.h>

#include <net/net_ip.h>
#include <net/net_pkt.h>
#include <net/net_if.h>
#include <net/net_context.h>

#include "ipv4.h"

/**
 * @brief Create IPv4 packet in provided net_pkt.
 *
 * @param pkt Network packet
 * @param src Source IPv4 address
 * @param dst Destination IPv4 address
 * @param iface Network interface
 * @param next_header Protocol type of the next header after IPv4 header.
 *
 * @return Return network packet that contains the IPv4 packet.
 */
struct net_pkt *net_ipv4_create_raw(struct net_pkt *pkt,
                                    const struct in_addr *src,
                                    const struct in_addr *dst,
                                    struct net_if *iface,
                                    u8_t next_header);

/**
 * @brief Create IPv4 packet in provided net_pkt.
 *
 * @param context Network context for a connection
 * @param pkt Network packet
 * @param src_addr Source address, or NULL to choose a default
 * @param dst_addr Destination IPv4 address
 *
 * @return Return network packet that contains the IPv6 packet.
 */
struct net_pkt *net_ipv4_create(struct net_context *context,
                                struct net_pkt *pkt,
                                const struct in_addr *src_addr,
                                const struct in_addr *dst_addr);

/**
 * @brief Finalize IPv4 packet. It should be called right before
 * sending the packet and after all the data has been added into
 * the packet. This function will set the length of the
 * packet and calculate the higher protocol checksum if needed.
 *
 * @param pkt Network packet
 * @param next_header Protocol type of the next header after IPv4 header.
 *
 * @return Return 0 on Success, < 0 on Failure.
 */
int net_ipv4_finalize_raw(struct net_pkt *pkt, u8_t next_header);

/**
 * @brief Finalize IPv4 packet. It should be called right before
 * sending the packet and after all the data has been added into
 * the packet. This function will set the length of the
 * packet and calculate the higher protocol checksum if needed.
 *
 * @param context Network context for a connection
 * @param pkt Network packet
 *
 * @return Return 0 on Success, < 0 on Failure.
 */
int net_ipv4_finalize(struct net_context *context, struct net_pkt *pkt);

#endif /* __IPV4_H */
