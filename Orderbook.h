#pragma once

#include <map>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Usings.h"
#include "Order.h"
#include "OrderModify.h"
#include "OrderbookLevelInfos.h"
#include "Trade.h"

class Orderbook
{
private:

    struct OrderEntry
    {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
    };

    

    //Basically the structure of the map looks like --> map[int] = {list}
    
    /*
    * The bids_ map is ordered in the 'Descending Price' fashion, meaning the highest price comes first,
    * as this is the maximum amount the buyer is willing to pay for that certain stock or security.
    */
    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    
    /*
    * The asks_ map is ordered in the 'Ascending Price' fashion, meaning the lowest price comes first,
    * as this is the minimum amount the seller is looking for selling that stock.
    */
    std::map<Price, OrderPointers, std::less<Price>> asks_;

    /*
    * Here we have used an unordered_map for a quick O(1) lookup to any order provided its OrderID is given
    */
    std::unordered_map<OrderID, OrderEntry> orders_;


















};