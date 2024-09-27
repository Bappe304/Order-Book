#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <list>
#include <ctime> 
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>
#include <format>

enum class OrderType
{
    GoodTillCancel,
    FillAndKill
};

//Defines the two sides of an order book
enum class Side
{
    Buy,
    Sell
};

//Using Aliases helps us making the code more readable and eays to understand
using Price = std::int32_t;
using Quantity  = std::uint32_t;
using OrderID = std::uint64_t;

struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};

//Alias for the actual order book
using LevelInfos = std::vector<LevelInfo>;

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

class Order
{
public:
    Order(OrderType ordertype, OrderID orderid, Side side, Price price, Quantity quantity)
        : ordertype_{ ordertype }
        , orderid_{ orderid }
        , side_{ side }
        , price_{ price }
        , initialQuantity_{ quantity }
        , remainingQuantity_{ quantity }
    { }

    OrderType GetOrderType() const{ return ordertype_; };
    OrderID GetOrderId() const{ return orderid_; }
    Side GetSide() const{ return side_; }
    Price GetPrice() const{ return price_; }
    Quantity GetInitialQuantity() const{ return initialQuantity_; }
    Quantity GetRemainingQuantity() const{ return remainingQuantity_; }
    Quantity GetFilledQuantity() const{ return GetInitialQuantity() - GetRemainingQuantity(); }
    bool isFilled() const { return GetRemainingQuantity() == 0; }
    void Fill(Quantity quantity)
    {
        if(quantity > GetRemainingQuantity())
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its rmeaning quantity.", GetOrderId()))
        
        remainingQuantity_ -= quantity;
    }

private:
    OrderType ordertype_ ;
    OrderID orderid_ ;
    Side side_ ;
    Price price_ ;
    Quantity initialQuantity_ ;
    Quantity remainingQuantity_ ;
};

/*
** Here we are using a shared pointer(smart pointer) that manages the lifetime of a dynamically allocated object.
   allowing multiple shared owenership of the object. Here this 'Order Pointer is created on the heap.
** It is more efficient to use "std::make_shared" insted of manually using "new" because it allocates both
   the object and the control block (which stores the reference counts) in a single memory location. 
*/
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;


class OrderModify
{
public:
    OrderModify(OrderID orderid, Side side, Price price, Quantity quantity)
        : orderid_{ orderid }
        , side_{ side }
        , price_{ price }
        , quantity_{ quantity }
    { }

    OrderID GetOrderId() const{ return orderid_; }
    Side GetSide() const{ return side_; }
    Price GetPrice() const{ return price_; }
    Quantity GetQuantity() const{ return quantity_; }

    OrderPointer ToOrderPointer(OrderType type)
    {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderID orderid_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

/*
** We are creating Trade Object to represent the situation wehen a trade happens.
** Now a Trade object is bascially an aggregation of two trade-info objects --> Bid TradeInfo and Ask TradeInfo
*/
struct TradeInfo
{
    OrderID orderid_;
    Price price_;
    Quantity quantity_;
};

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade{ bidTrade }
        , askTrade{ askTrade }
    { }

    const TradeInfo& GetBidTrade() const { return bidTrade };
    const TradeInfo& GetAskTrade() const { return askTrade };

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

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
    * The asks_ map is ordered in the 'Ascending Price' fashion, meaning the lowestt price comes first,
    * as this is the minimum amount the seller is looking for selling that stock.
    */
    std::map<Price, OrderPointers, std::less<Price>> asks_;

    /*
    * Here we have used an unordered_map for a quick O(1) lookup to any order provided its OrderID is given
    */
    std::unordered_map<OrderID, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const
    {
        if(side == Side::Buy)
        {
            if(asks_.empty())
                return false;
            
            //The following line returns the pair {Price, OrderPointers}, bestAsk represents the Price in the 
            // pair and _ underscore represents the OrderPointers. Using _ is a common convention for 
            // "unused variables" like the OrderPointers list here.
            const auto& [bestAsk, _] = *asks_.begin();
            return price >= bestAsk;
        }
        else
        {
            if(bids_.empty())
                return false;
            
            const auto& [bestBid, _] = *bids_begin();
            return price <= bestBid;
        }
    }

    Trades MatchOrders()
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
                auto& bid = bids.front();
                auto& ask = asks.front();

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
                    asks->pop_front();
                    orders_.erase(ask->GetOrderId());
                }

                //Here we remove the whole 'Price Level' if we have no more orders on that price level
                if(bids.empty())
                    bids_.erase(bidPrice);
                
                if(asks.empty())
                    asks_.erase(askPrice);

                //Now finally when the trade is matched we create a 'Trade Object' and add it in the 'trades' vector
                trades.push_back(Trade{
                    TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity},
                    TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity}
                    });
            }
        }

        /*
        * Now after the order matching loop we check if there are any bids and asks remaining and if that particular
        * bid or ask is of the type 'Fill&Kill' then we just cancel the remaining order.
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

    public:

        Trades AddOrder(OrderPointer order)
        {
            //Making sure we dont have duplicate orders
            if( orders_.contains(order->GetOrderId()))
                return { };

            //Not adding the order to the order book in case the order is Fill&Kill and we are not able to match it at the given moment
            if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
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

        void CancelOrder(OrderId orderId)
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
                    orders.erase(price);
            }
        }

        Trades MatchOrder(OrderModify order)
        {
            if(!orders_.contains(order.GetOrderId()))
                return { };
            
            const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
            CancelOrder(order.GetOrderId());
            return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
        }

        std::size_t Size() const { return orders_.size(); }

        OrderbookLevelInfos  GetOrderInfos() const 
        {
            LevelInfos bidInfos, askInfos;
            bidInfos.reserve(orders_.size());
            askInfos.reserve(orders_.size());

            auto CreateLevelInfos = [](Price price, const OrderPointers& orders)
            {
                return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), {Quantity}0,
                    [](std::size_t runningSum, const OrderPointer& order)
                    { return runningSum + order->GetRemainingQuantity(); }) };
            };

            for(const auto& [price, orders] : bids_)
                bidInfos.push_back(CreateLevelInfos(price, orders));
            
            for(const auto& [price, orders] : asks_)
                askInfos.push_back(CreateLevelInfos(price, orders));
            
            return OrderbookLevelInfos{ bidInfos, askInfos}; 
        }

        

};





// Just testing for now
int main()
{
    Orderbook orderbook;
    const OrderID orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << std::endl; // 1
    orderbook.CancelOrder(orderId);
    std::cout << orderbook.Size() << std::endl; // 0
    return 0;
}