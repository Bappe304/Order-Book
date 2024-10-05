#include "Orderbook.h"

#include <numeric>
#include <chrono>
#include <ctime>

void Orderbook::PruneGoodForDayOrders()
{
    using namespace std::chrono;
    const auto end = hours(16);

        while(true)
        {
            const auto now = system_clock::now(); // returns the time as a 'time_point' object, which is not human-readable.
            const auto now_c = system_clock::to_time_t(now);  // converts 'now' into a 'std::time_t' which is more of a traditional way.(Number of seconds since Unix epoch: January 1, 1970) 
            std::tm now_parts;
            localtime_s(&now_parts, &now_c);  // Converts the 'now_c' from std::time_t to 'now_parts' which is std::tm (basically splits into components like year, month, day, hours, min, sec). 
        
            if(now_parts.tm_hour >= end.count())
                now_parts.tm_mday += 1;
            
            now_parts.tm_hour = end.count();
            now_parts.tm_min = 0;
            now_parts.tm_sec = 0;

            auto next = system_clock::from_time_t(mktime(&now_parts));
            auto till = next - now + milliseconds(100);

            {
                std::unique_lock ordersLock { ordersMutex_ };  // unique_lock acquires the lock right away

                if(shutdown_.load(std::memory_order_acquire) || 
                        shutdownConditionVariable_.wait_for(ordersLock, till) == std::cv_status::no_timeout)
                        return;
            }

            OrderIDs orderIds;
            
            {
                std::scoped_lock ordersLock{ ordersMutex_};

                for(const auto& [_, entry] : orders_)
                {
                    const auto& [order, _] = entry;
                    if(order->GetOrderType() != OrderType::GoodForDay)
                        continue;
                    
                    orderIds.push_back(order->GetOrderId());
                }
            }

            CancelOrders(orderIds);                
        }
}


void Orderbook::CancelOrders(OrderIDs orderIds)
{
    std::scoped_lock ordersLock{ ordersMutex_ };
    for(const auto& orderId : orderIds)
        CancelOrderInternal(orderId);
}

        
void Orderbook::CancelOrderInternal(OrderID orderId)
{
    if(!orders_.contains(orderId))
        return;
    
    const auto& [order, iterator] = orders_.at(orderId);
    orders_.erase(orderId);

    if(order->GetSide() == Side::Buy)
    {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(iterator);
        if(orders.empty())
            bids_.erase(price);
    }
    else
    {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(iterator);
        if(orders.empty())
            asks_.erase(price);
    }

    OnOrderCancelled(order);
}


void Orderbook::OnOrderCancelled(OrderPointer order)
{
    UpdateLevelData(order->GetPrice(), order->GetRemainingQuantity(), LevelData::Action::Remove);
}

void Orderbook::OnOrderAdded(OrderPointer order)
{
    UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(), LevelData::Action::Add);
}

void Orderbook::OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled)
{   
    // Updates according to FullyFilled or Not.
    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}

void Orderbook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action)
{
    // We get the LevelData for the specific price 
    auto& data = data_[price];

    // We change the total count of orders on that specific price level according to the 'Action'
    // Match --> We do not change anything in this case as it might be possible that the order wasnt fully matched.
    // Add --> We add 1 to the count of orders. (In the case of AddOrder() ).
    // Remove --> We reduce the count of orders by 1. (In the case of CancelOrder() ). 
    data.count_ += action == LevelData::Action::Remove ? -1 : action == LevelData::Action::Add ? 1 : 0 ;
    
    // Similar implementation for the quantity of the price level
    if(action == LevelData::Action::Remove || action == LevelData::Action::Match)
    {
        data.quantity_ -= quantity;
    }
    else
    {
        data.quantity_ += quantity;
    }

    // In case there are no orders left for that particular price level we delete the price level.
    if(data.count_ == 0)
        data_.erase(price);
}
   

// This function is to check if we can Fully Fill an order on the buy or sell side on a given pricelevel.
// The main goal is to asses whether there is enough liquidity  in the order book at the relevant price levels 
// satisfy the quantity requested by the order.
bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity) const
{
    if(!CanMatch(side, price))
        return false;
    
    std::optional<Price> threshold;

    if(side == Side::Buy)
    {
        const auto [bestAskPrice, _] = *asks_.begin();
        threshold = bestAskPrice;
    }
    else
    {
        const auto [bestbidPrice, _] = *bids_.begin();
        threshold = bestbidPrice;
    }

    for(const auto& [levelPrice, levelData] : data_)
    {
        if(threshold.has_value() && 
            (side == Side::Buy && threshold.value() > levelPrice) ||
            (side == Side::Sell && threshold.value() < levelPrice))
            continue;
        
        if((side == Side::Buy && levelPrice > price) ||
            (side == Side::Sell &&  levelPrice < price))
            continue;
        
        if(quantity <= levelData.quantity_)
            return true;
        
        quantity -= levelData.quantity_;
    }

    return false;
}



   
bool Orderbook::CanMatch(Side side, Price price) const
{
    if(side == Side::Buy)
    {
        if(asks_.empty())
            return false;
        
        /*
        ** The following line returns the pair {Price, OrderPointers}, 'bestAsk' represents the Price in the pair and '_' underscore represents the OrderPointers. 
        ** Using _ is a common convention for "unused variables" like the OrderPointers list here.
        */
        const auto& [bestAsk, _] = *asks_.begin();
        return price >= bestAsk;   // Return 'True' if the Buy price is greater or equal to the Ask Price
    }
    else
    {
        if(bids_.empty())
            return false;
        
        const auto& [bestBid, _] = *bids_.begin();
        return price <= bestBid;  // Return 'True' if the Ask price is greater or equal to the Buy Price
    }
}



Trades Orderbook::MatchOrders()
{
    Trades trades;
    trades.reserve(orders_.size());

    while(true)
    {
        if(bids_.empty() || asks_.empty())
            break;
        
        auto& [bidPrice, bids] = *bids_.begin();
        auto& [askPrice, asks] = *asks_.begin();

        if(bidPrice < askPrice)
            break;
        
        while(bids.size() && asks.size())
        {
            auto& bid = bids.front(); // 'bid' here is the topmost pointer to one of the orders in the OrderPointers list at a particular price.
            auto& ask = asks.front(); // 'ask' here is the topmost pointer to one of the orders in the OrderPointers list at a particular price.

            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            bid->Fill(quantity);
            ask->Fill(quantity);

            //Removing the 'bid' order incase it is completely filled 
            if(bid->isFilled())
            {
                bids.pop_front();
                orders_.erase(bid->GetOrderId());
            }
            //Removing the 'ask' order incase it is completely filled
            if(ask->isFilled())
            {
                asks.pop_front();
                orders_.erase(ask->GetOrderId());
            }

            //Now finally when the trade is matched we create a 'Trade Object' and add it in the 'trades' vector
            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity},
                TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity}
                });

            OnOrderMatched(bid->GetPrice(), quantity, bid->isFilled());
            OnOrderMatched(ask->GetPrice(), quantity, ask->isFilled());
        }

        //Here we remove the whole 'Price Level' if we have no more orders on that price level
            if(bids.empty())
            {
                bids_.erase(bidPrice);
                data_.erase(bidPrice);
            }
                
            
            if(asks.empty())
            {
                asks_.erase(askPrice);
                data_.erase(askPrice);
            }
    }

    /*
    * Now after the order matching loop we check if there are any orders remaining in bids and asks lists and if that particular 
        order is of the type 'Fill&Kill' then we just cancel that order.
    */
    if(!bids_.empty())
    {
        auto& [_, bids] = *bids_.begin();
        auto& order = bids.front();
        if(order->GetOrderType() == OrderType::FillAndKill)
            CancelOrder(order->GetOrderId());
    }

    if((!asks_.empty()))
    {
        auto& [_,asks] = *asks_.begin();
        auto& order = asks.front();
        if(order->GetOrderType() == OrderType::FillAndKill)
            CancelOrder(order->GetOrderId());
    }

    return trades;
}

Orderbook::Orderbook() : ordersPruneThread_{ [this] { PruneGoodForDayOrders(); } } { }

Orderbook::~Orderbook()
{
    shutdown_.store(true,std::memory_order_release);
        shutdownConditionVariable_.notify_one();
        ordersPruneThread_.join();
}

    
Trades Orderbook::AddOrder(OrderPointer order)
{
    //Making sure we dont have duplicate orders
    if( orders_.contains(order->GetOrderId()))
        return { };

    // Till now the market orders are being executed at the worst price available
    if(order->GetOrderType() == OrderType::Market)
    {
        if(order->GetSide() == Side::Buy && !asks_.empty())
        {
            const auto& [worstAsk, _] = *asks_.rbegin();
            order->ToGoodTillCancel(worstAsk);
        }
        else if(order->GetSide() == Side::Sell && !bids_.empty())
        {
            const auto& [worstBid, _] = *bids_.rbegin();
            order->ToGoodTillCancel(worstBid);
        }
        else
            return { };
    }
    
    //Not adding the order to the order book in case the order is Fill&Kill and we are not able to match it at the given moment
    if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
        return { };
    
    if(order->GetOrderType() == OrderType::FillOrKill && !CanFullyFill(order->GetSide(), order->GetPrice(), order->GetInitialQuantity()))
        return { };


    OrderPointers::iterator iterator;

    if(order->GetSide() == Side::Buy)
    {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size()-1);
    }
    else
    {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size()-1);
    }

    orders_.insert({order->GetOrderId(), OrderEntry{ order, iterator }});
    return MatchOrders();
}




       
void Orderbook::CancelOrder(OrderID orderId)
{
    std::scoped_lock ordersLock { ordersMutex_ };
    CancelOrderInternal(orderId);
}


Trades Orderbook::ModifyOrder(OrderModify order)
{
    OrderType orderType;
    {
        std::scoped_lock ordersLock{ ordersMutex_ };
        if(!orders_.contains(order.GetOrderId()))
            return { };
    
        const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
        orderType = existingOrder->GetOrderType();
    }
    
    CancelOrder(order.GetOrderId()); 
    // Here we deleted this order from the 'orders_'(unordered_map) and therefore we need 'ToOrderPointer' 
    // to create a new order with all the details from the order that previously existed and make changes to it.
    return AddOrder(order.ToOrderPointer(orderType));
}

        
std::size_t Orderbook::Size() const 
{ 
    std::scoped_lock ordersLock{ ordersMutex_ };
    return orders_.size(); 
}


OrderbookLevelInfos  Orderbook::GetOrderInfos() const 
{
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());

    // Lambda Function
    auto CreateLevelInfos = [](Price price, const OrderPointers& orders)
    {
        return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
            [](Quantity runningSum, const OrderPointer& order)
            { return runningSum + order->GetRemainingQuantity(); }) };
    };

    for(const auto& [price, orders] : bids_)
        bidInfos.push_back(CreateLevelInfos(price, orders));
    
    for(const auto& [price, orders] : asks_)
        askInfos.push_back(CreateLevelInfos(price, orders));
    
    return OrderbookLevelInfos{ bidInfos, askInfos}; 
}

