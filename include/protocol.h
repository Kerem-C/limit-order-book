#pragma once
#include <cstdint>

// Disable struct padding, to match network wire format
#pragma pack(push, 1) 

struct OrderMessage {
    char type; // 'A' for Add, 'C' for Cancel, 'M' for Market Execute
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    char side; // 'B' for Bid, 'S' for Ask
};

#pragma pack(pop) // 18 bytes per message