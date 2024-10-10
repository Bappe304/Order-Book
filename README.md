# C++ Order-Book
## Overview
This project implements an order book system for managing and processing trade orders, specifically designed for use in trading applications. The system includes functionality for handling bid and ask orders, tracking order modifications, and maintaining trade information.

## Contents
- **Constants.h**: Defines constants used across the system.
- **Order.h / OrderModify.h**: Manages individual order details and modifications.
- **Trade.h / TradeInfo.h**: Handles trade data, including bid and ask trade aggregation.
- **Orderbook.cpp / Orderbook.h**: Core files for the order book, responsible for managing trades, levels, and orders.
- **test.cpp**: Contains test cases for validating system functionality.

## Supported Order Types
This order book supports a variety of order types, including:

1. **Limit Orders**: Orders placed at a specific price level, executed only at the set price or better.
2. **Market Orders**: Orders to buy or sell immediately at the best available price.
3. **Fill or Kill (FOK)**: Orders that must be filled in their entirety immediately or canceled.
4. **Fill and Kill (FAK)**: Orders that are partially filled immediately, with any unfilled portion canceled.
5. **Good for the Day (GFD)**: Orders that remain active only for the trading day and are canceled at the day’s end.
6. **Good Till Cancel (GTC)**: Orders that remain active until fully filled or manually canceled.

These order types offer flexibility for various trading strategies, making the system adaptable for diverse trading environments.

## Getting Started
To get started, clone the repository:
```bash
git clone https://github.com/Bappe304/Order-Book.git
```

## Usage
1. Include the necessary headers in your application.
2. Compile and link the project files with a C++ compiler.
3. Run `test.cpp` to verify system functionality.

## License
This project is licensed under the MIT License.

---

