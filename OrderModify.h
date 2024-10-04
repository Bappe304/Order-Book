#pragma once

#include "Order.h"


// Order Modify is basically used to modify an already existing order, since we already have a pointer pointing to the previous order when we make changes to that 
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

    OrderPointer ToOrderPointer(OrderType type) const
    {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderID orderid_;
    Side side_;
    Price price_;
    Quantity quantity_;
};