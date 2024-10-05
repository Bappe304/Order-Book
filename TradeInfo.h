#pragma once

#include "Usings.h"


/*
** We are creating Trade Object to represent the situation when a trade happens.
** Now a Trade object is bascially an aggregation of two trade-info objects --> Bid TradeInfo and Ask TradeInfo
*/
struct TradeInfo
{
    OrderID orderid_;
    Price price_;
    Quantity quantity_;
};