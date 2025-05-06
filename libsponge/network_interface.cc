#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    // if matched ethernet address is equal to EHTERNET_BROADCAST(if the address is still waiting for the ARP reply for
    // less than 5 seconds), don't send a second ARP request - just wait for a reply to the first one(just return!)
    if (_mapping_cache.find(next_hop_ip) != _mapping_cache.end() &&
        _mapping_cache.find(next_hop_ip)->second.first == ETHERNET_BROADCAST) {
        return;
    }

    // if the destination Ethernet address is already known(IPv4)
    if (_mapping_cache.find(next_hop_ip) != _mapping_cache.end()) {
        EthernetFrame frame;
        frame.payload().append(dgram.serialize());
        frame.header().src = _ethernet_address;
        frame.header().dst = _mapping_cache[next_hop_ip].first;
        frame.header().type = EthernetHeader::TYPE_IPv4;

        _frames_out.push(frame);
    } else {  // if the destination Ethernet address is unknown(ARP)
        // convert IP address of the current interface(src) to raw 32-bit representation
        const uint32_t src_ip = _ip_address.ipv4_numeric();

        ARPMessage arp_message;
        arp_message.sender_ip_address = src_ip;
        arp_message.sender_ethernet_address = _ethernet_address;
        arp_message.target_ip_address = next_hop_ip;
        arp_message.opcode = arp_message.OPCODE_REQUEST;

        EthernetFrame frame;
        frame.payload().append(arp_message.serialize());
        frame.header().src = _ethernet_address;
        frame.header().dst = ETHERNET_BROADCAST;
        frame.header().type = EthernetHeader::TYPE_ARP;

        _frames_out.push(frame);
        _ARP_queue.push_back({dgram, next_hop});

        _mapping_cache[next_hop_ip] = {ETHERNET_BROADCAST, 0};  // set mapped ethernet address to ETHERNET_BROADCAST
        // so that it can be identified that the ip address hasn't received corresponding ARP reply(corresponding
        // ethernet address)
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != ETHERNET_BROADCAST && frame.header().dst != _ethernet_address)
        return nullopt;

    if (frame.header().type ==
        EthernetHeader::TYPE_IPv4) {  // if the inbound frame is IPv4, parse the payload as an InternetDatagram
        InternetDatagram datagram;
        if (datagram.parse(frame.payload()) == ParseResult::NoError) {
            return datagram;
        }
    }

    else if (frame.header().type ==
             EthernetHeader::TYPE_ARP) {  // if the inbound frame is ARP, parse the payload as an ARPMessage
        ARPMessage arp_message;
        if (arp_message.parse(frame.payload()) == ParseResult::NoError) {
            _mapping_cache[arp_message.sender_ip_address] = {arp_message.sender_ethernet_address, 0};

            if (arp_message.opcode ==
                arp_message.OPCODE_REQUEST) {  // if it's an ARP request, send an appropriate ARP reply
                if (_ip_address.ipv4_numeric() == arp_message.target_ip_address) {
                    ARPMessage arp_reply;
                    arp_reply.sender_ip_address = _ip_address.ipv4_numeric();
                    arp_reply.sender_ethernet_address = _ethernet_address;
                    arp_reply.target_ip_address = arp_message.sender_ip_address;
                    arp_reply.target_ethernet_address = arp_message.sender_ethernet_address;
                    arp_reply.opcode = arp_reply.OPCODE_REPLY;

                    EthernetFrame frame_reply;
                    frame_reply.payload().append(arp_reply.serialize());
                    frame_reply.header().src = _ethernet_address;
                    frame_reply.header().dst = arp_reply.target_ethernet_address;
                    frame_reply.header().type = EthernetHeader::TYPE_ARP;

                    _frames_out.push(frame_reply);
                }
            }

            else if (arp_message.opcode ==
                     arp_message.OPCODE_REPLY) {  // if it's an ARP reply, pop the pending InternetDatagram from
                                                  // _ARP_queue and send it to the target address
                for (auto i = _ARP_queue.cbegin(); i != _ARP_queue.cend();) {
                    if (i->second.ipv4_numeric() == arp_message.sender_ip_address) {
                        send_datagram(i->first, i->second);
                        i = _ARP_queue.erase(i);
                        break;
                    } else
                        i++;
                }
            }
        }
    }

    return nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    for (auto i = _mapping_cache.begin(); i != _mapping_cache.end();) {
        i->second.second += ms_since_last_tick;
        if (i->second.first == ETHERNET_BROADCAST) {
            if (i->second.second >= 5000)
                i = _mapping_cache.erase(i);
            else
                i++;
        } else {  // remember the mapping between the sender's IP address and Ethernet address for 30 seconds
            if (i->second.second >= 30000)
                i = _mapping_cache.erase(i);
            else
                i++;
        }
    }
}
