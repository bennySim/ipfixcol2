/**
 * @file   IPAddr.cpp
 * @brief  IPv4/IPv6 address (implementation)
 * @author Lukas Hutak <lukas.hutak@hotmail.com>
 * @date   September 2020
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdexcept>
#include "IPAddr.hpp"

static IPAddr
mask_ip4(unsigned int mask)
{
    uint32_t bits;

    if (mask > 32U) {
        throw std::invalid_argument("Invalid IPv4 mask");
    }

    bits = (mask != 0)
        ? htonl((~(uint32_t) 0) << (32 - mask))
        : htonl(0);

    return IPAddr(bits);
}

static IPAddr
mask_ip6(unsigned int mask)
{
    IPAddr result{};
    int j = 0;

    if (mask > 128U) {
        throw std::invalid_argument("Invalid IPv6 mask");
    }

	for (int i = mask; i > 0; i -= 8) {
		uint8_t *byte = &result.u8[j++];

		if (i >= 8) {
			*byte = 0xFF;
		} else {
			*byte = (0xFFU << (8 - i));
		}
	}

    return result;
}

IPAddr::IPAddr(const std::string &str)
{
    struct in_addr ip4;
    struct in6_addr ip6;

    if (inet_pton(AF_INET, str.c_str(), &ip4) == 1) {
        u64[0] = 0;
        u32[2] = htonl(0x0000FFFF);
        memcpy(&u32[3], &ip4, 4U);
        return;
    }

    if (inet_pton(AF_INET6, str.c_str(), &ip6) == 1) {
        memcpy(&u8[0], &ip6, 16U);
        return;
    }

    throw std::invalid_argument("IPAddr: Not an IP address!");
}

std::string
IPAddr::to_string() const
{
    char buffer[INET6_ADDRSTRLEN];
    if (is_ip4()) {
        inet_ntop(AF_INET, &u32[3], buffer, sizeof(buffer));
    } else {
        inet_ntop(AF_INET6, &u8[0], buffer, sizeof(buffer));
    }
    return std::string(buffer);
};

std::ostream &operator<<(std::ostream &out, const IPAddr &ip)
{
    return out << ip.to_string();
}

IPAddr operator/(const IPAddr &ip, unsigned int mask)
{
    IPAddr bit_mask = ip.is_ip4() ? mask_ip4(mask) : mask_ip6(mask);
    return (ip & bit_mask);
}
