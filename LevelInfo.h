#pragma once

#include "Usings.h"

//LevelInfo is basically the different price levels in the Buy and Sell sides of an Orderbook
struct LevelInfo
{
    Price price;
    Quantity quantity_;
};

//We use vector of LevelInfo objects to store multiple price levels in the Buy and Sell sides
using LevelInfos = std::vector<LevelInfo>;