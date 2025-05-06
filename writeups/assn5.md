Assignment 5 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [7] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the NetworkInterface:
[

    For the NetworkInterface class, I added the following private members:
    - map<uint32_t, std::pair<EthernetAddress, size_t>> _mapping_cache{}: map-form cache for storing the mapping between IP address and Ethernet address, and elapsed time for each IP address
    - deque<std::pair<InternetDatagram, Address>> _ARP_queue{}: queue for storing the IP datagram waiting to receive the ARP reply

    And the functions below are the important functions that are implemented in NetworkInterface class. 

    (1) send_datagram

    - if the matched Ethernet address for the given input IP address is equal to ETHERNET_BROADCAST (i.e., the input IP address is still waiting for the ARP reply for less than 5 seconds), don't send a second ARP request — just wait for a reply to the first one (just return).
    - if the destination Ethernet address is already known (IPv4), create an EthernetFrame and set the payload, header's src, header's dst, and header's type appropriately. Then, push the frame to _frames_out to send the frame.
        1. payload: set it with the input Internet datagram dgram
        2. header().src: set it with the private member variable _ethernet_address
        3. header().dst: set it with the matched Ethernet address for the input IP address in _mapping_cache
        4. header().type: set it with TYPE_IPv4
    - if the destination Ethernet address is unknown (ARP), create an ARPMessage and set the members of the ARPMessage appropriately.
        1. arp_message.sender_ip_address: set it with _ip_address.ipv4_numeric()
        2. arp_message.sender_ethernet_address: set it with _ethernet_address
        3. arp_message.target_ip_address: set it with next_hop.ipv4_numeric()
        4. arp_message.opcode: set it with the REQUEST type
    Then, create an EthernetFrame and set the payload, header's src, header's dst, and header's type appropriately. Then, push the frame to _frames_out to send the frame, and push the corresponding input datagram to _ARP_queue. Set the mapped Ethernet address and the mapped elapsed time of the input IP address in _mapping_cache to {ETHERNET_BROADCAST, 0}.


    (2) recv_frame

    - if the input frame's header().dst is not equal to ETHERNET_BROADCAST and not equal to _ethernet_address, return nullopt. 
    - if the inbound frame is IPv4, parse the payload as an InternetDatagram using parse(). Then, return the datagram if the parse result is equal to ParseResult::NoError.
    - if the inbound frame is ARP:
        - parse the payload as an arp_message using parse(). If the parse result is equal to ParseResult::NoError:
            - set _mapping_cache's mapped value for arp_message.sender_ip_address to {arp_message.sender_ethernet_address, 0}.
            1. if the ARPMessage is of ARP request type:
                - create an ARPMessage(arp_reply) and set the members of the arp_reply appropriately. Specifically, set the opcode of the arp_reply to OPCODE_REPLY(REPLY type).
                - create an EthernetFrame and set the payload, header's src, header's dst, and header's type appropriately. Specifically, set the src address to _ethernet_address, and set the dst address to arp_message.sender_ethernet_address.
                - push the frame to _frames_out to send the frame to where the ARP request message was sent.
            2. if the ARPMessage is of ARP reply type:
                - pop the pending InternetDatagram from _ARP_queue and send it to the target address.


    (3) tick

    - while iterator i = _mapping_cache.begin() iterates over _mapping_cache,
        - add ms_since_last_tick to i->second.second (i.e., the elapsed time element in the pair <EthernetAddress, size_t> of _mapping_cache).
        1. if the EthernetAddress of the corresponding _mapping_cache iterator is equal to ETHERNET_BROADCAST:
            - if the corresponding elapsed time is greater than or equal to 5000 ms, do _mapping_cache.erase(i).
            - else, increment i.
        2. else, 
            - if the elapsed time of the corresponding _mapping_cache iterator is greater than or equal to 30000 ms, do _mapping_cache.erase(i).
            - else, increment i.
    

]

Implementation Challenges:
[


    - When implementing send_datagram and recv_frame, it was challenging that I had to think about which Ethernet address to assign when creating an EthernetFrame and setting the source and destination Ethernet addresses in the header. In addition, considering which values to assign for the IP address and Ethernet address when creating an ARPMessage was also tough.
    - I wasn’t sure how to implement storing the IP-to-Ethernet address mapping in _mapping_cache for a fixed period of time. Specifically, I spent time thinking about how to retain an Ethernet address for 5 seconds when a reply has not yet been received, and how to store other mappings for 30 seconds.


]

Remaining Bugs:
[

    As far a I know, there’re no remaining bugs in my code.

]
