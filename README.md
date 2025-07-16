# Enhanced Backtesting System

A powerful C++ backtesting framework that supports complex trading strategies with technical indicators, mathematical functions, and logical operations.

## Quick Start

```cpp
#include "DataLoader.h"
#include "BacktestEngine.h"
#include "Parser.h"

// Load data and run backtest
DataLoader loader;
loader.loadCSV("data/BTCUSDT.csv");
auto strategy = Parser::parseExpression("close > SMA(close, 20)");
BacktestEngine engine;
auto result = engine.run(loader.getData(), *strategy, 10000);
```

## Available Data Fields

| Field | Description |
|-------|-------------|
| `open` | Opening price |
| `close` | Closing price |
| `high` | Highest price |
| `low` | Lowest price |
| `volume` | Trading volume |

## Operators

### Arithmetic Operators
| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `close + open` |
| `-` | Subtraction | `close - open` |
| `*` | Multiplication | `close * 2` |
| `/` | Division | `close / open` |
| `-` (unary) | Negation | `-close` |

### Comparison Operators
| Operator | Description | Example | Returns |
|----------|-------------|---------|---------|
| `>` | Greater than | `close > open` | 1.0 if true, 0.0 if false |
| `<` | Less than | `close < open` | 1.0 if true, 0.0 if false |
| `>=` | Greater than or equal | `close >= high` | 1.0 if true, 0.0 if false |
| `<=` | Less than or equal | `close <= low` | 1.0 if true, 0.0 if false |
| `==` | Equal to | `close == open` | 1.0 if true, 0.0 if false |
| `!=` | Not equal to | `close != open` | 1.0 if true, 0.0 if false |

### Logical Operators
| Operator | Description | Example |
|----------|-------------|---------|
| `&&` | Logical AND | `close > open && volume > 1000` |
| `\|\|` | Logical OR | `close > high \|\| close < low` |

### Grouping
| Operator | Description | Example |
|----------|-------------|---------|
| `()` | Parentheses for grouping | `(close + open) / 2` |

## Technical Indicators

### Moving Averages
| Function | Syntax | Description | Example |
|----------|--------|-------------|---------|
| `SMA` | `SMA(expression, period)` | Simple Moving Average | `SMA(close, 20)` |
| `EMA` | `EMA(expression, period)` | Exponential Moving Average | `EMA(close, 50)` |

### Mathematical Functions
| Function | Syntax | Description | Example |
|----------|--------|-------------|---------|
| `log` | `log(expression)` | Natural logarithm | `log(close)` |
| `exp` | `exp(expression)` | Exponential function | `exp(close / 1000)` |
| `abs` | `abs(expression)` | Absolute value | `abs(close - open)` |
| `sqrt` | `sqrt(expression)` | Square root | `sqrt(volume)` |

## Constants

You can use numeric constants directly in expressions:

```cpp
close > 100           // Price above 100
volume > 50000        // Volume above 50,000
(close - open) / open > 0.02  // 2% price increase
```

## Operator Precedence

The system follows standard mathematical precedence (highest to lowest):

1. **Parentheses**: `()`
2. **Unary operators**: `-` (negation)
3. **Multiplication/Division**: `*`, `/`
4. **Addition/Subtraction**: `+`, `-`
5. **Comparison operators**: `>`, `<`, `>=`, `<=`, `==`, `!=`
6. **Logical AND**: `&&`
7. **Logical OR**: `||`

## Strategy Examples

### Basic Price Action
```cpp
// Bullish candle
"close > open"

// Price above yesterday's high
"close > high"

// Strong volume
"volume > 100000"
```

### Technical Analysis
```cpp
// Golden cross (bullish signal)
"SMA(close, 20) > SMA(close, 50)"

// Price above moving average
"close > EMA(close, 20)"

// Moving average crossover with volume confirmation
"SMA(close, 20) > SMA(close, 50) && volume > SMA(volume, 20)"
```

### Complex Conditions
```cpp
// Breakout strategy
"close > SMA(close, 20) && volume > SMA(volume, 10) * 1.5"

// Mean reversion
"close < SMA(close, 20) * 0.95 && volume > 50000"

// Momentum strategy
"(close - SMA(close, 20)) / SMA(close, 20) > 0.02"
```

### Mathematical Strategies
```cpp
// Log returns
"log(close) - log(open) > 0.01"

// Volatility-based
"abs(close - open) / open > 0.02"

// Price normalization
"(close - SMA(close, 20)) / sqrt(volume) > 0.001"
```

### Multi-Condition Strategies
```cpp
// Trend following with volume
"close > SMA(close, 20) && SMA(close, 20) > SMA(close, 50) && volume > SMA(volume, 20)"

// Reversal strategy
"close < SMA(close, 20) * 0.98 && volume > SMA(volume, 10) * 2"

// Breakout with confirmation
"close > high && volume > SMA(volume, 20) * 1.5 && close > SMA(close, 50)"
```

## Signal Interpretation

The expression result is interpreted as a trading signal:
- **Positive values**: Long position (buy signal)
- **Negative values**: Short position (sell signal)
- **Zero**: No position (neutral)

Signal magnitude affects position size (capped at 100% of capital).

## Error Handling

The system provides detailed error messages for:
- Invalid syntax
- Unknown operators or functions
- Missing parentheses
- Invalid function arguments
- Division by zero (returns 0.0)
- Log of non-positive numbers (returns 0.0)

## Performance Notes

- **SMA/EMA calculations**: Efficient recursive computation
- **Function evaluation**: Optimized for real-time backtesting
- **Memory usage**: Minimal overhead for large datasets
- **Parsing**: One-time parse, multiple evaluations

## Limitations

1. **Historical data dependency**: SMA/EMA require sufficient historical data
2. **Function domains**: Some functions have domain restrictions (log > 0, sqrt >= 0)
3. **Precision**: Uses double precision floating-point arithmetic
4. **Case sensitivity**: Function names are case-insensitive, field names are lowercase

## Advanced Usage

### Custom Indicators
```cpp
// RSI-like calculation
"(SMA(close, 14) - SMA(close, 28)) / SMA(close, 28)"

// Bollinger Band position
"(close - SMA(close, 20)) / sqrt(SMA((close - SMA(close, 20)) * (close - SMA(close, 20)), 20))"

// Price momentum
"(close - SMA(close, 10)) / SMA(close, 10) - (SMA(close, 10) - SMA(close, 20)) / SMA(close, 20)"
```

### Risk Management
```cpp
// Position sizing based on volatility
"(close > SMA(close, 20)) * (1 / abs(close - SMA(close, 20)))"

// Stop-loss conditions
"close > open && (close - low) / close < 0.02"
```

## Building and Running

1. Compile with C++17 or later
2. Link required libraries
3. Ensure data files are in correct format
4. Run with appropriate command-line arguments

```bash
g++ -std=c++17 -O3 main.cpp DataLoader.cpp BacktestEngine.cpp Parser.cpp -o backtest
./backtest
```
