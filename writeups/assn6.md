Assignment 6 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the Router:
[

    For the Router class, I added the following private members:
    -vector<tuple<uint32_t, uint8_t, optional<Address>, size_t>> _routing_table: a routing table for storing the routes, each consisting of a route prefix, a prefix length, the address of the next hop, and an interface number

    And the functions below are the important functions that are implemented in the Router class. 
    
    (1) add_route

    - I've chosen the vector data structure for storing each route in the routing table, and used the tuple structure for grouping {a route prefix, a prefix length, the address of the next hop, and an interface number} in each route.
    - Create a tuple with the given inputs of the add_route function, and push the tuple back to _routing_table.

    (2) route_one_datagram

    - If the TTL of the input dgram's header is less than or equal to 1, drop the datagram.
    - Decrement the TTL of the dgram's header.
    - Set longest_prefix_len to -1. (Initialize longest_prefix_len)
    - While iterating over _routing_table:
         - Get prefix_length with get<1>(*i), which means getting the second element of the i-th tuple in _routing_table.
         - If prefix_length is equal to 0, set mask to 0.
         - Else, set mask to ~((1 << (32 - prefix_length)) - 1) to keep only the most significant prefix_length bits of dgram_dst_addr.
         - If dgram_dst_addr & mask is equal to the route prefix of the i-th tuple in _routing_table, and if the prefix_length is greater than longest_prefix_len, set longest_prefix_match to *i, and set longest_prefix_len to prefix_length.
    - If longest_prefix_len is still equal to -1, drop the datagram.
    - Get next_hop and interface_num of the longest-prefix-match route with get<2>(longest_prefix_match), get<3>(longest_prefix_match).
    - If next_hop is not empty (if next_hop contains the IP address of the next router along the path), call interface(interface_num).send_datagram(dgram, next_hop.value()).
    - If next_hop is empty (if the router is directly attached to the network so that the next_hop is the datagram's destination address), call interface(interface_num).send_datagram(dgram, Address::from_ipv4_numeric(dgram_dst_addr)).

]

Implementation Challenges:
[

    - The most tricky part was implementing the code that checks whether the most significant prefix_length bits of the datagram's destination address match those of the route prefix. In other words, it was tricky to implement the process of finding the routes that match the datagram's destination address, and I had to think carefully about how to create the appropriate mask.

]

Remaining Bugs:
[

    As far a I know, there’re no remaining bugs in my code.
    
]
