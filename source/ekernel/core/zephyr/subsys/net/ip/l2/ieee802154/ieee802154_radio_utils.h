/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __IEEE802154_RADIO_UTILS_H__
#define __IEEE802154_RADIO_UTILS_H__

typedef int (ieee802154_radio_tx_frag_t)(struct net_if *iface,
        struct net_pkt *pkt,
        struct net_buf *frag);

static inline bool prepare_for_ack(struct ieee802154_context *ctx,
                                   struct net_pkt *pkt)
{
    if (ieee802154_ack_required(pkt))
    {
        ctx->ack_received = false;
        k_sem_init(&ctx->ack_lock, 0, UINT_MAX);

        return true;
    }

    return false;
}

static inline int wait_for_ack(struct ieee802154_context *ctx,
                               bool ack_required)
{
    if (!ack_required)
    {
        return 0;
    }

    if (k_sem_take(&ctx->ack_lock, 10) == 0)
    {
        /*
         * We reinit the semaphore in case handle_ack
         * got called multiple times.
         */
        k_sem_init(&ctx->ack_lock, 0, UINT_MAX);
    }

    return ctx->ack_received ? 0 : -EIO;
}

static inline int handle_ack(struct ieee802154_context *ctx,
                             struct net_pkt *pkt)
{
    if (pkt->frags->len == IEEE802154_ACK_PKT_LENGTH)
    {
        ctx->ack_received = true;
        k_sem_give(&ctx->ack_lock);

        return NET_OK;
    }

    return NET_CONTINUE;
}

static inline int tx_packet_fragments(struct net_if *iface,
                                      struct net_pkt *pkt,
                                      ieee802154_radio_tx_frag_t *tx_func)
{
    int ret = 0;
    struct net_buf *frag;

    frag = pkt->frags;
    while (frag)
    {
        ret = tx_func(iface, pkt, frag);
        if (ret)
        {
            break;
        }

        frag = frag->frags;
    }

    if (!ret)
    {
        net_pkt_unref(pkt);
    }

    return ret;
}

#endif /* __IEEE802154_RADIO_UTILS_H__ */
