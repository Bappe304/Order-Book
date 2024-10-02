#pragma once

#include <list>
#include <exception>
#include <format>

#include "OrderType.h"
#include "Side.h"
#include "Usings.h"
#include "Constants.h"


class Order
{
    Order(OrderType orderType, OrderID orderid, Side side, Price price, Quantity quantity)
        : orderType_{ orderType }
        , orderId_{ orderid }
        , side_{ side }
        , price_{ price }
        , initialQuantity_{ quantity }
        , remainingQuantity_{ quantity }
        { }


        Order(OrderID orderId, Side side, Quantity quantity)
            :Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
        { }

        OrderType GetOrderType() const{ return orderType_; };
        OrderID GetOrderId() const{ return orderId_; }
        Side GetSide() const{ return side_; }
        Price GetPrice() const{ return price_; }
        Quantity GetInitialQuantity() const{ return initialQuantity_; }
        Quantity GetRemainingQuantity() const{ return remainingQuantity_; }
        Quantity GetFilledQuantity() const{ return GetInitialQuantity() - GetRemainingQuantity(); }
        bool isFilled() const { return GetRemainingQuantity() == 0; }
        void Fill(Quantity quantity)
        {
            if(quantity > GetRemainingQuantity())
                throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));
            
            remainingQuantity_ -= quantity;
        }

private:
    OrderType orderType_ ;
    OrderID orderId_ ;
    Side side_ ;
    Price price_ ;
    Quantity initialQuantity_ ;
    Quantity remainingQuantity_ ;
};