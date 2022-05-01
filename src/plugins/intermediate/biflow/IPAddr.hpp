/**
 * @file   IPAddr.hpp
 * @brief  IPv4/IPv6 address
 * @author Lukas Hutak <lukas.hutak@hotmail.com>
 * @date   September 2020
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPFIXEXP_IPADDR_HPP
#define IPFIXEXP_IPADDR_HPP

#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>

/**
 * @brief IPv4/IPv6 address representation
 */
union IPAddr {
    uint8_t  u8[16];
    uint16_t u16[8];
    uint32_t u32[4];
    uint64_t u64[2];

    /**
     * @brief Default constructor (zeroes)
     */
    IPAddr()
    {
        memset(this, 0, sizeof(*this));
    }

    /**
     * @brief Create an IPv4/IPv6 address from a string
     * @param[in] str  IP address in text form
     * @throw std::invalid_argument if the IP address is not valid
     */
    explicit IPAddr(const std::string &str);

    /**
     * @brief Create an IPv4 address (from 4 bytes array)
     */
    explicit IPAddr(const uint32_t ip4)
    {
        u64[0] = 0;
        u32[2] = htonl(0x0000FFFF);
        u32[3] = ip4;
    };

    /**
     * @brief Create an IPv6 address (from 16 bytes array)
     */
    explicit IPAddr(const uint8_t ip6[16])
    {
        std::memcpy(&u8[0], &ip6[0], 16U);
    };

    /**
     * @brief Check if the address is IPv4
     */
    bool
    is_ip4() const
    {
        return u64[0] == 0 && u32[2] == htonl(0x0000FFFF);
    };

    /**
     * @brief Check if the address is IPv6
     */
    bool
    is_ip6() const
    {
        return !is_ip4();
    };

    /**
     * @brief Check if the address is invalid/unspecified
     */
    bool
    is_null() const
    {
        if (u64[0] == 0 && u64[1] == 0)  {
            return 1;
        }

        if (is_ip4() && get_ip4_as_uint() == 0) {
            return 1;
        }

        return 0;
    }

    /**
     * @brief Get IPv4 address as one 32b number
     * @returns Address in network byte order.
     */
    uint32_t get_ip4_as_uint() const
    {
        assert(is_ip4() && "Not an IPv4 address!");
        return u32[3];
    };

    /**
     * @brief Get pointer to byte array of IPv4 address
     * @note Address is in network byte order!
     * @returns Pointer to the first byte of IP address data structure.
     */
    uint8_t *
    get_ip4_as_bytes()
    {
        assert((is_ip4() || is_null()) && "Not an IPv4 address!");
        return &u8[12];
    };

    /**
     * @brief Get pointer to byte array of IPv6 address
     * @note Address is in network byte order!
     * @returns Pointer to the first byte of IP address data structure.
     */
    uint8_t *
    get_ip6_as_bytes()
    {
        assert(is_ip6() && "Not an IPv6 address!");
        return &u8[0];
    };

    /**
     * @brief Convert IP address to string
     */
    std::string to_string() const;

    IPAddr operator&(const IPAddr &rhs) const
    {
        assert((is_ip4() == rhs.is_ip4()) && "IP type missmatch!");

        IPAddr result{*this};
        result.u64[0] &= rhs.u64[0];
        result.u64[1] &= rhs.u64[1];
        return result;
    };

    IPAddr& operator&=(const IPAddr &rhs)
    {
        assert((is_ip4() == rhs.is_ip4()) && "IP type missmatch!");

        u64[0] &= rhs.u64[0];
        u64[1] &= rhs.u64[1];
        return *this;
    };

    IPAddr operator|(const IPAddr &rhs) const
    {
        assert((is_ip4() == rhs.is_ip4()) && "IP type missmatch!");

        IPAddr result{*this};
        result.u64[0] |= rhs.u64[0];
        result.u64[1] |= rhs.u64[1];
        return result;
    };

    IPAddr& operator|=(const IPAddr &rhs)
    {
        assert((is_ip4() == rhs.is_ip4()) && "IP type missmatch!");

        u64[0] |= rhs.u64[0];
        u64[1] |= rhs.u64[1];
        return *this;
    };

    // Standard comparison operators
    inline bool
    operator==(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) == 0;
    };

    inline bool
    operator!=(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) != 0;
    };

    inline bool
    operator<(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) < 0;
    };

    inline bool
    operator>(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) > 0;
    };

    inline bool
    operator<=(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) <= 0;
    };

    inline bool
    operator>=(const IPAddr &other) const
    {
        return memcmp(this, &other, sizeof(*this)) >= 0;
    };
};

static_assert(sizeof(IPAddr) == 16U, "Unexpected union size!");

/**
 * @brief Add conversion to I/O streams
 */
std::ostream &operator<<(std::ostream &out, const IPAddr &ip);

/**
 * @brief Apply network mask on the IP address
 * @param[in] mask 0-32 for IPv4 or 0-128 for IPv6
 * @throw std::invalid_argument if the mask is out of range!
 */
IPAddr operator/(const IPAddr &ip, unsigned int mask);

#endif // IPFIXEXP_IPADDR_HPP