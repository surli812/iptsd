/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_IPTS_PARSER_HPP_
#define _IPTSD_IPTS_PARSER_HPP_

#include "protocol.h"

#include <common/types.hpp>

#include <cstddef>
#include <functional>
#include <vector>

class IptsSingletouchData {
public:
	bool touch = 0;
	u16 x = 0;
	u16 y = 0;
};

class IptsStylusData {
public:
	u16 timestamp = 0;
	u16 mode = 0;
	u16 x = 0;
	u16 y = 0;
	u16 pressure = 0;
	u16 altitude = 0;
	u16 azimuth = 0;
	u32 serial = 0;
};

class IptsHeatmap {
public:
	u8 width;
	u8 height;
	u16 size;

	u8 y_min = 0;
	u8 y_max = 0;
	u8 x_min = 0;
	u8 x_max = 0;
	u8 z_min = 0;
	u8 z_max = 0;
	u16 count = 0;
	u32 timestamp = 0;

	std::vector<u8> data;

	IptsHeatmap(u8 w, u8 h) : width(w), height(h), size(w * h), data(size) {};
};

class IptsParser {
private:
	u8 *data;
	size_t size;
	size_t current;

	IptsHeatmap *heatmap;

	void read(void *dest, size_t size);
	void skip(size_t size);
	void reset(void);

	template <typename T> T read(void)
	{
		T value;
		this->read(&value, sizeof(value));
		return value;
	}

	void parse_payload(void);
	void parse_hid(void);

	void parse_stylus(struct ipts_payload_frame frame);
	void parse_stylus_report(struct ipts_report report);

	void parse_heatmap(struct ipts_payload_frame frame);
	void parse_heatmap_data(struct ipts_heatmap_dim dim, struct ipts_heatmap_timestamp time);

public:
	std::function<void(IptsSingletouchData)> on_singletouch;
	std::function<void(IptsStylusData)> on_stylus;
	std::function<void(IptsHeatmap)> on_heatmap;

	IptsParser(size_t size);
	~IptsParser(void);

	u8 *buffer();
	void parse();
};

#endif /* _IPTSD_IPTS_PARSER_HPP_ */
