// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <ipts/protocol.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <configure.h>
#include <filesystem>
#include <ini.h>
#include <string>

namespace iptsd::config {

struct iptsd_config_device {
	u16 vendor;
	u16 product;
};

static bool to_bool(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	return value == "true" || value == "yes" || value == "on" || value == "1";
}

static int parse_dev(void *user, const char *c_section, const char *c_name, const char *c_value)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	auto *dev = reinterpret_cast<struct iptsd_config_device *>(user);

	std::string section(c_section);
	std::string name(c_name);
	std::string value(c_value);

	if (section != "Device")
		return 1;

	if (name == "Vendor")
		dev->vendor = std::stoi(value, nullptr, 16);

	if (name == "Product")
		dev->product = std::stoi(value, nullptr, 16);

	return 1;
}

static int parse_conf(void *user, const char *c_section, const char *c_name, const char *c_value)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	auto *config = reinterpret_cast<Config *>(user);

	std::string section(c_section);
	std::string name(c_name);
	std::string value(c_value);

	if (section == "Config" && name == "InvertX")
		config->invert_x = to_bool(value);

	if (section == "Config" && name == "InvertY")
		config->invert_y = to_bool(value);

	if (section == "Config" && name == "Width")
		config->width = std::stof(value);

	if (section == "Config" && name == "Height")
		config->height = std::stof(value);

	if (section == "Touch" && name == "CheckCone")
		config->touch_check_cone = to_bool(value);

	if (section == "Touch" && name == "CheckStability")
		config->touch_check_stability = to_bool(value);

	if (section == "Touch" && name == "DisableOnPalm")
		config->touch_disable_on_palm = to_bool(value);

	if (section == "Touch" && name == "DisableOnStylus")
		config->touch_disable_on_stylus = to_bool(value);

	if (section == "Contacts" && name == "Detection") {
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		config->contacts_detection = value;
	}

	if (section == "Contacts" && name == "FingerSize")
		config->contacts_finger_size = std::stof(value);

	if (section == "Contacts" && name == "ThumbSize")
		config->contacts_thumb_size = std::stof(value);

	if (section == "Contacts" && name == "ThumbAspect")
		config->contacts_thumb_aspect = std::stof(value);

	if (section == "Contacts" && name == "PalmAspect")
		config->contacts_palm_aspect = std::stof(value);

	if (section == "Contacts" && name == "SizeThreshold")
		config->contacts_size_thresh = std::stof(value);

	if (section == "Contacts" && name == "PositionThreshold")
		config->contacts_position_thresh = std::stof(value);

	if (section == "Contacts" && name == "DistanceThreshold")
		config->contacts_distance_thresh = std::stof(value);

	if (section == "Cone" && name == "Angle")
		config->cone_angle = std::stof(value);

	if (section == "Cone" && name == "Distance")
		config->cone_distance = std::stof(value);

	if (section == "DFT" && name == "PositionMinAmp")
		config->dft_position_min_amp = std::stoi(value);

	if (section == "DFT" && name == "PositionMinMag")
		config->dft_position_min_mag = std::stoi(value);

	if (section == "DFT" && name == "PositionExp")
		config->dft_position_exp = std::stof(value);

	if (section == "DFT" && name == "ButtonMinMag")
		config->dft_button_min_mag = std::stoi(value);

	if (section == "DFT" && name == "FreqMinMag")
		config->dft_freq_min_mag = std::stoi(value);

	if (section == "DFT" && name == "TiltMinMag")
		config->dft_tilt_min_mag = std::stoi(value);

	if (section == "DFT" && name == "TiltDistance")
		config->dft_tilt_distance = std::stof(value);

	if (section == "DFT" && name == "TipDistance")
		config->dft_tip_distance = std::stof(value);

	return 1;
}

void Config::load_dir(const std::string &name)
{
	if (!std::filesystem::exists(name))
		return;

	for (auto &p : std::filesystem::directory_iterator(name)) {
		if (!p.is_regular_file())
			continue;

		struct iptsd_config_device dev {};
		ini_parse(p.path().c_str(), parse_dev, &dev);

		if (dev.vendor != this->vendor || dev.product != this->product)
			continue;

		ini_parse(p.path().c_str(), parse_conf, this);
	}
}

Config::Config(i16 vendor, i16 product) : vendor {vendor}, product {product}
{
	this->load_dir(IPTSD_CONFIG_DIR);
	this->load_dir("./etc/config");

	if (std::filesystem::exists(IPTSD_CONFIG_FILE))
		ini_parse(IPTSD_CONFIG_FILE, parse_conf, this);
}

contacts::Config Config::contacts() const
{
	contacts::Config config {};

	config.max_contacts = IPTS_MAX_CONTACTS;
	config.width = this->width;
	config.height = this->height;
	config.invert_x = this->invert_x;
	config.invert_y = this->invert_y;

	if (this->contacts_detection == "basic")
		config.mode = contacts::BlobDetection::BASIC;
	else if (this->contacts_detection == "advanced")
		config.mode = contacts::BlobDetection::ADVANCED;

	config.finger_size = this->contacts_finger_size;
	config.thumb_size = this->contacts_thumb_size;
	config.thumb_aspect = this->contacts_thumb_aspect;
	config.palm_aspect = this->contacts_palm_aspect;

	config.size_thresh = this->contacts_size_thresh;
	config.position_thresh = this->contacts_position_thresh;
	config.dist_thresh = this->contacts_distance_thresh;

	return config;
}

} // namespace iptsd::config
