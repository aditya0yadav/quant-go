
# 📝 **Backtesting System Documentation**

## 📦 **Module Overview**

This project implements a **flexible expression parser and evaluation system** for backtesting trading strategies. It supports:

✅ Mathematical, logical, and comparison operations
✅ Field accessors (`open`, `close`, `high`, `low`, `volume`)
✅ Common functions (`SMA`, `EMA`, `log`, `exp`, `abs`, `sqrt`)
✅ Conditional expressions
✅ CSV data loading with dynamic column mappings
✅ Strategy abstraction with entry and exit conditions

---

## 🔧 **Core Components**

### 1. **DataLoader**

**Header**: `DataLoader.h`
**Purpose**: Loads and maps market data from CSV files into structured `DataPoint` objects.

#### **Key Members**

| Member                                               | Description                      |
| ---------------------------------------------------- | -------------------------------- |
| `std::vector<DataPoint> data`                        | Parsed market data rows          |
| `std::map<std::string, int> columnIndex`             | Maps column names to CSV indices |
| `std::map<std::string, nlohmann::json> columnConfig` | JSON config for column mapping   |
| `std::string symbol, timeframe`                      | Metadata for the dataset         |

#### **Key Methods**

* `void loadMapping(const std::string& configFile);`
  Loads JSON config mapping CSV columns to data fields.

* `void loadCSV(const std::string& filename, const std::string& symbolName, const std::string& tf);`
  Loads CSV data for the given **symbol** and **timeframe**.

* `const std::vector<DataPoint>& getData() const;`
  Returns loaded data (reference).

* `const std::string& getSymbol() const;`
  Returns dataset symbol.

* `const std::string& getTimeframe() const;`
  Returns dataset timeframe.

---

### 2. **MarketData**

**Header**: `MarketData.h`
**Purpose**: Manages multiple `DataLoader` instances organized by **symbol** and **timeframe**.

#### **Key Members**

| Member                                                                              | Description                                |
| ----------------------------------------------------------------------------------- | ------------------------------------------ |
| `std::map<std::string, std::map<std::string, std::unique_ptr<DataLoader>>> dataMap` | Nested map of loaders per symbol-timeframe |

#### **Key Methods**

* `void load(symbol, timeframe, filename, configFile);`
  Loads data into the nested map by creating a new `DataLoader`.

* `const DataLoader* getLoader(symbol, timeframe) const;`
  Retrieves loader for a specific **symbol-timeframe**.

---

### 3. **Parser**

**Header**: `Parser.h`, `Parser.cpp`
**Namespace**: `Parser`

#### **Purpose**

Parses string expressions into evaluable AST (Abstract Syntax Tree) structures, supporting:

✅ Constants
✅ Fields
✅ Functions (`SMA`, `EMA`, `log`, etc.)
✅ Unary and binary operations
✅ Logical and comparison operations
✅ Parentheses for precedence

#### **Key Classes and Functions**

##### **Tokenizer**

Splits input strings into tokens:

* Constants (`3.14`)
* Fields (`close`)
* Operators (`+`, `-`, `*`, `/`, `>`, `<`, etc.)
* Functions (`SMA`, `EMA`, etc.)
* Parentheses, commas

##### **ExpressionParser**

Converts token streams into expression trees using recursive descent parsing:

* `parseLogicalOr`, `parseLogicalAnd`
* `parseComparison`
* `parseArithmetic`, `parseTerm`, `parseUnary`, `parsePrimary`
* `parseFunction`

##### **parseExpression(input)**

Entry point function returning parsed `Expression` tree.

---

### 4. **Expression Hierarchy**

**Base Class**: `Expression`
Defines interface for evaluation against a `DataPoint` and history.

#### **Derived Classes**

| Class                   | Purpose                           |
| ----------------------- | --------------------------------- |
| `FieldExpr`             | Evaluates to specific field value |
| `ConstantExpr`          | Numeric constants                 |
| `UnaryExpression`       | Unary ops (e.g. negation)         |
| `BinaryExpression`      | Math, comparison, logical ops     |
| `FunctionExpression`    | Functions (`SMA`, `EMA`, etc.)    |
| `ConditionalExpression` | Ternary-like conditionals         |

---

### 5. **Strategy**

**Header**: `Strategy.h`
**Purpose**: Represents a trading strategy.

#### **Key Members**

| Member                                       | Description         |
| -------------------------------------------- | ------------------- |
| `std::unique_ptr<Expression> entryCondition` | Entry condition AST |
| `std::unique_ptr<Expression> exitCondition`  | Exit condition AST  |

#### **Key Methods**

* `const Expression& getEntryCondition() const;`
* `const Expression& getExitCondition() const;`

---

## 📊 **Available Data Fields**

| Field    | Description    |
| -------- | -------------- |
| `open`   | Opening price  |
| `close`  | Closing price  |
| `high`   | Highest price  |
| `low`    | Lowest price   |
| `volume` | Trading volume |

---

## ➕ **Operators**

### 🔢 **Arithmetic**

| Operator    | Description    | Example        |
| ----------- | -------------- | -------------- |
| `+`         | Addition       | `close + open` |
| `-`         | Subtraction    | `close - open` |
| `*`         | Multiplication | `close * 2`    |
| `/`         | Division       | `close / open` |
| `-` (unary) | Negation       | `-close`       |

---

### ⚖️ **Comparison**

| Operator | Description      | Example         | Returns                   |
| -------- | ---------------- | --------------- | ------------------------- |
| `>`      | Greater than     | `close > open`  | 1.0 if true, 0.0 if false |
| `<`      | Less than        | `close < open`  | 1.0 if true, 0.0 if false |
| `>=`     | Greater or equal | `close >= high` | 1.0 if true, 0.0 if false |
| `<=`     | Less or equal    | `close <= low`  | 1.0 if true, 0.0 if false |
| `==`     | Equal            | `close == open` | 1.0 if true, 0.0 if false |
| `!=`     | Not equal        | `close != open` | 1.0 if true, 0.0 if false |

---

### 🧠 **Logical**

| Operator | Description | Example                         |            |                |   |               |
| -------- | ----------- | ------------------------------- | ---------- | -------------- | - | ------------- |
| `&&`     | Logical AND | `close > open && volume > 1000` |            |                |   |               |
| \`       |             | \`                              | Logical OR | \`close > high |   | close < low\` |

---

### 🔗 **Grouping**

| Operator | Description | Example              |
| -------- | ----------- | -------------------- |
| `()`     | Parentheses | `(close + open) / 2` |

---

## 📈 **Functions**

### 📉 **Moving Averages**

| Function | Syntax              | Example          |
| -------- | ------------------- | ---------------- |
| `SMA`    | `SMA(expr, period)` | `SMA(close, 20)` |
| `EMA`    | `EMA(expr, period)` | `EMA(close, 50)` |

---

### 🧮 **Mathematical**

| Function | Syntax       | Example           |
| -------- | ------------ | ----------------- |
| `log`    | `log(expr)`  | `log(close)`      |
| `exp`    | `exp(expr)`  | `exp(close/1000)` |
| `abs`    | `abs(expr)`  | `abs(close-open)` |
| `sqrt`   | `sqrt(expr)` | `sqrt(volume)`    |

---

## ⚙️ **Operator Precedence**

1. `()` Parentheses
2. `-` (unary negation)
3. `*`, `/`
4. `+`, `-`
5. Comparison operators
6. `&&` Logical AND
7. `||` Logical OR

---

## 📚 **Strategy Examples**

### 🔹 **Price Action**

* `"close > open"`
* `"volume > 100000"`

### 🔹 **Technical**

* `"SMA(close,20) > SMA(close,50)"`

### 🔹 **Complex**

* `"close > SMA(close,20) && volume > SMA(volume,10) * 1.5"`

### 🔹 **Mathematical**

* `"log(close) - log(open) > 0.01"`

---

## 💡 **Signal Interpretation**

* **Positive value** → Long position
* **Negative value** → Short position
* **Zero** → Neutral

Magnitude affects position size (capped at 100% capital).

---

## ❗ **Error Handling**

Errors thrown for:

* Invalid syntax or operators
* Unknown functions
* Missing parentheses
* Division by zero → returns 0.0
* Log of non-positive → returns 0.0

---

## ⚡ **Performance Notes**

* Efficient SMA/EMA computation
* Parser runs once; evaluated repeatedly
* Minimal memory overhead

---

## ⚠️ **Limitations**

1. SMA/EMA need sufficient historical data
2. Domain restrictions: log (>0), sqrt (≥0)
3. Double precision floating-point
4. Field names lowercase; functions case-insensitive

---

## 🚀 **Advanced Usage**

### 🔹 **Custom Indicators**

```cpp
(SMA(close,14)-SMA(close,28))/SMA(close,28)
```

### 🔹 **Risk Management**

```cpp
(close > SMA(close,20)) * (1 / abs(close - SMA(close,20)))
```

## ✨ **Design Highlights**

✅ Recursive descent parsing
✅ Function evaluation with history
✅ Data-driven multi-symbol support
✅ Clean class separation
✅ Extensible for new functions/fields

---
