/*
 * DVB USB library - provides a generic interface for a DVB USB device driver.
 *
 * dvb-usb-stub.c
 *
 * Copyright (C) 2004-6 Patrick Boettcher (patrick.boettcher@desy.de)
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the Free
 *      Software Foundation, version 2.
 *
 * see Documentation/dvb/README.dvb-usb for more information
 */

#include <linux/spinlock.h>
#include <linux/dmapool.h>
#include "dvb-usb.h"
#include "hcd.h"

int dvb_usb_device_power_ctrl(struct dvb_usb_device *d, int onoff);

//-- usb/core/buffer.c

/* FIXME tune these based on pool statistics ... */
static size_t pool_max[HCD_BUFFER_POOLS] = {
	32, 128, 512, 2048,
};

void *hcd_buffer_alloc(
	struct usb_bus *bus,
	size_t         size,
	gfp_t          mem_flags,
	dma_addr_t     *dma
)
{
	struct usb_hcd *hcd = bus_to_hcd(bus);
	int            i;

	/* some USB hosts just use PIO */
	if (!bus->controller->dma_mask
	&&  !(hcd->driver->flags & HCD_LOCAL_MEM))
	{
		*dma = ~(dma_addr_t) 0;
		return kmalloc(size, mem_flags);
	}

	for (i = 0; i < HCD_BUFFER_POOLS; i++)
	{
		if (size <= pool_max[i])
		{
			return dma_pool_alloc(hcd->pool[i], mem_flags, dma);
		}
	}
	return dma_alloc_coherent(hcd->self.controller, size, dma, mem_flags);
}

void hcd_buffer_free(
	struct usb_bus *bus,
	size_t         size,
	void           *addr,
	dma_addr_t     dma
)
{
	struct usb_hcd *hcd = bus_to_hcd(bus);
	int            i;

	if (!addr)
	{
		return;
	}
	if (!bus->controller->dma_mask
	&&  !(hcd->driver->flags & HCD_LOCAL_MEM))
	{
		kfree(addr);
		return;
	}

	for (i = 0; i < HCD_BUFFER_POOLS; i++)
	{
		if (size <= pool_max[i])
		{
			dma_pool_free(hcd->pool[i], addr, dma);
			return;
		}
	}
	dma_free_coherent(hcd->self.controller, size, addr, dma);
}

//-- usb.c

void *usb_alloc_coherent(struct usb_device *dev, size_t size, gfp_t mem_flags, dma_addr_t *dma)
{
	if (!dev || !dev->bus)
	{
		return NULL;
	}
	return hcd_buffer_alloc(dev->bus, size, mem_flags, dma);
}

void usb_free_coherent(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
	if (!dev || !dev->bus)
	{
		return;
	}
	if (!addr)
	{
		return;
	}
	hcd_buffer_free(dev->bus, size, addr, dma);
}

//-- dvb-usb-demux.c

void dvb_dmx_swfilter_raw(struct dvb_demux *demux, const u8 *buf, size_t count)
{
	spin_lock(&demux->lock);

	demux->feed->cb.ts(buf, count, NULL, 0, &demux->feed->feed.ts, DMX_OK);

	spin_unlock(&demux->lock);
}

//-- dvb-usb-urb.c

/* URB stuff for streaming */
static void usb_urb_complete(struct urb *urb)
{
	struct usb_data_stream *stream = urb->context;
	int ptype = usb_pipetype(urb->pipe);
	int i;
	u8 *b;

//	deb_uxfer("'%s' urb completed. status: %d, length: %d/%d, pack_num: %d, errors: %d\n",
//		ptype == PIPE_ISOCHRONOUS ? "isoc" : "bulk",
//		urb->status,urb->actual_length,urb->transfer_buffer_length,
//		urb->number_of_packets,urb->error_count);

	switch (urb->status)
	{
		case 0:         /* success */
		case -ETIMEDOUT:    /* NAK */
		{
			break;
		}
		case -ECONNRESET:   /* kill */
		case -ENOENT:
		case -ESHUTDOWN:
		{
			return;
		}
		default:        /* error */
		{
			info("URB completion error %d.\n", urb->status);
			break;
		}
	}

	b = (u8 *) urb->transfer_buffer;
	switch (ptype)
	{
		case PIPE_ISOCHRONOUS:
		{
			for (i = 0; i < urb->number_of_packets; i++)
			{
				if (urb->iso_frame_desc[i].status != 0)
				{
					info("IDO frame descriptor has an error: %d\n",urb->iso_frame_desc[i].status);
				}
				else if (urb->iso_frame_desc[i].actual_length > 0)
				{
					stream->complete(stream, b + urb->iso_frame_desc[i].offset, urb->iso_frame_desc[i].actual_length);
				}
				urb->iso_frame_desc[i].status = 0;
				urb->iso_frame_desc[i].actual_length = 0;
			}
//			debug_dump(b,20,deb_uxfer);
			break;
		}
		case PIPE_BULK:
		{
			if (urb->actual_length > 0)
			{
				stream->complete(stream, b, urb->actual_length);
			}
			break;
		}
		default:
		{
			err("Unknown endpoint type in completion handler.");
			return;
		}
	}
	usb_submit_urb(urb,GFP_ATOMIC);
}

int usb_urb_kill(struct usb_data_stream *stream)
{
	int i;
	for (i = 0; i < stream->urbs_submitted; i++)
	{
		info("Killing URB no. %d.\n",i);

		/* stop the URB */
		usb_kill_urb(stream->urb_list[i]);
	}
	stream->urbs_submitted = 0;
	return 0;
}

int usb_urb_submit(struct usb_data_stream *stream)
{
	int i,ret;
	for (i = 0; i < stream->urbs_initialized; i++)
	{
//		deb_ts("submitting URB no. %d\n",i);
		if ((ret = usb_submit_urb(stream->urb_list[i],GFP_ATOMIC)))
		{
			err("Could not submit URB no. %d - get them all back",i);
			usb_urb_kill(stream);
			return ret;
		}
		stream->urbs_submitted++;
	}
	return 0;
}

static int usb_free_stream_buffers(struct usb_data_stream *stream)
{
	if (stream->state & USB_STATE_URB_BUF)
	{
		while (stream->buf_num)
		{
			stream->buf_num--;
//			deb_mem("freeing buffer %d\n",stream->buf_num);
			usb_free_coherent(stream->udev, stream->buf_size, stream->buf_list[stream->buf_num], stream->dma_addr[stream->buf_num]);
		}
	}
	stream->state &= ~USB_STATE_URB_BUF;

	return 0;
}

static int usb_allocate_stream_buffers(struct usb_data_stream *stream, int num, unsigned long size)
{
	stream->buf_num = 0;
	stream->buf_size = size;

//	deb_mem("all in all I will use %lu bytes for streaming\n",num*size);

	for (stream->buf_num = 0; stream->buf_num < num; stream->buf_num++)
	{
//		deb_mem("allocating buffer %d\n",stream->buf_num);
		if (( stream->buf_list[stream->buf_num] = usb_alloc_coherent(stream->udev, size, GFP_ATOMIC, &stream->dma_addr[stream->buf_num]) ) == NULL)
		{
			err("Not enough memory for urb-buffer allocation.\n");
			usb_free_stream_buffers(stream);
			return -ENOMEM;
		}
//		deb_mem("buffer %d: %p (dma: %Lu)\n", stream->buf_num, stream->buf_list[stream->buf_num], (long long)stream->dma_addr[stream->buf_num]);
		memset(stream->buf_list[stream->buf_num],0,size);
		stream->state |= USB_STATE_URB_BUF;
	}
//	deb_mem("allocation successful\n");

	return 0;
}

static int usb_bulk_urb_init(struct usb_data_stream *stream)
{
	int i, j;

	if ((i = usb_allocate_stream_buffers(stream,stream->props.count, stream->props.u.bulk.buffersize)) < 0)
	{
		return i;
	}
	/* allocate the URBs */
	for (i = 0; i < stream->props.count; i++)
	{
		stream->urb_list[i] = usb_alloc_urb(0, GFP_ATOMIC);
		if (!stream->urb_list[i])
		{
//			deb_mem("not enough memory for urb_alloc_urb!.\n");
			for (j = 0; j < i; j++)
			{
				usb_free_urb(stream->urb_list[j]);
			}
			return -ENOMEM;
		}
		usb_fill_bulk_urb( stream->urb_list[i], stream->udev,
			usb_rcvbulkpipe(stream->udev,stream->props.endpoint),
			stream->buf_list[i],
			stream->props.u.bulk.buffersize,
			usb_urb_complete, stream);

		stream->urb_list[i]->transfer_flags = URB_NO_TRANSFER_DMA_MAP;
		stream->urb_list[i]->transfer_dma = stream->dma_addr[i];
		stream->urbs_initialized++;
	}
	return 0;
}

static int usb_isoc_urb_init(struct usb_data_stream *stream)
{
	int i, j;

	if ((i = usb_allocate_stream_buffers(stream,stream->props.count, stream->props.u.isoc.framesize*stream->props.u.isoc.framesperurb)) < 0)
	{
		return i;
	}
	/* allocate the URBs */
	for (i = 0; i < stream->props.count; i++)
	{
		struct urb *urb;
		int frame_offset = 0;

		stream->urb_list[i] = usb_alloc_urb(stream->props.u.isoc.framesperurb, GFP_ATOMIC);
		if (!stream->urb_list[i])
		{
			err("Not enough memory for urb_alloc_urb!\n");
			for (j = 0; j < i; j++)
			{
				usb_free_urb(stream->urb_list[j]);
			}
			return -ENOMEM;
		}

		urb = stream->urb_list[i];

		urb->dev = stream->udev;
		urb->context = stream;
		urb->complete = usb_urb_complete;
		urb->pipe = usb_rcvisocpipe(stream->udev,stream->props.endpoint);
		urb->transfer_flags = URB_ISO_ASAP | URB_NO_TRANSFER_DMA_MAP;
		urb->interval = stream->props.u.isoc.interval;
		urb->number_of_packets = stream->props.u.isoc.framesperurb;
		urb->transfer_buffer_length = stream->buf_size;
		urb->transfer_buffer = stream->buf_list[i];
		urb->transfer_dma = stream->dma_addr[i];

		for (j = 0; j < stream->props.u.isoc.framesperurb; j++)
		{
			urb->iso_frame_desc[j].offset = frame_offset;
			urb->iso_frame_desc[j].length = stream->props.u.isoc.framesize;
			frame_offset += stream->props.u.isoc.framesize;
		}

		stream->urbs_initialized++;
	}
	return 0;
}

int usb_urb_init(struct usb_data_stream *stream, struct usb_data_stream_properties *props)
{
	if (stream == NULL || props == NULL)
	{
		return -EINVAL;
	}
	memcpy(&stream->props, props, sizeof(*props));

	usb_clear_halt(stream->udev,usb_rcvbulkpipe(stream->udev,stream->props.endpoint));

	if (stream->complete == NULL)
	{
		err("There is no data callback - this does not make sense.");
		return -EINVAL;
	}

	switch (stream->props.type)
	{
		case USB_BULK:
			return usb_bulk_urb_init(stream);
		case USB_ISOC:
			return usb_isoc_urb_init(stream);
		default:
			err("Unknown URB-type for data transfer.");
			return -EINVAL;
	}
}

int usb_urb_exit(struct usb_data_stream *stream)
{
	int i;

	usb_urb_kill(stream);

	for (i = 0; i < stream->urbs_initialized; i++)
	{
		if (stream->urb_list[i] != NULL)
		{
			info("Freeing URB no. %d.\n",i);
			/* free the URBs */
			usb_free_urb(stream->urb_list[i]);
		}
	}
	stream->urbs_initialized = 0;

	usb_free_stream_buffers(stream);
	return 0;
}

static void dvb_usb_data_complete(struct usb_data_stream *stream, u8 *buffer, size_t length)
{
	struct dvb_usb_adapter *adap = stream->user_priv;
	if (adap->feedcount > 0 && adap->state & DVB_USB_ADAP_STATE_DVB)
	{
		dvb_dmx_swfilter(&adap->demux, buffer, length);
	}
}

static void dvb_usb_data_complete_204(struct usb_data_stream *stream, u8 *buffer, size_t length)
{
	struct dvb_usb_adapter *adap = stream->user_priv;
	if (adap->feedcount > 0 && adap->state & DVB_USB_ADAP_STATE_DVB)
	{
		dvb_dmx_swfilter_204(&adap->demux, buffer, length);
	}
}

static void dvb_usb_data_complete_raw(struct usb_data_stream *stream, u8 *buffer, size_t length)
{
	struct dvb_usb_adapter *adap = stream->user_priv;
	if (adap->feedcount > 0 && adap->state & DVB_USB_ADAP_STATE_DVB)
	{
		dvb_dmx_swfilter_raw(&adap->demux, buffer, length);
	}
}

int dvb_usb_adapter_stream_init(struct dvb_usb_adapter *adap)
{
	int i, ret = 0;
	for (i = 0; i < adap->props.num_frontends; i++)
	{

		adap->fe_adap[i].stream.udev = adap->dev->udev;
		if (adap->props.fe[i].caps & DVB_USB_ADAP_RECEIVES_204_BYTE_TS)
		{
			adap->fe_adap[i].stream.complete = dvb_usb_data_complete_204;
		}
		else if (adap->props.fe[i].caps & DVB_USB_ADAP_RECEIVES_RAW_PAYLOAD)
		{
			adap->fe_adap[i].stream.complete = dvb_usb_data_complete_raw;
		}
		else
		{
			adap->fe_adap[i].stream.complete  = dvb_usb_data_complete;
		}
		adap->fe_adap[i].stream.user_priv = adap;
		ret = usb_urb_init(&adap->fe_adap[i].stream, &adap->props.fe[i].stream);
		if (ret < 0)
		{
			break;
		}
	}
	return ret;
}

int dvb_usb_adapter_stream_exit(struct dvb_usb_adapter *adap)
{
	int i;

	for (i = 0; i < adap->props.num_frontends; i++)
	{
		usb_urb_exit(&adap->fe_adap[i].stream);
	}
	return 0;
}

//-- dvb-usb-dvb.c

/* does the complete input transfer handling */
static int dvb_usb_ctrl_feed(struct dvb_demux_feed *dvbdmxfeed, int onoff)
{
	struct dvb_usb_adapter *adap = dvbdmxfeed->demux->priv;
	int newfeedcount, ret;

	if (adap == NULL)
	{
		return -ENODEV;
	}
	if ((adap->active_fe < 0)
	||  (adap->active_fe >= adap->num_frontends_initialized))
	{
		return -EINVAL;
	}

	newfeedcount = adap->feedcount + (onoff ? 1 : -1);

	/* stop feed before setting a new pid if there will be no pid anymore */
	if (newfeedcount == 0)
	{
		info("Stop feeding\n");
		usb_urb_kill(&adap->fe_adap[adap->active_fe].stream);

		if (adap->props.fe[adap->active_fe].streaming_ctrl != NULL)
		{
			ret = adap->props.fe[adap->active_fe].streaming_ctrl(adap, 0);
			if (ret < 0)
			{
				err("Error while stopping stream.");
				return ret;
			}
		}
	}

	adap->feedcount = newfeedcount;

	/* activate the pid on the device specific pid_filter */
	info("Setting PID (%s): %5d %04x at index %d '%s'\n", adap->fe_adap[adap->active_fe].pid_filtering ? "yes" : "no", dvbdmxfeed->pid, dvbdmxfeed->pid, dvbdmxfeed->index, onoff ? "on" : "off");

	if (adap->props.fe[adap->active_fe].caps & DVB_USB_ADAP_HAS_PID_FILTER
	&&  adap->fe_adap[adap->active_fe].pid_filtering
	&&  adap->props.fe[adap->active_fe].pid_filter != NULL)
	{
		adap->props.fe[adap->active_fe].pid_filter(adap, dvbdmxfeed->index, dvbdmxfeed->pid, onoff);
	}
	/* start the feed if this was the first feed and there is still a feed
	 * for reception.
	 */
	if (adap->feedcount == onoff && adap->feedcount > 0)
	{
		info("Submitting all URBs\n");
		usb_urb_submit(&adap->fe_adap[adap->active_fe].stream);

		info("Controlling PID parser\n");
		if (adap->props.fe[adap->active_fe].caps & DVB_USB_ADAP_HAS_PID_FILTER
		&&  adap->props.fe[adap->active_fe].caps & DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF
		&&  adap->props.fe[adap->active_fe].pid_filter_ctrl != NULL)
		{
			ret = adap->props.fe[adap->active_fe].pid_filter_ctrl(adap, adap->fe_adap[adap->active_fe].pid_filtering);
			if (ret < 0)
			{
				err("could not handle pid_parser");
				return ret;
			}
		}
		info("Start feeding\n");
		if (adap->props.fe[adap->active_fe].streaming_ctrl != NULL)
		{
			ret = adap->props.fe[adap->active_fe].streaming_ctrl(adap, 1);
			if (ret < 0)
			{
				err("error while enabling fifo.");
				return ret;
			}
		}
	}
	return 0;
}

static int dvb_usb_start_feed(struct dvb_demux_feed *dvbdmxfeed)
{
	info("Start PID: 0x%04x, feedtype: %d\n", dvbdmxfeed->pid, dvbdmxfeed->type);
	return dvb_usb_ctrl_feed(dvbdmxfeed, 1);
}

static int dvb_usb_stop_feed(struct dvb_demux_feed *dvbdmxfeed)
{
	info("Stop PID: 0x%04x, feedtype: %d\n", dvbdmxfeed->pid, dvbdmxfeed->type);
	return dvb_usb_ctrl_feed(dvbdmxfeed, 0);
}

static void dvb_usb_media_device_register(struct dvb_usb_adapter *adap)
{
#ifdef CONFIG_MEDIA_CONTROLLER_DVB
	struct media_device *mdev;
	struct dvb_usb_device *d = adap->dev;
	struct usb_device *udev = d->udev;
	int ret;

	mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
	if (!mdev)
	{
		return;
	}

	mdev->dev = &udev->dev;
	strlcpy(mdev->model, d->desc->name, sizeof(mdev->model));
	if (udev->serial)
	{
		strlcpy(mdev->serial, udev->serial, sizeof(mdev->serial));
	}
	strcpy(mdev->bus_info, udev->devpath);
	mdev->hw_revision = le16_to_cpu(udev->descriptor.bcdDevice);
	mdev->driver_version = LINUX_VERSION_CODE;

	ret = media_device_register(mdev);
	if (ret)
	{
		dev_err(&d->udev->dev, "Could not create a media device. Error: %d\n", ret);
		kfree(mdev);
		return;
	}
	dvb_register_media_controller(&adap->dvb_adap, mdev);

	dev_info(&d->udev->dev, "media controller created\n");
#endif
}

static void dvb_usb_media_device_unregister(struct dvb_usb_adapter *adap)
{
#ifdef CONFIG_MEDIA_CONTROLLER_DVB
	if (!adap->dvb_adap.mdev)
	{
		return;
	}
	media_device_unregister(adap->dvb_adap.mdev);
	kfree(adap->dvb_adap.mdev);
	adap->dvb_adap.mdev = NULL;
#endif
}

int dvb_usb_adapter_dvb_init(struct dvb_usb_adapter *adap, short *adapter_nums)
{
	int i;
	int ret = dvb_register_adapter(&adap->dvb_adap, adap->dev->desc->name, adap->dev->owner, &adap->dev->udev->dev, adapter_nums);

	if (ret < 0)
	{
//		deb_info("dvb_register_adapter failed: error %d", ret);
		err("dvb_register_adapter failed: error %d", ret);
		goto err;
	}
	adap->dvb_adap.priv = adap;

	dvb_usb_media_device_register(adap);

	if (adap->dev->props.read_mac_address)
	{
		if (adap->dev->props.read_mac_address(adap->dev, adap->dvb_adap.proposed_mac) == 0)
		{
			info("MAC address: %pM", adap->dvb_adap.proposed_mac);
		}
		else
		{
			err("MAC address reading failed.");
		}
	}

	adap->demux.dmx.capabilities = DMX_TS_FILTERING | DMX_SECTION_FILTERING;
	adap->demux.priv             = adap;

	adap->demux.filternum        = 0;
	for (i = 0; i < adap->props.num_frontends; i++)
	{
		if (adap->demux.filternum < adap->fe_adap[i].max_feed_count)
		{
			adap->demux.filternum = adap->fe_adap[i].max_feed_count;
		}
	}
	adap->demux.feednum          = adap->demux.filternum;
	adap->demux.start_feed       = dvb_usb_start_feed;
	adap->demux.stop_feed        = dvb_usb_stop_feed;
	adap->demux.write_to_decoder = NULL;
	if ((ret = dvb_dmx_init(&adap->demux)) < 0)
	{
		err("dvb_dmx_init failed: error %d", ret);
		goto err_dmx;
	}

	adap->dmxdev.filternum       = adap->demux.filternum;
	adap->dmxdev.demux           = &adap->demux.dmx;
	adap->dmxdev.capabilities    = 0;
	if ((ret = dvb_dmxdev_init(&adap->dmxdev, &adap->dvb_adap)) < 0)
	{
		err("dvb_dmxdev_init failed: error %d", ret);
		goto err_dmx_dev;
	}

	if ((ret = dvb_net_init(&adap->dvb_adap, &adap->dvb_net, &adap->demux.dmx)) < 0)
	{
		err("dvb_net_init failed: error %d", ret);
		goto err_net_init;
	}

	adap->state |= DVB_USB_ADAP_STATE_DVB;
	return 0;

err_net_init:
	dvb_dmxdev_release(&adap->dmxdev);
err_dmx_dev:
	dvb_dmx_release(&adap->demux);
err_dmx:
	dvb_usb_media_device_unregister(adap);
	dvb_unregister_adapter(&adap->dvb_adap);
err:
	return ret;
}

int dvb_usb_adapter_dvb_exit(struct dvb_usb_adapter *adap)
{
	if (adap->state & DVB_USB_ADAP_STATE_DVB)
	{
//		deb_info("unregistering DVB part\n");
		info("Unregistering DVB part\n");
		dvb_net_release(&adap->dvb_net);
		adap->demux.dmx.close(&adap->demux.dmx);
		dvb_dmxdev_release(&adap->dmxdev);
		dvb_dmx_release(&adap->demux);
		dvb_usb_media_device_unregister(adap);
		dvb_unregister_adapter(&adap->dvb_adap);
		adap->state &= ~DVB_USB_ADAP_STATE_DVB;
	}
	return 0;
}

static int dvb_usb_set_active_fe(struct dvb_frontend *fe, int onoff)
{
	struct dvb_usb_adapter *adap = fe->dvb->priv;

	int ret = (adap->props.frontend_ctrl) ? adap->props.frontend_ctrl(fe, onoff) : 0;

	if (ret < 0)
	{
		err("frontend_ctrl request failed");
		return ret;
	}
	if (onoff)
	{
		adap->active_fe = fe->id;
	}
	return 0;
}

static int dvb_usb_fe_wakeup(struct dvb_frontend *fe)
{
	struct dvb_usb_adapter *adap = fe->dvb->priv;

	dvb_usb_device_power_ctrl(adap->dev, 1);

	dvb_usb_set_active_fe(fe, 1);

	if (adap->fe_adap[fe->id].fe_init)
	{
		adap->fe_adap[fe->id].fe_init(fe);
	}
	return 0;
}

static int dvb_usb_fe_sleep(struct dvb_frontend *fe)
{
	struct dvb_usb_adapter *adap = fe->dvb->priv;

	if (adap->fe_adap[fe->id].fe_sleep)
	{
		adap->fe_adap[fe->id].fe_sleep(fe);
	}
	dvb_usb_set_active_fe(fe, 0);

	return dvb_usb_device_power_ctrl(adap->dev, 0);
}

int dvb_usb_adapter_frontend_init(struct dvb_usb_adapter *adap)
{
	int ret, i;

	/* register all given adapter frontends */
	for (i = 0; i < adap->props.num_frontends; i++)
	{
		if (adap->props.fe[i].frontend_attach == NULL)
		{
			err("Strange: '%s' #%d,%d doesn't want to attach a frontend.", adap->dev->desc->name, adap->id, i);
			return 0;
		}
		ret = adap->props.fe[i].frontend_attach(adap);
		if (ret || adap->fe_adap[i].fe == NULL)
		{
			/* only print error when there is no FE at all */
			if (i == 0)
			{
				err("No frontend was attached by '%s'",	adap->dev->desc->name);
			}
			return 0;
		}

		adap->fe_adap[i].fe->id = i;

		/* re-assign sleep and wakeup functions */
		adap->fe_adap[i].fe_init = adap->fe_adap[i].fe->ops.init;
		adap->fe_adap[i].fe->ops.init  = dvb_usb_fe_wakeup;
		adap->fe_adap[i].fe_sleep = adap->fe_adap[i].fe->ops.sleep;
		adap->fe_adap[i].fe->ops.sleep = dvb_usb_fe_sleep;

		if (dvb_register_frontend(&adap->dvb_adap, adap->fe_adap[i].fe))
		{
			err("Frontend %d registration failed.", i);
			dvb_frontend_detach(adap->fe_adap[i].fe);
			adap->fe_adap[i].fe = NULL;
			/* In error case, do not try register more FEs,
			* still leaving already registered FEs alive. */
			if (i == 0)
			{
				return -ENODEV;
			}
			else
			{
				return 0;
			}
		}

		/* only attach the tuner if the demod is there */
		if (adap->props.fe[i].tuner_attach != NULL)
		{
			adap->props.fe[i].tuner_attach(adap);
		}
		adap->num_frontends_initialized++;
	}

//	dvb_create_media_graph(&adap->dvb_adap);

	return 0;
}

int dvb_usb_adapter_frontend_exit(struct dvb_usb_adapter *adap)
{
	int i = adap->num_frontends_initialized - 1;

	/* unregister all given adapter frontends */
	for (; i >= 0; i--)
	{
		if (adap->fe_adap[i].fe != NULL)
		{
			dvb_unregister_frontend(adap->fe_adap[i].fe);
			dvb_frontend_detach(adap->fe_adap[i].fe);
		}
	}
	adap->num_frontends_initialized = 0;

	return 0;
}

//-- dvb-usb-firmware.c
int dvb_usb_download_firmware(struct usb_device *udev, struct dvb_usb_device_properties *props)
{
	int ret;
	const struct firmware *fw = NULL;

	if ((ret = request_firmware(&fw, props->firmware, &udev->dev)) != 0)
	{
		err("Did not find the firmware file. (%s) Please see linux/Documentation/dvb/ for more details on firmware-problems. (%d)", props->firmware,ret);
		return ret;
	}

	info("Downloading firmware from file '%s'",props->firmware);

	switch (props->usb_ctrl)
	{
//		case CYPRESS_AN2135:
//		case CYPRESS_AN2235:
//		case CYPRESS_FX2:
//		{
//			ret = usb_cypress_load_firmware(udev, fw, props->usb_ctrl);
//			break;
//		}
		case DEVICE_SPECIFIC:
		{
			if (props->download_firmware)
			{
				ret = props->download_firmware(udev,fw);
			}
			else
			{
				err("BUG: driver didn't specified a download_firmware-callback, although it claims to have a DEVICE_SPECIFIC one.");
				ret = -EINVAL;
			}
			break;
		}
		default:
		{
			ret = -EINVAL;
			break;
		}
	}
	release_firmware(fw);
	return ret;
}

//-- dvb-usb-i2c.c

int dvb_usb_i2c_init(struct dvb_usb_device *d)
{
	int ret = 0;

	if (!(d->props.caps & DVB_USB_IS_AN_I2C_ADAPTER))
	{
		return 0;
	}
	if (d->props.i2c_algo == NULL)
	{
		err("No i2c algorithm specified");
		return -EINVAL;
	}

	strlcpy(d->i2c_adap.name, d->desc->name, sizeof(d->i2c_adap.name));
	d->i2c_adap.algo       = d->props.i2c_algo;
	d->i2c_adap.algo_data  = NULL;
	d->i2c_adap.dev.parent = &d->udev->dev;

	i2c_set_adapdata(&d->i2c_adap, d);

	if ((ret = i2c_add_adapter(&d->i2c_adap)) < 0)
	{
		err("Could not add i2c adapter");
	}
	d->state |= DVB_USB_STATE_I2C;

	return ret;
}

int dvb_usb_i2c_exit(struct dvb_usb_device *d)
{
	if (d->state & DVB_USB_STATE_I2C)
	{
		i2c_del_adapter(&d->i2c_adap);
	}
	d->state &= ~DVB_USB_STATE_I2C;
	return 0;
}

//-- dvb-usb-init.c

static int dvb_usb_force_pid_filter_usage;
module_param_named(force_pid_filter_usage, dvb_usb_force_pid_filter_usage, int, 0444);
MODULE_PARM_DESC(force_pid_filter_usage, "Force all dvb-usb-devices to use a PID filter, if any (default: 0).");

static int dvb_usb_adapter_init(struct dvb_usb_device *d, short *adapter_nrs)
{
	struct dvb_usb_adapter *adap;
	int ret, n, o;

	for (n = 0; n < d->props.num_adapters; n++)
	{
		adap = &d->adapter[n];
		adap->dev = d;
		adap->id  = n;

		memcpy(&adap->props, &d->props.adapter[n], sizeof(struct dvb_usb_adapter_properties));

		for (o = 0; o < adap->props.num_frontends; o++)
		{
			struct dvb_usb_adapter_fe_properties *props = &adap->props.fe[o];
			/* speed - when running at FULL speed we need a HW PID filter */
			if (d->udev->speed == USB_SPEED_FULL && !(props->caps & DVB_USB_ADAP_HAS_PID_FILTER))
			{
				err("This USB2.0 device cannot be run on a USB1.1 port. (it lacks a hardware PID filter)");
				return -ENODEV;
			}

			if ((d->udev->speed == USB_SPEED_FULL && props->caps & DVB_USB_ADAP_HAS_PID_FILTER)
			||  (props->caps & DVB_USB_ADAP_NEED_PID_FILTERING))
			{
				info("Will use the device's hardware PID filter (table count: %d).", props->pid_filter_count);
				adap->fe_adap[o].pid_filtering  = 1;
				adap->fe_adap[o].max_feed_count = props->pid_filter_count;
			}
			else
			{
				info("Will pass the complete MPEG2 transport stream to the software demuxer.");
				adap->fe_adap[o].pid_filtering  = 0;
				adap->fe_adap[o].max_feed_count = 255;
			}

			if (!adap->fe_adap[o].pid_filtering
			&&  dvb_usb_force_pid_filter_usage
			&&  props->caps & DVB_USB_ADAP_HAS_PID_FILTER)
			{
				info("PID filter enabled by module option.");
				adap->fe_adap[o].pid_filtering  = 1;
				adap->fe_adap[o].max_feed_count = props->pid_filter_count;
			}

			if (props->size_of_priv > 0)
			{
				adap->fe_adap[o].priv = kzalloc(props->size_of_priv, GFP_KERNEL);
				if (adap->fe_adap[o].priv == NULL)
				{
					err("No memory for priv for adapter %d fe %d.", n, o);
					return -ENOMEM;
				}
			}
		}

		if (adap->props.size_of_priv > 0)
		{
			adap->priv = kzalloc(adap->props.size_of_priv, GFP_KERNEL);
			if (adap->priv == NULL)
			{
				err("no memory for priv for adapter %d.", n);
				return -ENOMEM;
			}
		}

		if ((ret = dvb_usb_adapter_stream_init(adap))
		||  (ret = dvb_usb_adapter_dvb_init(adap, adapter_nrs))
		||  (ret = dvb_usb_adapter_frontend_init(adap)))
		{
			return ret;
		}

		/* use exclusive FE lock if there is multiple shared FEs */
		if (adap->fe_adap[1].fe)
		{
			adap->dvb_adap.mfe_shared = 1;
		}
		d->num_adapters_initialized++;
		d->state |= DVB_USB_STATE_DVB;
	}

	/*
	 * when reloading the driver w/o replugging the device
	 * sometimes a timeout occurs, this helps
	 */
	if (d->props.generic_bulk_ctrl_endpoint != 0)
	{
		usb_clear_halt(d->udev, usb_sndbulkpipe(d->udev, d->props.generic_bulk_ctrl_endpoint));
		usb_clear_halt(d->udev, usb_rcvbulkpipe(d->udev, d->props.generic_bulk_ctrl_endpoint));
	}

	return 0;
}

static int dvb_usb_adapter_exit(struct dvb_usb_device *d)
{
	int n;

	for (n = 0; n < d->num_adapters_initialized; n++)
	{
		dvb_usb_adapter_frontend_exit(&d->adapter[n]);
		dvb_usb_adapter_dvb_exit(&d->adapter[n]);
		dvb_usb_adapter_stream_exit(&d->adapter[n]);
		kfree(d->adapter[n].priv);
	}
	d->num_adapters_initialized = 0;
	d->state &= ~DVB_USB_STATE_DVB;
	return 0;
}

/* general initialization functions */
static int dvb_usb_exit(struct dvb_usb_device *d)
{
//	deb_info("state before exiting everything: %x\n", d->state);
	info("State before exiting everything: %x\n", d->state);
#if defined RC_ENABLE
	dvb_usb_remote_exit(d);
#endif
	dvb_usb_adapter_exit(d);
	dvb_usb_i2c_exit(d);
//	deb_info("state should be zero now: %x\n", d->state);
	info("State should be zero now: %x\n", d->state);
	d->state = DVB_USB_STATE_INIT;
	kfree(d->priv);
	kfree(d);
	return 0;
}

int dvb_usb_device_power_ctrl(struct dvb_usb_device *d, int onoff)
{
	if (onoff)
	{
		d->powered++;
	}
	else
	{
		d->powered--;
	}
	if (d->powered == 0 || (onoff && d->powered == 1))
	{ /* when switching from 1 to 0 or from 0 to 1 */
//		deb_info("power control: %d", onoff);
		info("Power control: %d", onoff);
		if (d->props.power_ctrl)
		{
			return d->props.power_ctrl(d, onoff);
		}
	}
	return 0;
}

static int dvb_usb_init(struct dvb_usb_device *d, short *adapter_nums)
{
	int ret = 0;

	mutex_init(&d->usb_mutex);
	mutex_init(&d->i2c_mutex);

	d->state = DVB_USB_STATE_INIT;

	if (d->props.size_of_priv > 0)
	{
		d->priv = kzalloc(d->props.size_of_priv, GFP_KERNEL);
		if (d->priv == NULL)
		{
			err("No memory for priv in 'struct dvb_usb_device'");
			return -ENOMEM;
		}
	}

	/* check the capabilities and set appropriate variables */
	dvb_usb_device_power_ctrl(d, 1);

	if ((ret = dvb_usb_i2c_init(d))
	||  (ret = dvb_usb_adapter_init(d, adapter_nums)))
	{
		dvb_usb_exit(d);
		return ret;
	}

#if defined RC_ENABLE
	if ((ret = dvb_usb_remote_init(d)))
	{
		err("Could not initialize remote control.");
	}
#endif

	dvb_usb_device_power_ctrl(d, 0);

	return 0;
}

/* determine the name and the state of the just found USB device */
static struct dvb_usb_device_description *dvb_usb_find_device(struct usb_device *udev, struct dvb_usb_device_properties *props, int *cold)
{
	int i, j;
	struct dvb_usb_device_description *desc = NULL;

	*cold = -1;

	for (i = 0; i < props->num_device_descs; i++) {

		for (j = 0; j < DVB_USB_ID_MAX_NUM && props->devices[i].cold_ids[j] != NULL; j++)
		{
//			deb_info("check for cold %x %x", props->devices[i].cold_ids[j]->idVendor, props->devices[i].cold_ids[j]->idProduct);
			info("Check for cold %04x %04x", props->devices[i].cold_ids[j]->idVendor, props->devices[i].cold_ids[j]->idProduct);
			if (props->devices[i].cold_ids[j]->idVendor  == le16_to_cpu(udev->descriptor.idVendor)
			&&  props->devices[i].cold_ids[j]->idProduct == le16_to_cpu(udev->descriptor.idProduct))
			{
				*cold = 1;
				desc = &props->devices[i];
				break;
			}
		}

		if (desc != NULL)
		{
			break;
		}

		for (j = 0; j < DVB_USB_ID_MAX_NUM && props->devices[i].warm_ids[j] != NULL; j++)
		{
//			deb_info("check for warm %x %x\n", props->devices[i].warm_ids[j]->idVendor, props->devices[i].warm_ids[j]->idProduct);
			info("Check for warm %04x %04x", props->devices[i].warm_ids[j]->idVendor, props->devices[i].warm_ids[j]->idProduct);
			if (props->devices[i].warm_ids[j]->idVendor == le16_to_cpu(udev->descriptor.idVendor)
			&&  props->devices[i].warm_ids[j]->idProduct == le16_to_cpu(udev->descriptor.idProduct))
			{
				*cold = 0;
				desc = &props->devices[i];
				break;
			}
		}
	}

	if (desc != NULL && props->identify_state != NULL)
	{
		props->identify_state(udev, props, &desc, cold);
	}
	return desc;
}

/*
 * USB
 */
int dvb_usb_device_init1(struct usb_interface *intf, struct dvb_usb_device_properties *props, struct module *owner, struct dvb_usb_device **du, short *adapter_nums)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct dvb_usb_device *d = NULL;
	struct dvb_usb_device_description *desc = NULL;

	int ret = -ENOMEM, cold = 0;

	if (du != NULL)
	{
		*du = NULL;
	}

	if ((desc = dvb_usb_find_device(udev, props, &cold)) == NULL)
	{
//		deb_err("something went very wrong, device was not found in current device list - let's see what comes next.\n");
		err("Something went very wrong, device was not found in current device list\n - let's see what comes next");
		return -ENODEV;
	}

	if (cold)
	{
		info("Found a '%s' in cold state, will try to load a firmware", desc->name);
		ret = dvb_usb_download_firmware(udev, props);
		if (!props->no_reconnect || ret != 0)
		{
			return ret;
		}
	}

	info("Found a '%s' in warm state", desc->name);
	d = kzalloc(sizeof(struct dvb_usb_device), GFP_KERNEL);
	if (d == NULL)
	{
		err("No memory for 'struct dvb_usb_device'");
		return -ENOMEM;
	}

	d->udev = udev;
	memcpy(&d->props, props, sizeof(struct dvb_usb_device_properties));
	d->desc = desc;
	d->owner = owner;

	usb_set_intfdata(intf, d);

	if (du != NULL)
	{
		*du = d;
	}
	ret = dvb_usb_init(d, adapter_nums);

	if (ret == 0)
	{
		info("%s successfully initialized and connected", desc->name);
	}
	else
	{
		info("%s: Error while loading driver (%d)", desc->name, ret);
	}
	return ret;
}

void dvb_usb_device_exit1(struct usb_interface *intf)
{
	struct dvb_usb_device *d = usb_get_intfdata(intf);
	const char *name = "generic DVB-USB module";

	usb_set_intfdata(intf, NULL);
	if (d != NULL && d->desc != NULL)
	{
		name = d->desc->name;
		dvb_usb_exit(d);
	}
	info("%s successfully deinitialized and disconnected.", name);

}
// vim:ts=4
