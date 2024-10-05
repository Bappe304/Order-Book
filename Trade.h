#pragma once

#include "TradeInfo.h"

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade_{ bidTrade }
        , askTrade_{ askTrade }
    { }

    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};


//Here we are using a vector to store all Trades that take place. These trade objects consist of various bidTrade and askTrade structure objects.
using Trades = std::vector<Trade>;