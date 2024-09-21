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

sturct 
int main()
{
    return 0;
}