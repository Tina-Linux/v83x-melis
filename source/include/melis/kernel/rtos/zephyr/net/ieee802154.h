/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief IEEE 802.15.4 L2 stack public header
 */

#ifndef __IEEE802154_H__
#define __IEEE802154_H__

#include <net/net_mgmt.h>
#include <crypto/cipher_structs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief IEEE 802.15.4 library
 * @defgroup ieee802154 IEEE 802.15.4 Library
 * @{
 */

#define IEEE802154_MAX_ADDR_LENGTH  8

struct ieee802154_security_ctx
{
    u32_t frame_counter;
    struct cipher_ctx enc;
    struct cipher_ctx dec;
    u8_t key[16];
    u8_t key_len;
    u8_t level  : 3;
    u8_t key_mode   : 2;
    u8_t _unused    : 3;
};

/* This not meant to be used by any code but 802.15.4 L2 stack */
struct ieee802154_context
{
    u16_t pan_id;
    u16_t channel;
    struct k_sem ack_lock;
    u16_t short_addr;
    u8_t ext_addr[IEEE802154_MAX_ADDR_LENGTH];
#ifdef CONFIG_NET_L2_IEEE802154_MGMT
    struct ieee802154_req_params *scan_ctx;
    union
    {
        struct k_sem res_lock;
        struct k_sem req_lock;
    };
    union
    {
        u8_t ext_addr[IEEE802154_MAX_ADDR_LENGTH];
        u16_t short_addr;
    } coord;
    u8_t coord_addr_len;
#endif
#ifdef CONFIG_NET_L2_IEEE802154_SECURITY
    struct ieee802154_security_ctx sec_ctx;
#endif
    s16_t tx_power;
    u8_t sequence;
    u8_t ack_received   : 1;
    u8_t ack_requested  : 1;
    u8_t associated     : 1;
    u8_t _unused        : 5;
};


/* Management part definitions */

#define _NET_IEEE802154_LAYER   NET_MGMT_LAYER_L2
#define _NET_IEEE802154_CODE    0x154
#define _NET_IEEE802154_BASE    (NET_MGMT_IFACE_BIT |           \
                                 NET_MGMT_LAYER(_NET_IEEE802154_LAYER) |\
                                 NET_MGMT_LAYER_CODE(_NET_IEEE802154_CODE))
#define _NET_IEEE802154_EVENT   (_NET_IEEE802154_BASE | NET_MGMT_EVENT_BIT)

enum net_request_ieee802154_cmd
{
    NET_REQUEST_IEEE802154_CMD_SET_ACK = 1,
    NET_REQUEST_IEEE802154_CMD_UNSET_ACK,
    NET_REQUEST_IEEE802154_CMD_PASSIVE_SCAN,
    NET_REQUEST_IEEE802154_CMD_ACTIVE_SCAN,
    NET_REQUEST_IEEE802154_CMD_CANCEL_SCAN,
    NET_REQUEST_IEEE802154_CMD_ASSOCIATE,
    NET_REQUEST_IEEE802154_CMD_DISASSOCIATE,
    NET_REQUEST_IEEE802154_CMD_SET_CHANNEL,
    NET_REQUEST_IEEE802154_CMD_GET_CHANNEL,
    NET_REQUEST_IEEE802154_CMD_SET_PAN_ID,
    NET_REQUEST_IEEE802154_CMD_GET_PAN_ID,
    NET_REQUEST_IEEE802154_CMD_SET_EXT_ADDR,
    NET_REQUEST_IEEE802154_CMD_GET_EXT_ADDR,
    NET_REQUEST_IEEE802154_CMD_SET_SHORT_ADDR,
    NET_REQUEST_IEEE802154_CMD_GET_SHORT_ADDR,
    NET_REQUEST_IEEE802154_CMD_GET_TX_POWER,
    NET_REQUEST_IEEE802154_CMD_SET_TX_POWER,
    NET_REQUEST_IEEE802154_CMD_SET_SECURITY_SETTINGS,
    NET_REQUEST_IEEE802154_CMD_GET_SECURITY_SETTINGS,
};


#define NET_REQUEST_IEEE802154_SET_ACK                  \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_SET_ACK)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_ACK);

#define NET_REQUEST_IEEE802154_UNSET_ACK                \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_UNSET_ACK)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_UNSET_ACK);

#define NET_REQUEST_IEEE802154_PASSIVE_SCAN             \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_PASSIVE_SCAN)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_PASSIVE_SCAN);

#define NET_REQUEST_IEEE802154_ACTIVE_SCAN              \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_ACTIVE_SCAN)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_ACTIVE_SCAN);

#define NET_REQUEST_IEEE802154_CANCEL_SCAN              \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_CANCEL_SCAN)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_CANCEL_SCAN);

#define NET_REQUEST_IEEE802154_ASSOCIATE                \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_ASSOCIATE)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_ASSOCIATE);

#define NET_REQUEST_IEEE802154_DISASSOCIATE             \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_DISASSOCIATE)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_DISASSOCIATE);

#define NET_REQUEST_IEEE802154_SET_CHANNEL              \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_SET_CHANNEL)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_CHANNEL);

#define NET_REQUEST_IEEE802154_GET_CHANNEL              \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_GET_CHANNEL)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_CHANNEL);

#define NET_REQUEST_IEEE802154_SET_PAN_ID               \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_SET_PAN_ID)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_PAN_ID);

#define NET_REQUEST_IEEE802154_GET_PAN_ID               \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_GET_PAN_ID)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_PAN_ID);

#define NET_REQUEST_IEEE802154_SET_EXT_ADDR             \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_SET_EXT_ADDR)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_EXT_ADDR);

#define NET_REQUEST_IEEE802154_GET_EXT_ADDR             \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_GET_EXT_ADDR)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_EXT_ADDR);

#define NET_REQUEST_IEEE802154_SET_SHORT_ADDR               \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_SET_SHORT_ADDR)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_SHORT_ADDR);

#define NET_REQUEST_IEEE802154_GET_SHORT_ADDR               \
    (_NET_IEEE802154_BASE | NET_REQUEST_IEEE802154_CMD_GET_SHORT_ADDR)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_SHORT_ADDR);

#define NET_REQUEST_IEEE802154_GET_TX_POWER             \
    (_NET_IEEE802154_BASE |                     \
     NET_REQUEST_IEEE802154_CMD_GET_TX_POWER)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_TX_POWER);

#define NET_REQUEST_IEEE802154_SET_TX_POWER             \
    (_NET_IEEE802154_BASE |                     \
     NET_REQUEST_IEEE802154_CMD_SET_TX_POWER)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_TX_POWER);

#ifdef CONFIG_NET_L2_IEEE802154_SECURITY

#define NET_REQUEST_IEEE802154_SET_SECURITY_SETTINGS            \
    (_NET_IEEE802154_BASE |                     \
     NET_REQUEST_IEEE802154_CMD_SET_SECURITY_SETTINGS)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_SET_SECURITY_SETTINGS);

#define NET_REQUEST_IEEE802154_GET_SECURITY_SETTINGS            \
    (_NET_IEEE802154_BASE |                     \
     NET_REQUEST_IEEE802154_CMD_GET_SECURITY_SETTINGS)

NET_MGMT_DEFINE_REQUEST_HANDLER(NET_REQUEST_IEEE802154_GET_SECURITY_SETTINGS);

#endif /* CONFIG_NET_L2_IEEE802154_SECURITY */

enum net_event_ieee802154_cmd
{
    NET_EVENT_IEEE802154_CMD_SCAN_RESULT = 1,
};

#define NET_EVENT_IEEE802154_SCAN_RESULT                \
    (_NET_IEEE802154_EVENT | NET_EVENT_IEEE802154_CMD_SCAN_RESULT)


#define IEEE802154_IS_CHAN_SCANNED(_channel_set, _chan) \
    (_channel_set & BIT(_chan - 1))
#define IEEE802154_IS_CHAN_UNSCANNED(_channel_set, _chan)   \
    (!IEEE802154_IS_CHAN_SCANNED(_channel_set, _chan))

/* Useful define to request all channels to be scanned,
 * from 11 to 26 included.
 */
#define IEEE802154_ALL_CHANNELS (0x03FFFC00)

/**
 * @brief Scanning parameters
 *
 * Used to request a scan and get results as well
 */
struct ieee802154_req_params
{
    /** The set of channels to scan, use above macros to manage it */
    u32_t channel_set;

    /** Duration of scan, per-channel, in milliseconds */
    u32_t duration;

    /** Current channel in use as a result */
    u16_t channel;
    /** Current pan_id in use as a result */
    u16_t pan_id;

    /** Result address */
    union
    {
        u8_t addr[IEEE802154_MAX_ADDR_LENGTH];
        u16_t short_addr;
    };

    /** length of address */
    u8_t len;
    /** Link quality information, between 0 and 255 */
    u8_t lqi;
};

/**
 * @brief Security parameters
 *
 * Used to setup the link-layer security settings
 */
struct ieee802154_security_params
{
    u8_t key[16];
    u8_t key_len;
    u8_t key_mode   : 2;
    u8_t level  : 3;
    u8_t _unused    : 3;
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __IEEE802154_H__ */
