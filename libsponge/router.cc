#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    // insert the tuple into the routing table
    _routing_table.push_back(make_tuple(route_prefix, prefix_length, next_hop, interface_num));
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // drop the datagram if TTL was already zero, or hits zero after the decrement
    if (dgram.header().ttl <= 1)
        return;
    // decrement the TTL
    dgram.header().ttl--;

    uint32_t dgram_dst_addr = dgram.header().dst;

    tuple<uint32_t, uint8_t, optional<Address>, size_t> longest_prefix_match;
    int16_t longest_prefix_len = -1;

    for (auto i = _routing_table.begin(); i != _routing_table.end(); i++) {
        uint8_t prefix_length = get<1>(*i);
        uint32_t mask;

        // if prefix_length == 0, set mask to 0(to prevent shifting a 32-bit integer by 32 bits)
        if (prefix_length == 0)
            mask = 0;
        else {
            // first, left shit 1 by (32 - prefix_length), and substract 1 from it.
            // e.g. if prefix_length = 17, 1 << (32 - prefix_length) = 00000000 00000000 10000000 00000000
            // and 1 << (32 - prefix_length) - 1 = 00000000 00000000 01111111 11111111
            // finally, mask = 11111111 11111111 10000000 00000000 (it can keep only the most significant prefix_length
            // bits of dgram_dst_addr)
            mask = ~((1 << (32 - prefix_length)) - 1);
        }

        // if the destination address of the datagram matches routes in the routing table
        if ((dgram_dst_addr & mask) == get<0>(*i)) {
            // if the prefix length of the route is larger than longest_prefix_len
            if (prefix_length > longest_prefix_len) {
                longest_prefix_match = *i;
                longest_prefix_len = prefix_length;
            }
        }
    }

    // if no routes matched, drop the datagram
    if (longest_prefix_len == -1)
        return;

    // get next_hop, interface_num of the longest-prefix-match route
    optional<Address> next_hop = get<2>(longest_prefix_match);
    size_t interface_num = get<3>(longest_prefix_match);

    if (next_hop.has_value()) {  // if the next_hop is not empty, then it conatins the IP address of the next router
                                 // along the path
        interface(interface_num).send_datagram(dgram, next_hop.value());
    } else {  // if theb next_hop is empty, then the router is directly attached to the network
        // so the next_hop is the datagram's destination address
        interface(interface_num).send_datagram(dgram, Address::from_ipv4_numeric(dgram_dst_addr));
    }
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
