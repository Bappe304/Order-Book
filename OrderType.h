#pragma once

enum class OrderType
{
    GoodTillCancel,
    FillAndKill, // In these we fill whatever quantity is available at the time and then kill the rest of the order.
    FillOrKill, // In these we either fill the whole order quantity or incase we can't then just kill the whole order.
                    // A FillOrKill Order never adds a new order to the order book
    GoodForDay, // These are basically GTC orders which are cancelled by the exchange automatically at the end of the trading day
    Market,    // Market Orders gurantees execution but not the preferred price. Allows partial fill too.
};