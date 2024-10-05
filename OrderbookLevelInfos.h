#pragma once

#include "LevelInfo.h"

//Class for creating Buy and Sell side LevelInfo vectors in the Orderbook
class OrderbookLevelInfos
{
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : bids_{ bids }
        , asks_{ asks }
    { }

    const LevelInfos& GetBids() const{ return bids_; }
    const LevelInfos& GetAsks() const{ return asks_; }

private:
    LevelInfos bids_;
    LevelInfos asks_;

};