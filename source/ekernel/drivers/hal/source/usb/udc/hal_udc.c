/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.

 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.

 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PART'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNER'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PART'S TECHNOLOGY.


 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>

//#include <arch/mach/platform.h>
//#include <arch/mach/irqs.h>
#include <interrupt.h>
//#include <io.h>
#include <aw_types.h>
#include <sunxi_hal_common.h>
#include <usb/ch9.h>
#include <usb/sunxi_hal_usb.h>
#include <hal_osal.h>

#include "udc.h"
#include "udc_platform.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x)/sizeof((x)[0]))
#endif

/* UDC base address */
static volatile UDC_REGISTER_T *musb = (UDC_REGISTER_T *)SUNXI_USB_OTG_PBASE;
static volatile USBPHY_REGISTER_T *musb_phy = (USBPHY_REGISTER_T *)(SUNXI_USB_OTG_PBASE + USB_PHY_BASE_OFFSET);

/* UDC private data */
udc_priv_t g_udc;

/* NOTICE: 4K or 8K fifo size */
#define SW_UDC_EPNUMS	4
static udc_fifo_t g_ep_fifo[] = {
	{0,			0,	512,	0}, /* ep0 */
	{1 | USB_DIR_IN,	512,	512,	0}, /* bulk-in */
	{1 | USB_DIR_OUT,	1024,	512,	0}, /* bulk-out */
	{2 | USB_DIR_IN,	1536,	512,	0}, /* bulk-in */
	{2 | USB_DIR_OUT,	2048,	512,	0}, /* bulk-out */
	{3,			2560,	1024,	0}, /* iso */
	{4,			3584,	512,	0}, /* int */
};

/**
 * ep_fifo_in[i] = {n} i: the physic ep index, n: ep_fifo's index for the ep
 *
 * eg: ep_fifo_in[2] = {3} ===> ep2_in is in ep_fifo[3]
 *
 * ep3_iso_name and ep4_int_name cannot be tx or rx simultaneously.
 *
 */
static const uint32_t g_ep_fifo_in[] = {0, 1, 3, 5, 6, 7};
static const uint32_t g_ep_fifo_out[] = {0, 2, 4, 5, 6, 8};

#define SW_UDC_ENDPOINTS 	ARRAY_SIZE(g_ep_fifo)

/* identify ep0 control request */
static uint8_t g_crq_bRequest;
static uint8_t g_crq_wIndex;

static void usbc_wakeup_clear_change_detect(void)
{
	USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_VBUS_CHANGE_DETECT);
	USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_ID_CHANGE_DETECT);
	USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_DPDM_CHANGE_DETECT);
}

static void usbc_enable_dpdm_pullup(bool enable)
{
	if (enable) {
		USB_DRV_SetBits32(&musb_phy->iscr, USB_ISCR_DPDM_PULLUP_EN);
	} else {
		USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_DPDM_PULLUP_EN);
	}

	usbc_wakeup_clear_change_detect();

	log_udc_dbg("dp dm pull up %s\r\n", enable ? "enabled" : "disabled");
}

static void usbc_enable_id_pullup(bool enable)
{
	if (enable) {
		USB_DRV_SetBits32(&musb_phy->iscr, USB_ISCR_ID_PULLUP_EN);
	} else {
		USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_ID_PULLUP_EN);
	}

	usbc_wakeup_clear_change_detect();

	log_udc_dbg("id pull up %s\r\n", enable ? "enabled" : "disabled");
}

static void usbc_force_id(uint32_t id_type)
{
	USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_FORCE_ID_MASK);
	USB_DRV_SetBits32(&musb_phy->iscr, id_type);

	usbc_wakeup_clear_change_detect();

	log_udc_dbg("force id type: 0x%x\r\n", id_type);
}

static void usbc_force_vbus_valid(uint32_t vbus_type)
{
	USB_DRV_ClearBits32(&musb_phy->iscr, USB_ISCR_FORCE_VBUS_MASK);
	USB_DRV_SetBits32(&musb_phy->iscr, vbus_type);

	usbc_wakeup_clear_change_detect();

	log_udc_dbg("force vbus valid type: 0x%x\r\n", vbus_type);
}

static void usbc_select_bus(udc_io_type_t io_type, udc_ep_type_t ep_type, uint32_t ep_index)
{
	uint32_t reg_val;

	reg_val = USB_DRV_Reg8(&musb->vend0);

	if (io_type == UDC_IO_TYPE_DMA) {
		if (ep_type == UDC_EP_TYPE_TX) {
			reg_val |= ((ep_index - 0x01) << 1)
					<< USB_VEND0_DRQ_SEL; /* drq_sel */
			reg_val |= 0x1 << USB_VEND0_BUS_SEL; /* io_dma */
		}
	} else {
		reg_val &= 0x00; /* clear drq_sel, select pio */
	}

	/*
	 * in SUN8IW5 SUN8IW6 and later ic, FIFO_BUS_SEL bit(bit24 of reg0x40
	 * for host/device) is fixed to 1, the hw guarantee that it's ok for
	 * cpu/inner_dma/outer_dma transfer.
	 */
	reg_val |= 0x1 << USB_VEND0_BUS_SEL;

	USB_DRV_WriteReg8(&musb->vend0, reg_val);
}

static void usbc_phy_set_ctl(bool set)
{
	/* NOTICE: 40nm platform is different */

	if (set) {
		USB_DRV_SetBits32(&musb_phy->phyctrl28nm, USB_PHYCTL28NM_VBUSVLDEXT);
		USB_DRV_ClearBits32(&musb_phy->phyctrl28nm, USB_PHYCTL28NM_SIDDQ);
	} else {
		USB_DRV_SetBits32(&musb_phy->phyctrl28nm, USB_PHYCTL28NM_SIDDQ);
	}

	log_udc_dbg("phy %s ctl\r\n", set ? "set" : "clear");
}

static void usbc_phy_otg_sel(bool otg_sel)
{
	if (otg_sel) {
		USB_DRV_SetBits32(&musb_phy->physel, USB_PHYSEL_OTG_SEL);
	} else {
		USB_DRV_ClearBits32(&musb_phy->physel, USB_PHYSEL_OTG_SEL);
	}
}

static uint32_t usbc_get_active_ep(void)
{
	return USB_DRV_Reg8(&musb->index);
}

static void usbc_select_active_ep(uint8_t ep_index)
{
	USB_DRV_WriteReg8(&musb->index, ep_index);
}

static void usbc_udc_disable(void)
{
	log_udc_dbg("udc disable\r\n");

	/* disable all interrupts */
	USB_DRV_WriteReg8(&musb->intrusbe, 0);
	USB_DRV_WriteReg8(&musb->intrtxe, 0);
	USB_DRV_WriteReg8(&musb->intrrxe, 0);

	/* clear the interrupt registers */
	USB_DRV_WriteReg(&musb->intrtx, 0xffff);
	USB_DRV_WriteReg(&musb->intrrx, 0xffff);
	USB_DRV_WriteReg8(&musb->intrusb, 0xff);

	/* clear soft connect */
	USB_DRV_ClearBits8(&musb->power, USB_POWER_SOFTCONN);
}

static void usbc_udc_enable(void)
{
	log_udc_dbg("udc enable\r\n");

	/* config usb transfer type, default: bulk transfer */
	USB_DRV_ClearBits8(&musb->power, USB_POWER_ISOUPDATE);

	/* config usb gadget speed, default: high speed */
	USB_DRV_SetBits8(&musb->power, USB_POWER_HSENAB);

	/* enable usb bus interrupt */
	USB_DRV_SetBits8(&musb->intrusbe, USB_INTRUSB_SUSPEND
						| USB_INTRUSB_RESUME
						| USB_INTRUSB_RESET);

	/* enable ep0 interrupt */
	USB_DRV_SetBits(&musb->intrtxe, USB_INTRE_EPEN << 0);

	/* set soft connect */
	USB_DRV_SetBits8(&musb->power, USB_POWER_SOFTCONN);

	//krhino_spin_lock_init(&g_udc.lock);
}

/* mask the useless irq, save disconect, reset, resume, suspend */
static uint32_t usbc_filtrate_irq(uint32_t usb_irq)
{
	uint32_t irq = usb_irq;

	irq &= ~(USB_INTRUSBE_VBUSERROR
		| USB_INTRUSBE_SESSREQ
		| USB_INTRUSBE_CONN
		| USB_INTRUSBE_SOF);

	USB_DRV_ClearBits8(&musb->intrusb, USB_INTRUSBE_VBUSERROR
						| USB_INTRUSBE_SESSREQ
						| USB_INTRUSBE_CONN
						| USB_INTRUSBE_SOF);

	return irq;
}

static void *usbc_select_fifo(uint32_t ep_index)
{
	uint32_t offset;

	offset = 0x0 + (ep_index << 2);

	return (void *)((char *)&musb->fifo0 + offset);
}

static uint32_t usbc_read_packet(void *fifo, uint32_t cnt, void *buf)
{
	uint32_t len, i32, i8;
	uint8_t *buf8;
	uint32_t *buf32;

	/* adjust data */
	buf32 = buf;
	len = cnt;

	i32 = len >> 2;
	i8 = len & 0x03;

	/* deal with 4 byte part */
	while (i32--) {
		*buf32++ = USB_DRV_Reg32(fifo);
	}

	/* deal with not 4 byte part */
	buf8 = (uint8_t *)buf32;
	while (i8--) {
		*buf8++ = USB_DRV_Reg8(fifo);
	}

	return len;
}

static void usbc_write_packet(void *fifo, uint32_t cnt, void *buf)
{
	uint32_t len, i32, i8;
	uint8_t *buf8;
	uint32_t *buf32;

	/* adjust data */
	buf32 = buf;
	len = cnt;

	i32 = len >> 2;
	i8 = len & 0x03;

	/* deal with 4 byte part */
	while (i32--) {
		USB_DRV_WriteReg32(fifo, *buf32++);
	}

	/* deal with not 4 byte part */
	buf8 = (uint8_t *)buf32;
	while (i8--) {
		USB_DRV_WriteReg8(fifo, *buf8++);
	}
}

static void usbc_ep_config_default(uint8_t is_in)
{
	/* NOTICE: must already select active ep */

	if (is_in) {
		/* clear tx csr */
		USB_DRV_WriteReg(&musb->txcsr, 0x00);
		/* clear tx ep max packet */
		USB_DRV_WriteReg(&musb->txmap, 0x00);
		/* flush fifo */
		USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_CLRDATATOG
						| USB_TXCSR_FLUSHFIFO);
	} else {
		/* clear rx csr */
		USB_DRV_WriteReg(&musb->rxcsr, 0x00);
		/* clear rx ep max packet */
		USB_DRV_WriteReg(&musb->rxmap, 0x00);
		/* flush fifo */
		USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_CLRDATATOG
						| USB_RXCSR_FLUSHFIFO);
	}
}

static void usbc_ep_config(uint32_t ts_type, uint16_t maxpacket,
			uint8_t is_double_fifo, uint8_t is_in)
{
	uint32_t reg_val;
	uint32_t temp;

	/* NOTICE: must already select active ep */

	if (is_in) {
		/* config tx csr */
		reg_val = USB_TXCSR_MODE | USB_TXCSR_CLRDATATOG | USB_TXCSR_FLUSHFIFO;
		USB_DRV_WriteReg(&musb->txcsr, reg_val);
		if (is_double_fifo) /* config twice */
			USB_DRV_WriteReg(&musb->txcsr, reg_val);

		/* config tx ep max packet */
		reg_val = USB_DRV_Reg(&musb->txmap);
		temp = maxpacket & USB_TXMAXP_MAXPAYLOAD_MASK;
		reg_val |= temp;
		USB_DRV_WriteReg(&musb->txmap, reg_val);

		/* config tx ep transfer type */
		switch (ts_type) {
		case USB_ENDPOINT_XFER_ISOC:
			USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_ISO);
			break;
		case USB_ENDPOINT_XFER_INT:
		case USB_ENDPOINT_XFER_BULK:
			USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_ISO);
			break;
		default:
			USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_ISO);
			break;
		}
	} else {
		/* config rx csr */
		reg_val = USB_RXCSR_CLRDATATOG | USB_RXCSR_FLUSHFIFO;
		USB_DRV_WriteReg(&musb->rxcsr, reg_val);
		if (is_double_fifo) /* config twice */
			USB_DRV_WriteReg(&musb->rxcsr, reg_val);

		/* config rx ep max packet */
		reg_val = USB_DRV_Reg(&musb->rxmap);
		temp = maxpacket & USB_RXMAXP_MAXPAYLOAD_MASK;
		reg_val |= temp;
		USB_DRV_WriteReg(&musb->rxmap, reg_val);

		/* config rx ep transfer type */
		switch (ts_type) {
		case USB_ENDPOINT_XFER_ISOC:
			USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_ISO);
			break;
		case USB_ENDPOINT_XFER_INT:
			USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_DISNYET);
			break;
		case USB_ENDPOINT_XFER_BULK:
			USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_ISO);
			break;
		default:
			USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_ISO);
			break;
		}
	}
}

static void usbc_ep_fifo_config(uint32_t fifo_addr, uint32_t fifo_size,
			uint8_t is_double_fifo, uint8_t is_in)
{
	uint32_t temp;
	uint32_t size; /* fifo_size = 2^(size + 3) */
	uint32_t addr; /* fifo_addr = addr * 8 */

	/* NOTICE: must already select active ep */

	/* 512 align */
	size = 0;
	temp = fifo_size + 511;
	temp &= ~511;
	temp >>= 3;
	temp >>= 1;
	while (temp) {
		size++;
		temp >>= 1;
	}

	/* caculate addr */
	addr = fifo_addr >> 3;

	if (is_in) {
		/* config fifo addr */
		USB_DRV_WriteReg(&musb->txfifoadd, addr);
		/* config fifo size */
		USB_DRV_WriteReg8(&musb->txfifosz, (size & USB_FIFOSZ_SIZE_MASK));
		if (is_double_fifo)
			USB_DRV_SetBits8(&musb->txfifosz, USB_FIFOSZ_DPB);
	} else {
		/* config fifo addr */
		USB_DRV_WriteReg(&musb->rxfifoadd, addr);
		/* config fifo size */
		USB_DRV_WriteReg8(&musb->rxfifosz, (size & USB_FIFOSZ_SIZE_MASK));
		if (is_double_fifo)
			USB_DRV_SetBits8(&musb->rxfifosz, USB_FIFOSZ_DPB);
	}
}

static void usbc_ep_intr_enable(uint8_t ep_idx, uint8_t is_in)
{
	/* NOTICE: must already select active ep */

	if (is_in) {
		USB_DRV_SetBits(&musb->intrtxe, USB_INTRE_EPEN << ep_idx);
	} else {
		USB_DRV_SetBits(&musb->intrrxe, USB_INTRE_EPEN << ep_idx);
	}
}

static udc_errno_t crq_get_status(struct usb_ctrlrequest *crq)
{
	uint16_t status = 0;
	uint8_t buf[8];
	uint8_t ep_idx = crq->wIndex & 0x7f;
	uint8_t is_in = crq->wIndex & USB_DIR_IN;
	void *fifo;

	switch (crq->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_INTERFACE:
		buf[0] = 0x00;
		buf[1] = 0x00;
		break;

	case USB_RECIP_DEVICE:
		buf[0] = 0x01;
		buf[1] = 0x00;
		break;

	case USB_RECIP_ENDPOINT:
		if (crq->wLength > 2)
			return UDC_ERRNO_CMD_INVALID;

		if (ep_idx == 0) {
			status = USB_DRV_Reg(&musb->txcsr) & USB_CSR0_SENDSTALL;
		} else {
			if (is_in)
				status = USB_DRV_Reg(&musb->txcsr) & USB_TXCSR_SENDSTALL;
			else
				status = USB_DRV_Reg(&musb->rxcsr) & USB_RXCSR_SENDSTALL;
		}

		status = status ? 1 : 0;
		if (status) {
			buf[0] = 0x01;
			buf[1] = 0x00;
		} else {
			buf[0] = 0x00;
			buf[1] = 0x00;
		}
		break;

	default:
		return UDC_ERRNO_CMD_INVALID;
	}

	/* need udelay(5)? */
	USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY);
	fifo = usbc_select_fifo(0);
	usbc_write_packet(fifo, crq->wLength, buf);
	USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_TXPKTRDY
					| USB_CSR0_DATAEND);

	return UDC_ERRNO_SUCCESS;
}

static int32_t pio_read_fifo(udc_ep_t *ep, bool need_callback)
{
	bool is_last;
	void *fifo;
	void *buf;
	uint32_t idx;
	uint32_t fifo_count;
	uint32_t bufferspace;
	uint32_t avail;
	uint32_t count;

	if (!ep->pdata) {
		log_udc_dbg("ep data buf is NULL\r\n");
		return -3;
	}

	idx = ep->ep_addr & 0x7f;

	/* select fifo*/
	fifo = usbc_select_fifo(idx);

	buf = ep->pdata + ep->data_actual;
	bufferspace = ep->data_len - ep->data_actual;
	if (bufferspace <= 0) {
		log_udc_err("receive buffer full\r\n");
		/* callback to user ?*/
		return -3;
	}

	fifo_count = USB_DRV_Reg(&musb->rxcount);
	if (fifo_count > ep->maxpacket)
		avail = ep->maxpacket;
	else
		avail = fifo_count;

	count = min(bufferspace, avail);

	ep->data_actual += count;

	usbc_read_packet(fifo, count, buf);

	/* checking this with ep0 is not accurate as we already
	* read a control request */
	if (idx != 0 && (fifo_count < ep->maxpacket || ep->data_len == ep->data_actual))
		is_last = 1;
	else
		is_last = (ep->data_len <= ep->data_actual) ? 1 : 0;

	if (idx) { /* ep1~4*/
		USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY
						| USB_RXCSR_OVERRUN
						| USB_RXCSR_DATAERROR);
		if (is_last) {
			/* callback to user */
			if (need_callback && g_udc.callback)
				g_udc.callback(ep->ep_addr, UDC_EVENT_RX_DATA,
						ep->pdata, ep->data_actual);

			ep->pdata = NULL;
			ep->data_actual = 0;
		}
	} else { /* ep0 */
		if (is_last) {
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY
							| USB_CSR0_DATAEND);

			/* callback to user */
			if (need_callback && g_udc.callback)
				g_udc.callback(ep->ep_addr, UDC_EVENT_RX_DATA,
						ep->pdata, ep->data_actual);

			ep->pdata = NULL;
			ep->data_actual = 0;
			g_udc.ep0state = UDC_EP0_IDLE;
		} else {
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY);
		}
	}

	return count;
}

static uint32_t pio_read_fifo_crq(struct usb_ctrlrequest *crq)
{
	uint32_t fifo_count;
	uint32_t i;
	void *pout = crq;
	void *fifo;

	fifo = usbc_select_fifo(0);
	fifo_count = USB_DRV_Reg(&musb->rxcount);

	log_udc_dbg("ep0 fifo count is %d\r\n", fifo_count);

	if (fifo_count != 8) {
		i = 0;

		while (i < 16 && (fifo_count != 8)) {
			fifo_count = USB_DRV_Reg(&musb->rxcount);
			i++;
		}

		if (i >= 16) {
			log_udc_err("ep0 get fifo len failed\r\n");
		}
	}

	return usbc_read_packet(fifo, fifo_count, pout);
}

static int32_t pio_write_fifo(udc_ep_t *ep)
{
	bool is_last;
	void *fifo;
	void *buf;
	uint32_t idx;
	uint32_t count;

	if (!ep->pdata) {
		log_udc_dbg("ep data buf is NULL\r\n");
		return UDC_ERRNO_BUF_NULL;
	}

	idx = ep->ep_addr & 0x7f;

	fifo = usbc_select_fifo(idx);

	count = min(ep->data_len - ep->data_actual, ep->maxpacket);
	buf = ep->pdata + ep->data_actual;
	ep->data_actual += count;

	usbc_write_packet(fifo, count, buf);

	/* check if the last packet
	* last packet is often short (sometimes a zlp) */
	if (count != ep->maxpacket || ep->data_len == ep->data_actual)
		is_last = 1;
	else
		is_last = 0;

	if (idx) { /* ep1~4 */
		USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_UNDERRUN)
		USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
		if (is_last) {
			ep->pdata = NULL;
			ep->data_actual = 0;
		}
	} else { /* ep0 */
		if (is_last) {
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_TXPKTRDY
							| USB_CSR0_DATAEND);

			ep->pdata = NULL;
			ep->data_actual = 0;
			g_udc.ep0state = UDC_EP0_IDLE;
		} else {
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_TXPKTRDY);
		}
	}

	return count;
}

static void udc_set_halt_ex(uint8_t ep_addr, int value);
static void udc_handle_ep0_idle(void)
{
	uint32_t len;
	uint32_t string_idx;
	uint32_t config_idx;
	struct usb_ctrlrequest *crq = &g_udc.crq;
	udc_ep_t *ep0 = &g_udc.ep0;
	int is_in = 0;

	/* start control request */
	if (!(USB_DRV_Reg(&musb->txcsr) & USB_CSR0_RXPKTRDY)) {
		log_udc_dbg("ep0 setup data is not ready\r\n");
		return;
	}

	len = pio_read_fifo_crq(crq);
	if (len != sizeof(*crq)) {
		USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY);
		USB_DRV_SetBits(&musb->txcsr, USB_CSR0_SENDSTALL);

		goto stall;
	}

	log_udc_dbg("ep0: bRequest = 0x%x, bRequestType = 0x%x, wValue = 0x%x, "
		    "wIndex = 0x%x, wLength = 0x%x\r\n",
			crq->bRequest, crq->bRequestType, crq->wValue,
			crq->wIndex, crq->wLength);

	g_udc.req_std = ((crq->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD);

	if (g_udc.req_std) { /* standard request */
		switch (crq->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			if (crq->bRequestType != USB_DIR_IN) {
				goto stall;
			}

			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY);
			switch (crq->wValue >> 8) {
			case USB_DT_DEVICE:
				log_udc_dbg("get device descriptor\r\n");

				/* fill device descriptor */
				ep0->pdata = g_udc.device_desc;
				ep0->data_len = min(crq->wLength, (uint16_t)sizeof(*g_udc.device_desc));
				ep0->data_actual = 0;

				g_udc.ep0state = UDC_EP0_IN_DATA_PHASE;
				break;
			case USB_DT_CONFIG:
				log_udc_dbg("get configuration descriptor\r\n");

				ep0->pdata = g_udc.config_desc;
				ep0->data_len = min(crq->wLength, g_udc.config_desc_len);
				ep0->data_actual = 0;

				g_udc.ep0state = UDC_EP0_IN_DATA_PHASE;
				break;
			case USB_DT_STRING:
				log_udc_dbg("get string descriptor\r\n");

				string_idx = crq->wValue & 0xff;
				if (string_idx > g_udc.string_desc_num) {
					log_udc_err("get string descriptor index overflow\r\n");
					goto stall;
				}
				ep0->pdata = g_udc.string_desc[string_idx];
				ep0->data_len = min(crq->wLength, g_udc.string_desc[string_idx]->bLength);
				ep0->data_actual = 0;

				g_udc.ep0state = UDC_EP0_IN_DATA_PHASE;
				break;
			default:
				/* not support */
				log_udc_err("Get descriptor request not supported\r\n");
				goto stall;
			}
			break;
		case USB_REQ_SET_CONFIGURATION:
			log_udc_dbg("set configuration\r\n");

			/* rx receive over, data end, tx packet ready */
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY
							| USB_CSR0_DATAEND);

			config_idx = crq->wValue & 0xff;
			if (config_idx > g_udc.device_desc->bNumConfigurations) {
				log_udc_err("set configuration index overflow\r\n");
				goto stall;
			}
			/* callback to user to reset configuration, interfaces and endpoints */
			if (g_udc.callback)
				g_udc.callback(0, UDC_EVENT_RX_STANDARD_REQUEST, crq, sizeof(*crq));
			break;
		case USB_REQ_GET_CONFIGURATION:
			log_udc_info("get configuration\r\n");
			/* TODO */
			break;
		case USB_REQ_SET_INTERFACE:
			log_udc_dbg("set interface\r\n");

			/* rx receive over, data end, tx packet ready */
			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY
							| USB_CSR0_DATAEND);

			/* TODO: callback to user to set altsetting */
			break;
		case USB_REQ_GET_INTERFACE:
			log_udc_info("get interface\r\n");
			/* TODO */
			break;
		case USB_REQ_SET_ADDRESS:
			log_udc_dbg("set address\r\n");

			if (crq->bRequestType == USB_RECIP_DEVICE) {
				g_udc.address = crq->wValue & 0x7f;

				/* rx receive over, data end, tx packet ready */
				USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY
								| USB_CSR0_DATAEND);

				g_udc.ep0state = UDC_EP0_END_XFER;
				g_crq_bRequest = USB_REQ_SET_ADDRESS;

				return;
			}
			break;
		case USB_REQ_GET_STATUS:
			log_udc_dbg("get status\r\n");

			if (crq_get_status(crq) != UDC_ERRNO_SUCCESS)
				goto stall;
			break;
		case USB_REQ_CLEAR_FEATURE:
			log_udc_dbg("clear feature\r\n");
			/* --<1>--data direction must be host to device */
			if (crq->bRequestType & (1 << 7)) {
				log_udc_err("USB_REQ_CLEAR_FEATURE:\n");
				log_udc_err("data is not host to device\n");
				break;
			}

			USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY
							| USB_CSR0_DATAEND);

			/* --<3>--data stage */
			if (crq->bRequestType == USB_RECIP_DEVICE) {
				/* wValue 0-1 */
				if (crq->wValue) {
					/*dev->devstatus &= ~(1 << USB_DEVICE_REMOTE_WAKEUP);*/
				} else {
					int k = 0;
					for (k = 0; k < SW_UDC_ENDPOINTS; k++) {
						is_in = crq->wIndex & USB_DIR_IN;
						udc_set_halt_ex(g_ep_fifo[k].ep_addr, 0);
					}
				}
			} else if (crq->bRequestType == USB_RECIP_INTERFACE) {
				/* do nothing */
			} else if (crq->bRequestType == USB_RECIP_ENDPOINT) {
				/* --<3>--release the forbidden of ep */
				/* wValue 0-1 */
				if (crq->wValue) {
					/*dev->devstatus &= ~(1 << USB_DEVICE_REMOTE_WAKEUP);*/
				} else {
					int k = 0;
					is_in = crq->wIndex & USB_DIR_IN;
					for (k = 0; k < SW_UDC_ENDPOINTS; k++) {
						if (g_ep_fifo[k].ep_addr == (crq->wIndex & 0xff))
							udc_set_halt_ex(g_ep_fifo[k].ep_addr, 0);
					}
				}
			} else {
				log_udc_dbg("PANIC : nonsupport set feature request. (%d)\r\n",
					crq->bRequestType);
				goto stall;
			}
			g_udc.ep0state = UDC_EP0_IDLE;
			break;
		case USB_REQ_SET_FEATURE:
			log_udc_info("set feature\r\n");
			/* TODO */
			break;
		default:
			/* not support */
			log_udc_err("Standard request not supported\r\n");
			goto stall;
		}

		/* callback to user to handle after then */
		if (g_udc.callback)
			g_udc.callback(0, UDC_EVENT_RX_STANDARD_REQUEST, crq, sizeof(*crq));
	} else { /* class request */
		/* callback to user to handle specific class requests */
		if (g_udc.callback)
			g_udc.callback(0, UDC_EVENT_RX_CLASS_REQUEST, crq, sizeof(*crq));
	}

	/* finish data stage in just one interrupt */
	switch (g_udc.ep0state) {
	case UDC_EP0_IN_DATA_PHASE:
		if (!(USB_DRV_Reg(&musb->txcsr) & USB_CSR0_TXPKTRDY))
			pio_write_fifo(&g_udc.ep0);
		g_udc.ep0state = UDC_EP0_IDLE;
		break;
	case UDC_EP0_OUT_DATA_PHASE:
		if (USB_DRV_Reg(&musb->txcsr) & USB_CSR0_RXPKTRDY)
			pio_read_fifo(&g_udc.ep0, true);
		g_udc.ep0state = UDC_EP0_IDLE;
		break;
	default:
		break;
	}

	return;

stall:
	log_udc_dbg("ep0 send stall...\r\n");
	USB_DRV_WriteReg(&musb->txcsr, USB_CSR0_SERVICEDRXPKTRDY);
	USB_DRV_SetBits(&musb->txcsr, USB_CSR0_SENDSTALL);
}

static void udc_handle_ep0(void)
{
	log_udc_dbg("Handling ep0, ep0state is %d\r\n", g_udc.ep0state);

	/* select ep0 */
	usbc_select_active_ep(0);

	/* clear stall status */
	if (USB_DRV_Reg(&musb->txcsr) & USB_CSR0_SENTSTALL) {
		log_udc_err("EP0 Stall\r\n");

		/* clear ep0 stall */
		USB_DRV_ClearBits(&musb->txcsr, USB_CSR0_SENDSTALL);
		USB_DRV_ClearBits(&musb->txcsr, USB_CSR0_SENTSTALL);

		g_udc.ep0state = UDC_EP0_IDLE;
		return;
	}

	/* clear setup end */
	if (USB_DRV_Reg(&musb->txcsr) & USB_CSR0_SETUPEND) {
		log_udc_dbg("EP0 Setup End\r\n");

		/* clear ep0 setup end */
		USB_DRV_SetBits(&musb->txcsr, USB_CSR0_SERVICEDSETUPEND);

		g_udc.ep0state = UDC_EP0_IDLE;
	}

	switch (g_udc.ep0state) {
	case UDC_EP0_IDLE:
		udc_handle_ep0_idle();
		break;
	case UDC_EP0_IN_DATA_PHASE: /* GET_DESCRIPTER etc... */
		log_udc_dbg("ep0 in data phase...\r\n");

		if (!(USB_DRV_Reg(&musb->txcsr) & USB_CSR0_TXPKTRDY))
			pio_write_fifo(&g_udc.ep0);
		break;
	case UDC_EP0_OUT_DATA_PHASE: /* SET_DESCRIPTER etc... */
		log_udc_dbg("ep0 out data phase...\r\n");

		if (USB_DRV_Reg(&musb->txcsr) & USB_CSR0_RXPKTRDY)
			pio_read_fifo(&g_udc.ep0, true);
		break;
	case UDC_EP0_END_XFER:
		log_udc_dbg("ep0 end xfer, g_crq_bRequest = 0x%x\r\n", g_crq_bRequest);

		switch (g_crq_bRequest) {
		case USB_REQ_SET_ADDRESS:
			/* clear ep0 setup end */
			USB_DRV_SetBits(&musb->txcsr, USB_CSR0_SERVICEDSETUPEND);

			/* set address */
			USB_DRV_WriteReg8(&musb->faddr, g_udc.address);
			log_udc_dbg("Set address: %d\r\n", g_udc.address);
			break;
		case USB_REQ_SET_FEATURE:
			/* TODO: enter test mode */
			break;
		default:
			break;
		}

		g_crq_bRequest = 0;
		g_udc.ep0state = UDC_EP0_IDLE;
		break;
	case UDC_EP0_STALL:
		log_udc_dbg("ep0 stall...\r\n");
		g_udc.ep0state = UDC_EP0_IDLE;
		break;
	}
}

static void udc_handle_ep(uint32_t fifo_idx)
{
	udc_fifo_t fifo_config;
	uint8_t ep_idx;
	uint8_t old_ep_idx;
	uint8_t is_in;

	fifo_config = g_ep_fifo[fifo_idx];

	ep_idx = fifo_config.ep_addr & 0x7f;
	is_in = fifo_config.ep_addr & USB_DIR_IN;

	/* select ep */
	old_ep_idx = usbc_get_active_ep();
	usbc_select_active_ep(ep_idx);

	log_udc_dbg("Handling %s ep%d...\n", is_in ? "tx" : "rx", ep_idx);

	if (is_in) {
		if (USB_DRV_Reg(&musb->txcsr) & USB_TXCSR_SENTSTALL) {
			log_udc_err("tx ep%d is stall\n", ep_idx);
			USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_SENTSTALL
							| USB_TXCSR_SENDSTALL);
			/* clear data toggle */
			USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_CLRDATATOG);

			goto end;
		}

		if (!(USB_DRV_Reg(&musb->txcsr) & USB_TXCSR_TXPKTRDY)) {
			if (!g_udc.epin[ep_idx - 1].pdata) {
				/* callback to user to fill tx buf */
				if (g_udc.callback)
					g_udc.callback(g_udc.epin[ep_idx - 1].ep_addr,
							UDC_EVENT_TX_COMPLETE, NULL, 0);
			} else {
				pio_write_fifo(&g_udc.epin[ep_idx - 1]);
			}
		}
	} else {
		if (USB_DRV_Reg(&musb->rxcsr) & USB_RXCSR_SENTSTALL) {
			log_udc_err("rx ep%d is stall\n", ep_idx);
			USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_SENTSTALL
							| USB_RXCSR_SENDSTALL);
			/* clear data toggle */
			USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_CLRDATATOG);

			goto end;
		}

		if (USB_DRV_Reg(&musb->rxcsr) & USB_RXCSR_RXPKTRDY)
			pio_read_fifo(&g_udc.epout[ep_idx - 1], true);
	}

end:
	/* restore ep */
	usbc_select_active_ep(old_ep_idx);
}

static irqreturn_t udc_irq_handler(int dummy, void *dev_id)
{
	uint32_t usb_irq, tx_irq, rx_irq;
	uint32_t old_ep_idx;
	uint32_t i;
	uint32_t flags;

	// krhino_spin_lock_irq_save(&g_udc.lock, flags);
	flags = hal_spin_lock_irqsave();

	/* save index */
	old_ep_idx = USB_DRV_Reg8(&musb->index);

	/* read status registers */
	usb_irq = USB_DRV_Reg8(&musb->intrusb);
	tx_irq = USB_DRV_Reg(&musb->intrtx);
	rx_irq = USB_DRV_Reg(&musb->intrrx);

	usb_irq = usbc_filtrate_irq(usb_irq);

	log_udc_dbg("usb_irq: %02x, tx_irq: %02x, rx_irq: %02x\n",
						usb_irq, tx_irq, rx_irq);

	/* RESET */
	if (usb_irq & USB_INTRUSB_RESET) {
		log_udc_dbg("irq: Reset\r\n");

		/* clear irq pending */
		USB_DRV_WriteReg8(&musb->intrusb, USB_INTRUSB_RESET);

		/* select ep0 */
		usbc_select_active_ep(0);

		/* set default address: 0x0 */
		USB_DRV_WriteReg8(&musb->faddr, 0x00);

		g_udc.address = 0;
		g_udc.ep0state = UDC_EP0_IDLE;
		g_udc.speed = UDC_SPEED_UNKNOWN;

		goto end;
	}

	/* RESUME */
	if (usb_irq & USB_INTRUSB_RESUME) {
		log_udc_dbg("irq: Resume\r\n");

		/* clear irq pending */
		USB_DRV_WriteReg8(&musb->intrusb, USB_INTRUSB_RESUME);

		if (g_udc.speed != UDC_SPEED_UNKNOWN) {
			/* TODO: Resume work */
		}
	}

	/* SUSPEND */
	if (usb_irq & USB_INTRUSB_SUSPEND) {
		log_udc_dbg("irq: Suspend\r\n");

		/* clear irq pending */
		USB_DRV_WriteReg8(&musb->intrusb, USB_INTRUSB_SUSPEND);

		if (g_udc.speed != UDC_SPEED_UNKNOWN) {
			/* TODO: Suspend work */
		}

		g_udc.ep0state = UDC_EP0_IDLE;
	}

	/* EP0 control transfer */
	if (tx_irq & USB_INTRTX_EP0) {
		log_udc_dbg("irq: ep0\r\n");

		/* clear irq pending by setting it to a 1 */
		USB_DRV_WriteReg(&musb->intrtx, USB_INTRTX_EP0);

		if (g_udc.speed == UDC_SPEED_UNKNOWN) {
			if (USB_DRV_Reg8(&musb->power) & USB_POWER_HSMODE) {
				log_udc_dbg("usb enter High-speed mode\r\n");
				g_udc.speed = UDC_SPEED_HIGH;
			} else {
				log_udc_dbg("usb enter Full-speed mode\r\n");
				g_udc.speed = UDC_SPEED_FULL;
			}
		}

		udc_handle_ep0();
	}

	/* firstly to get data */

	/* rx endpoint data transfers */
	for (i = 1; i <= SW_UDC_EPNUMS; i++) {
		uint32_t tmp = 1 << i;

		if (rx_irq & tmp) {
			log_udc_dbg("rx irq: ep%d\n", i);

			/* clear irq pending by setting it to a 1 */
			USB_DRV_WriteReg(&musb->intrrx, 0x1 << i);

			udc_handle_ep(g_ep_fifo_out[i]);
		}
	}

	/* tx endpoint data transfers */
	for (i = 1; i <= SW_UDC_EPNUMS; i++) {
		uint32_t tmp = 1 << i;

		if (tx_irq & tmp) {
			log_udc_dbg("tx irq: ep%d\n", i);

			/* clear irq pending by setting it to a 1 */
			USB_DRV_WriteReg(&musb->intrtx, 0x1 << i);

			udc_handle_ep(g_ep_fifo_in[i]);
		}
	}

end:
	/* restore ep */
	usbc_select_active_ep(old_ep_idx);

	// krhino_spin_unlock_irq_restore(&g_udc.lock, flags);
	hal_spin_unlock_irqrestore(flags);

	return IRQ_HANDLED;
}

void udc_device_desc_init(struct usb_device_descriptor *device_desc)
{
	g_udc.device_desc = device_desc;
}

void udc_config_desc_init(void *config_desc, uint32_t len)
{
	g_udc.config_desc = config_desc;
	g_udc.config_desc_len = len;
}

void udc_string_desc_init(const void *string_desc)
{
	g_udc.string_desc[g_udc.string_desc_num]
				= (struct usb_string_descriptor *)string_desc;

	g_udc.string_desc_num++;
}

static void ep_info_init(void)
{
	uint32_t i;

	/* ep0 init */
	g_udc.ep0.ep_addr = 0;
	g_udc.ep0.maxpacket = UDC_MAX_PACKET_SIZE_EP0;

	/* epin init */
	for (i = 0; i < UDC_MAX_NUM_EP_TX; i++) {
		g_udc.epin[i].ep_addr = i + 1;
		g_udc.epin[i].maxpacket = UDC_MAX_PACKET_SIZE_EP_BULK;
	}

	/* epout init */
	for (i = 0; i < UDC_MAX_NUM_EP_RX; i++) {
		g_udc.epout[i].ep_addr = i + 1;
		g_udc.epout[i].maxpacket = UDC_MAX_PACKET_SIZE_EP_BULK;
	}
}

static void udc_set_halt_ex(uint8_t ep_addr, int value)
{
	uint8_t ep_idx;
	uint8_t is_in;
	uint8_t old_ep_idx;

	ep_idx = ep_addr & 0x7f;
	is_in = ep_addr & USB_DIR_IN;
	/* select ep */
	old_ep_idx = usbc_get_active_ep();
	usbc_select_active_ep(ep_idx);

	if (ep_idx == 0) {
			USB_DRV_ClearBits(&musb->txcsr, USB_CSR0_SENDSTALL);
			USB_DRV_ClearBits(&musb->txcsr, USB_CSR0_SENTSTALL);
	} else {
		if (is_in) {
			if (value) {
				USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_SENDSTALL);
			} else {
				USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_SENTSTALL);
				USB_DRV_ClearBits(&musb->txcsr, USB_TXCSR_SENDSTALL);
				USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_CLRDATATOG);
			}
		} else {
			if (value) {
				USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_SENDSTALL);
			} else {
				USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_SENDSTALL);
				USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_SENTSTALL);
				USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_CLRDATATOG);
			}
		}
	}
	usbc_select_active_ep(old_ep_idx);

	return;
}

void udc_ep_enable(uint8_t ep_addr, uint16_t maxpacket, uint32_t ts_type)
{
	uint8_t fifo_idx;
	uint8_t ep_idx;
	uint8_t is_in;
	udc_fifo_t fifo_config;
	uint8_t old_ep_idx;
	uint32_t reg_val;
	uint32_t flags;

	ep_idx = ep_addr & 0x7f;
	is_in = ep_addr & USB_DIR_IN;

	/* maybe use in irq, can't spinlock */
	/*krhino_spin_lock_irq_save(&g_udc.lock, flags);*/

	/* select ep */
	old_ep_idx = usbc_get_active_ep();
	usbc_select_active_ep(ep_idx);

	if (is_in)
		fifo_idx = g_ep_fifo_in[ep_idx];
	else
		fifo_idx = g_ep_fifo_out[ep_idx];

	fifo_config = g_ep_fifo[fifo_idx];

	log_udc_dbg("ep_addr: 0x%x, maxpacket: %d, ts_type: %d\n",
					ep_addr, maxpacket, ts_type);
	log_udc_dbg("fifo_idx: %d, fifo.ep_addr: 0x%x, fifo.addr: %d, fifo.size: %d, fifo.dbf: %d\n",
					fifo_idx, fifo_config.ep_addr, fifo_config.fifo_addr,
					fifo_config.fifo_size, fifo_config.double_fifo);

	/* TODO: check validity */

	usbc_ep_config_default(is_in);

	/* set max packet, type, direction, address;
	 * reset fifo counters, enable irq */
	usbc_ep_config(ts_type, maxpacket, fifo_config.double_fifo, is_in);
	usbc_ep_fifo_config(fifo_config.fifo_addr, fifo_config.fifo_size,
				fifo_config.double_fifo, is_in);

	if (ts_type == USB_ENDPOINT_XFER_ISOC)
		USB_DRV_SetBits8(&musb->power, USB_POWER_ISOUPDATE);

	/* enable ep interrupt */
	usbc_ep_intr_enable(ep_idx, is_in);

	/* restore ep */
	usbc_select_active_ep(old_ep_idx);

	/*krhino_spin_unlock_irq_restore(&g_udc.lock, flags);*/
}

void udc_ep_disable(uint8_t ep_addr)
{
	/* TODO */
}

int32_t udc_ep_read(uint8_t ep_addr, void *buf, uint32_t len)
{
	uint8_t ep_idx;
	uint8_t is_in;
	udc_ep_t *ep;
	uint8_t old_ep_idx;
	int32_t ret = UDC_ERRNO_RX_NOT_READY;
	uint32_t flags;

	ep_idx = ep_addr & 0x7f;
	is_in = ep_addr & USB_DIR_IN;

	// krhino_spin_lock_irq_save(&g_udc.lock, flags);
	flags = hal_spin_lock_irqsave();

	/* select ep */
	old_ep_idx = usbc_get_active_ep();
	usbc_select_active_ep(ep_idx);

	if (ep_idx == 0) {
		ep = &g_udc.ep0;
	} else if (!is_in) {
		ep = &g_udc.epout[ep_idx - 1];
	} else {
		ret = UDC_ERRNO_EP_INVALID;
		goto end;
	}

	ep->pdata = buf;
	ep->data_len = len;

	if (USB_DRV_Reg(&musb->rxcsr) & USB_RXCSR_RXPKTRDY)
		ret = pio_read_fifo(ep, false);

end:
	/* restore ep */
	usbc_select_active_ep(old_ep_idx);

	// krhino_spin_unlock_irq_restore(&g_udc.lock, flags);
	hal_spin_unlock_irqrestore(flags);

	return ret;
}

int32_t udc_ep_write(uint8_t ep_addr, void *buf, uint32_t len)
{
	uint8_t ep_idx;
	uint8_t is_in;
	udc_ep_t *ep;
	uint8_t old_ep_idx;
	int32_t ret = UDC_ERRNO_TX_BUSY;
	uint32_t flags;

	ep_idx = ep_addr & 0x7f;
	is_in = ep_addr & USB_DIR_IN;

	// krhino_spin_lock_irq_save(&g_udc.lock, flags);
	flags = hal_spin_lock_irqsave();

	/* select ep */
	old_ep_idx = usbc_get_active_ep();
	usbc_select_active_ep(ep_idx);

	if (ep_idx == 0) {
		ep = &g_udc.ep0;
	} else if (is_in) {
		ep = &g_udc.epin[ep_idx - 1];
	} else {
		ret = UDC_ERRNO_EP_INVALID;
		goto end;
	}

	ep->pdata = buf;
	ep->data_len = len;

	while((USB_DRV_Reg(&musb->txcsr) & USB_TXCSR_TXPKTRDY));
	ret = pio_write_fifo(ep);

end:
	/* restore ep */
	usbc_select_active_ep(old_ep_idx);

	// krhino_spin_unlock_irq_restore(&g_udc.lock, flags);
	hal_spin_unlock_irqrestore(flags);

	return ret;
}

void udc_ep_set_buf(uint8_t ep_addr, void *buf, uint32_t len)
{
	uint8_t ep_idx;
	uint8_t is_in;
	uint32_t flags;

	ep_idx = ep_addr & 0x7f;
	is_in = ep_addr & USB_DIR_IN;

	// krhino_spin_lock_irq_save(&g_udc.lock, flags);
	flags = hal_spin_lock_irqsave();

	if (is_in) {
		g_udc.epin[ep_idx - 1].pdata = buf;
		g_udc.epin[ep_idx - 1].data_len = len;
	} else {
		g_udc.epout[ep_idx - 1].pdata = buf;
		g_udc.epout[ep_idx - 1].data_len = len;
	}

	// krhino_spin_unlock_irq_restore(&g_udc.lock, flags);
	hal_spin_unlock_irqrestore(flags);
}

void udc_register_callback(udc_callback_t user_callback)
{
	g_udc.callback = user_callback;
}

int32_t udc_enter_test_mode(uint32_t test_mode)
{
	/* TODO */
	return 0;
}

int32_t udc_deinit(void)
{
	/* TODO */
	return 0;
}

int32_t udc_init(void)
{
	/* NOTICE: clock and regulator must be ready first */

	ep_info_init();

	/* udc bsp init */
	usbc_enable_dpdm_pullup(true);
	usbc_enable_id_pullup(true);
	usbc_force_id(USB_ISCR_FORCE_ID_HIGH);
	usbc_force_vbus_valid(USB_ISCR_FORCE_VBUS_HIGH);
	usbc_select_bus(UDC_IO_TYPE_PIO, UDC_EP_TYPE_EP0, 0);
	usbc_phy_set_ctl(true);
	usbc_phy_otg_sel(true);

	/* disable udc before request irq */
	usbc_udc_disable();

	/* request irq */
	if (request_irq(SUNXI_IRQ_USBOTG, udc_irq_handler, 0, "usb_udc", NULL) < 0) {
		log_udc_err("request irq error\n");
		return -1;
	}

	enable_irq(SUNXI_IRQ_USBOTG);

	/* udc enable */
	usbc_udc_enable();

	return 0;
}
