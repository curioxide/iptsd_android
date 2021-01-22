/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_UTILS_HPP_
#define _IPTSD_UTILS_HPP_

#include <cstdint>
#include <string>
#include <system_error>

class Utils {
public:
	static void msleep(uint64_t msecs);
	static std::system_error cerror(std::string msg);
	static void signal(int signum, void (*handler)(int));
};

#endif /* _IPTSD_UTILS_HPP_ */