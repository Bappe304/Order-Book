#pragma once

#include "Usings.h"

struct LevelInfo
{
    Price price;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;