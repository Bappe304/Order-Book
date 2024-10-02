#pragma once

enum class OrderType
{
    GoodTillCancel,
    FillAndKill,
    FillOrKill,
    GoodForDay, // These are basically GTC orders which are cancelled by the exchange automatically at the end of the trading day
    Market,    // Market Orders gurantees execution but not the preferred price. Allows partial fill too.
};