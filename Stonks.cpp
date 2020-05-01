// Project identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include <getopt.h>
#include <algorithm>
#include <deque>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "P2random.h"

using namespace std;

struct TraderInfo {
  int stocksBought = 0;
  int stocksSold = 0;
  int netTransfer = 0;
};

struct Order {
  int timestamp;
  int traderID;
  int stockID;
  // true for buy, false for sell
  bool buyOrSell;
  int price;
  int quantity;
  int uniqNum;
};

enum State {
  initialState,
  // if you spotted at least one selling stock
  sellOrderSpotted,
  // if you spotted a buying order that will result in a profit
  profitableTrade,
  // if you see another seller selling for a cheaper price then the original
  // trade but you are not sure if it will end up resulting in more profit than
  // the current trade you are already considering
  potentionalNewTrade
};

// struct that looks at all orders and decides which are profitable to buy
class ProfitableTradeTracker {
 private:
  State s = initialState;
  Order stockToBuy;
  Order buyerToSellTo;
  Order potentialStockToBuy;

 public:
  ProfitableTradeTracker() {
    Order placeHolder;
    placeHolder.timestamp = -1;
    placeHolder.traderID = -1;
    placeHolder.stockID = -1;
    placeHolder.buyOrSell = false;
    placeHolder.price = -1;
    placeHolder.quantity = -1;
    stockToBuy = placeHolder;
    buyerToSellTo = placeHolder;
    potentialStockToBuy = placeHolder;
  }

  void handleOrder(Order &o) {
    // if the order is selling
    if (!o.buyOrSell) {
      // if no sellings have been spotted yet
      if (s == initialState) {
        s = sellOrderSpotted;
        stockToBuy = o;
      }
      // if you already have a seller for the stock but you spot a seller
      // selling for less
      else if (s == sellOrderSpotted && o.price < stockToBuy.price) {
        stockToBuy = o;
      }
      // if you already have a completed trade lined up but you spot a cheaper
      // price afterwards
      else if (s == profitableTrade && o.price < stockToBuy.price) {
        potentialStockToBuy = o;
        s = potentionalNewTrade;
      }
      // if you have a new potential trade and you see a seller selling for less
      else if ((s == potentionalNewTrade) &&
               (o.price < potentialStockToBuy.price)) {
        potentialStockToBuy = o;
      }
    }
    // if the order is buying
    else {
      // if you have a stock to sell and you find a buyer that will buy for more
      // than you bought for
      if (s == sellOrderSpotted && o.price > stockToBuy.price) {
        s = profitableTrade;
        buyerToSellTo = o;
      }
      // if you already have a trade lined up but you find a buyer willing to
      // buy for more
      else if (s == profitableTrade && o.price > buyerToSellTo.price) {
        s = profitableTrade;
        buyerToSellTo = o;
      }
      // if you have a potential new trade that will result in more profit than
      // your previously set up trade
      else if ((s == potentionalNewTrade) &&
               ((o.price - potentialStockToBuy.price) >
                (buyerToSellTo.price - stockToBuy.price))) {
        s = profitableTrade;
        stockToBuy = potentialStockToBuy;
        buyerToSellTo = o;
      }
    }
  }

  // prints a summary for one trade tracker
  void printSummary(int index) {
    if (s == initialState || s == sellOrderSpotted) {
      cout << "A time traveler could not make a profit on shares of Stock "
           << index << '\n';
    } else {
      cout << "A time traveler would buy shares of Stock " << index
           << " at time " << stockToBuy.timestamp << " for $"
           << stockToBuy.price << " and sell these shares at time "
           << buyerToSellTo.timestamp << " for $" << buyerToSellTo.price
           << '\n';
    }
  }
};

// comparator that returns the highest buyer
struct MaxComparator {
  bool operator()(Order lhs, Order rhs) {
    if (lhs.price > rhs.price) {
      return true;
    } else if (lhs.price < rhs.price) {
      return false;
    }
    // if buying prices are equal compare timestamps
    else {
      if (lhs.timestamp > rhs.timestamp) {
        return true;
      } else if (lhs.timestamp < rhs.timestamp) {
        return false;
      }
      // if timestamps are equal compare uniqNums
      else {
        if (lhs.uniqNum > rhs.uniqNum) {
          return true;
        } else {
          return false;
        }
      }
    }
  }
};

// comparator that returns true if lhs < rhs
struct MinComparator {
  bool operator()(Order lhs, Order rhs) {
    if (lhs.price < rhs.price) {
      return true;
    } else if (lhs.price > rhs.price) {
      return false;
    }
    // if buying prices are equal compare timestamps
    else {
      if (lhs.timestamp > rhs.timestamp) {
        return true;
      } else if (lhs.timestamp < rhs.timestamp) {
        return false;
      }
      // if timestamps are equal compare uniqNums
      else {
        if (lhs.uniqNum > rhs.uniqNum) {
          return true;
        } else {
          return false;
        }
      }
    }
  }
};

// an object for each unique stock that the program comes across
// two priority queues per stock type
class Stock {
 public:
  // vector that holds how much each stock has sold for - used to calculate
  // median
  // vector<int> salePrices;
  priority_queue<int> salePricesSmall;
  priority_queue<int, std::vector<int>, std::greater<int>> salePricesBig;
  int salesCounter = 0;
  priority_queue<Order, std::vector<Order>, MinComparator> buying;
  priority_queue<Order, std::vector<Order>, MaxComparator> selling;

  void handleSalePrice(int sale) {
    salesCounter++;

    // if the sale is bigger than or equal to the biggest element in the small
    // queue,
    /*  if ((!salePricesSmall.empty()) && sale >= salePricesSmall.top()){
        int sizeDiff = (int)salePricesBig.size() - (int)salePricesSmall.size();
        //if the sides are already balanced
        if(sizeDiff == 0 || sizeDiff == -1){
          salePricesBig.push()
        }
      }
      //if they are both empty
      else if(salePricesSmall.empty() && salePricesBig.empty()){
        salePricesSmall.push(sale);
      }*/

    // if the sale is bigger than or equal to the last element in the smaller
    // half
    if ((!salePricesSmall.empty()) && sale >= salePricesSmall.top()) {
      int sizeDiff = (int)salePricesBig.size() - (int)salePricesSmall.size();
      // if the two sides are already balanced just push sale to the appropriate
      // side and bring the smallest element to the front
      if (sizeDiff == 0 || sizeDiff == -1) {
        salePricesBig.push(sale);
      }
      // if the big vector already had an extra sale you have to put one in the
      // small vector to keep it even, then find the smallest element in the big
      // vector and bring it to the front
      else if (sizeDiff == 1) {
        salePricesBig.push(sale);
        salePricesSmall.push(salePricesBig.top());
        salePricesBig.pop();
      }
    }
    // if the sale is smaller or equal to the first element in the
    else if ((!salePricesBig.empty()) && sale <= salePricesBig.top()) {
      int sizeDiff = (int)salePricesSmall.size() - (int)salePricesBig.size();
      // if the two are already balanced just push the sale to the appropriate
      // side and bring the biggest element to the back
      if (sizeDiff == 0 || sizeDiff == -1) {
        salePricesSmall.push(sale);
      } else if (sizeDiff == 1) {
        salePricesSmall.push(sale);
        salePricesBig.push(salePricesSmall.top());
        salePricesSmall.pop();
      }
    }
    // if one of them are empty
    else if (salePricesSmall.empty() && (!salePricesBig.empty())) {
      if (sale > salePricesBig.top()) {
        salePricesSmall.push(salePricesBig.top());
        salePricesBig.pop();
        salePricesBig.push(sale);
      } else {
        salePricesSmall.push(sale);
      }
    } else if ((!salePricesSmall.empty()) && salePricesBig.empty()) {
      if (sale >= salePricesSmall.top()) {
        salePricesBig.push(sale);
      } else {
        salePricesBig.push(salePricesSmall.top());
        salePricesSmall.pop();
        salePricesSmall.push(sale);
      }
    }  // else if ((salePricesSmall.empty()) && salePricesBig.empty()) {
    else {
      salePricesSmall.push(sale);
    }
  }

  int calculateMedian() {
    if (salePricesSmall.size() == salePricesBig.size()) {
      return ((salePricesBig.top() + salePricesSmall.top()) / 2);
    } else if (salePricesSmall.size() > salePricesBig.size()) {
      return salePricesSmall.top();
    } else if (salePricesSmall.size() < salePricesBig.size()) {
      return salePricesBig.top();
    }
    return 0;
  }
};

// class that contains this entire program
class Market {
 private:
  // true for TL, false for PR
  bool mode;
  int numberOfTraders;
  int numberOfStocks;
  int currentTimestamp;
  int currentUniqNum;

  // for PR
  int randomSeed = 0;
  int numberOfOrders = 0;
  int arrivalRate = 0;
  int numberOfTransactions = 0;

  // flags
  // TODO set back to false
  bool verbose = false;
  bool median = false;
  bool traderInfo = false;
  bool timeTravelers = false;

  vector<Stock> stocks;
  vector<TraderInfo> traders;
  vector<ProfitableTradeTracker> tradeTrackers;

  // vector<Transaction> transactions;

 public:
  // constructor
  Market() {
    // deafult info
    mode = false;
    numberOfTraders = 0;
    numberOfStocks = 0;
    currentTimestamp = 0;
    currentUniqNum = 0;
  }

  void read() {
    string trash;
    // stores comment line in trash
    getline(cin, trash);

    cin >> trash >> trash;

    if (trash == "TL") {
      mode = true;
    } else {
      mode = false;
    }

    getline(cin, trash);

    cin >> trash >> numberOfTraders;

    getline(cin, trash);

    cin >> trash >> numberOfStocks;

    getline(cin, trash);

    // initialize traders vector with right size and default info
    if (traderInfo) {
      traders.resize(numberOfTraders);
      TraderInfo ti;
      std::fill(traders.begin(), traders.end(), ti);
    }

    if (timeTravelers) {
      tradeTrackers.resize(numberOfStocks);
      ProfitableTradeTracker p = ProfitableTradeTracker();
      std::fill(tradeTrackers.begin(), tradeTrackers.end(), p);
    }

    // set up stocks vector with right size and gives default data
    stocks.resize(numberOfStocks);
    Stock s;
    std::fill(stocks.begin(), stocks.end(), s);

    if (mode) {
      readOrders(cin);
    }
    // read the rest of the lines if it
    else {
      stringstream s;

      cin >> trash >> randomSeed;
      getline(cin, trash);

      cin >> trash >> numberOfOrders;
      getline(cin, trash);

      cin >> trash >> arrivalRate;
      getline(cin, trash);

      P2random::PR_init(s, randomSeed, numberOfTraders, numberOfStocks,
                        numberOfOrders, arrivalRate);

      readOrders(s);
    }
    cout << "---End of Day---" << '\n';
    cout << "Orders Processed: " << numberOfTransactions << '\n';

    if (traderInfo) {
      traderInfoSummary();
    }
    if (timeTravelers) {
      cout << "---Time Travelers---" << '\n';
      for (int i = 0; i < numberOfStocks; i++) {
        tradeTrackers[i].printSummary(i);
      }
    }
  }

  // will read the orders
  void readOrders(istream &in) {
    string stringTrash;
    char charTrash;
    int input;
    // for (int i = 0; i < numberOfOrders; i++) {
    // while (in) {

    cout << "Processing orders..." << '\n';

    while (in >> input) {
      Order o;
      // in >> input;
      o.timestamp = input;
      if (o.timestamp < 0 || o.timestamp < currentTimestamp) {
        string err = "Timestamp is invalid";
        throw err;
      }
      in >> stringTrash;
      if (stringTrash == "SELL") {
        o.buyOrSell = false;
      } else {
        o.buyOrSell = true;
      }
      in >> charTrash;
      in >> input;
      o.traderID = input;
      if (o.traderID < 0 || o.traderID >= numberOfTraders) {
        string err = "TraderID is invalid";
        throw err;
      }
      in >> charTrash;
      in >> input;
      o.stockID = input;
      if (o.stockID < 0 || o.stockID >= numberOfStocks) {
        string err = "StockID is invalid";
        throw err;
      }
      in >> charTrash;
      in >> input;
      o.price = input;
      if (o.price <= 0) {
        string err = "Price is not positive";
        throw err;
      }
      in >> charTrash;
      in >> input;
      o.quantity = input;
      if (o.quantity <= 0) {
        string err = "Quantity is not positive";
        throw err;
      }

      o.uniqNum = currentUniqNum;
      currentUniqNum++;

      // if you are going into a new timestamp
      // TODO - here is where you should print median info if needed
      if (currentTimestamp != o.timestamp) {
        if (median) {
          for (int i = 0; i < (int)stocks.size(); i++) {
            if (!(stocks[i].salePricesSmall.empty() &&
                  stocks[i].salePricesBig.empty())) {
              cout << "Median match price of Stock " << i << " at time "
                   << currentTimestamp << " is $" << stocks[i].calculateMedian()
                   << '\n';
            }
          }
        }
        currentTimestamp = o.timestamp;
      }
      handleOrder(o);

      // if timeTravelers is activated
      if (timeTravelers) {
        tradeTrackers[o.stockID].handleOrder(o);
      }
    }
    // calculate median one last time
    if (median) {
      for (int i = 0; i < (int)stocks.size(); i++) {
        if (!(stocks[i].salePricesSmall.empty() &&
              stocks[i].salePricesBig.empty())) {
          cout << "Median match price of Stock " << i << " at time "
               << currentTimestamp << " is $" << stocks[i].calculateMedian()
               << '\n';
        }
      }
    }
  }

  void handleOrder(Order &o) {
    // if buying and there is at least one element to pop
    if (o.buyOrSell && !stocks[o.stockID].selling.empty()) {
      Order seller = stocks[o.stockID].selling.top();
      // if the buying price is higher than the selling price,
      // a transaction will happen at the price of the seller
      // buyer will buy as many as they can / are willing to
      // if the buyer want to buy more than provided,
      // make a new order and push to buying PQ
      // if the seller has extra stocks after transaction,
      // update the amount for seller
      if (o.price >= seller.price) {
        // if buyer want to buy exactly how many stocks are avialable
        if (o.quantity == seller.quantity) {
          if (traderInfo) {
            traders[seller.traderID].stocksSold += seller.quantity;
            traders[seller.traderID].netTransfer +=
                seller.quantity * seller.price;
            traders[o.traderID].stocksBought += o.quantity;
            traders[o.traderID].netTransfer -= o.quantity * seller.price;
          }
          stocks[o.stockID].selling.pop();
          stocks[o.stockID].handleSalePrice(seller.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << o.traderID << " purchased " << o.quantity
                 << " shares of Stock " << o.stockID << " from Trader "
                 << seller.traderID << " for $" << seller.price << "/share"
                 << '\n';
          }
        }
        // if the buyer wants to buy more than are available
        else if (o.quantity > seller.quantity) {
          if (traderInfo) {
            traders[seller.traderID].stocksSold += seller.quantity;
            traders[seller.traderID].netTransfer +=
                seller.quantity * seller.price;
            traders[o.traderID].stocksBought += seller.quantity;
            traders[o.traderID].netTransfer -= seller.quantity * seller.price;
          }
          stocks[o.stockID].selling.pop();
          o.quantity -= seller.quantity;
          stocks[o.stockID].handleSalePrice(seller.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << o.traderID << " purchased " << seller.quantity
                 << " shares of Stock " << o.stockID << " from Trader "
                 << seller.traderID << " for $" << seller.price << "/share"
                 << '\n';
          }
          handleOrder(o);
        }
        // if the buyer doesnt buy all the available stocks
        else {
          if (traderInfo) {
            traders[seller.traderID].stocksSold += o.quantity;
            traders[seller.traderID].netTransfer += o.quantity * seller.price;
            traders[o.traderID].stocksBought += o.quantity;
            traders[o.traderID].netTransfer -= o.quantity * seller.price;
          }
          stocks[o.stockID].selling.pop();
          seller.quantity -= o.quantity;
          // stocks[o.stockID].selling.push(seller);
          stocks[o.stockID].handleSalePrice(seller.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << o.traderID << " purchased " << o.quantity
                 << " shares of Stock " << o.stockID << " from Trader "
                 << seller.traderID << " for $" << seller.price << "/share"
                 << '\n';
          }
          handleOrder(seller);
        }
      }
      // if the buying price is less than the selling price,
      // push the order into the buying PQ
      else {
        stocks[o.stockID].buying.push(o);
      }
    }
    // if selling and there is at least one element to pop
    else if (!o.buyOrSell && !stocks[o.stockID].buying.empty()) {
      Order buyer = stocks[o.stockID].buying.top();
      // if current order is selling for a price lower than the buyer,
      // a transaction will happen at the price of the buyer
      // seller will sell as many as they can / are willing to
      // if the seller has more to sell after the transaction,
      // make a new order and push to selling PQ
      // if the buyer wants to buy more than the seller has,
      // update amount for buyer
      if (o.price <= buyer.price) {
        // if seller is selling exact amount that the buyer wants
        if (o.quantity == buyer.quantity) {
          if (traderInfo) {
            traders[buyer.traderID].stocksBought += buyer.quantity;
            traders[buyer.traderID].netTransfer -= buyer.quantity * buyer.price;
            traders[o.traderID].stocksSold += o.quantity;
            traders[o.traderID].netTransfer += o.quantity * buyer.price;
          }
          stocks[o.stockID].buying.pop();
          stocks[o.stockID].handleSalePrice(buyer.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << buyer.traderID << " purchased "
                 << buyer.quantity << " shares of Stock " << o.stockID
                 << " from Trader " << o.traderID << " for $" << buyer.price
                 << "/share" << '\n';
          }
        }
        // if the seller wants to sell more than the buyer is willing to buy
        else if (o.quantity > buyer.quantity) {
          if (traderInfo) {
            traders[buyer.traderID].stocksBought += buyer.quantity;
            traders[buyer.traderID].netTransfer -= buyer.quantity * buyer.price;
            traders[o.traderID].stocksSold += buyer.quantity;
            traders[o.traderID].netTransfer += buyer.quantity * buyer.price;
          }
          stocks[o.stockID].buying.pop();
          o.quantity -= buyer.quantity;
          // since the buyer still has stocks he wants to buy, make a new order
          // and push to buyingPQ for that stockID
          // stocks[o.stockID].selling.push(o);
          stocks[o.stockID].handleSalePrice(buyer.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << buyer.traderID << " purchased "
                 << buyer.quantity << " shares of Stock " << o.stockID
                 << " from Trader " << o.traderID << " for $" << buyer.price
                 << "/share" << '\n';
          }
          handleOrder(o);
        }
        // if the buyer wants to buy more stocks than are available
        else {
          if (traderInfo) {
            traders[buyer.traderID].stocksBought += o.quantity;
            traders[buyer.traderID].netTransfer -= o.quantity * buyer.price;
            traders[o.traderID].stocksSold += o.quantity;
            traders[o.traderID].netTransfer += o.quantity * buyer.price;
          }
          stocks[o.stockID].buying.pop();
          buyer.quantity -= o.quantity;
          // stocks[o.stockID].buying.push(buyer);
          stocks[o.stockID].handleSalePrice(buyer.price);
          numberOfTransactions++;
          if (verbose) {
            cout << "Trader " << buyer.traderID << " purchased " << o.quantity
                 << " shares of Stock " << o.stockID << " from Trader "
                 << o.traderID << " for $" << buyer.price << "/share" << '\n';
          }
          handleOrder(buyer);
        }
      }
      // if current order is selling for more than the buyer,
      // no transaction can be made so push order to selling
      else {
        stocks[o.stockID].selling.push(o);
      }
    }
    // if this is an unseen stock, or a needed vector is empty, modify the
    // current "Stock" at the index of the unseen stock
    else {
      // if buying
      if (o.buyOrSell) {
        stocks[o.stockID].buying.push(o);
      }
      // if selling
      else {
        stocks[o.stockID].selling.push(o);
      }
    }
  }

  void traderInfoSummary() {
    cout << "---Trader Info---" << '\n';
    for (int i = 0; i < numberOfTraders; i++) {
      cout << "Trader " << i << " bought " << traders[i].stocksBought
           << " and sold " << traders[i].stocksSold
           << " for a net transfer of $" << traders[i].netTransfer << '\n';
    }
  }

  void timeTravelerInfo() {}

  void get_options(int argc, char **argv) {
    int option_index = 0, option = 0;

    // display getopt error messages about options
    opterr = true;

    // use getopt to find command line options
    struct option longOpts[] = {{"verbose", no_argument, nullptr, 'v'},
                                {"median", no_argument, nullptr, 'm'},
                                {"trader_info", no_argument, nullptr, 'i'},
                                {"time_travelers", no_argument, nullptr, 't'},
                                {nullptr, 0, nullptr, '\0'}};

    while ((option = getopt_long(argc, argv, "vmit", longOpts,
                                 &option_index)) != -1) {
      switch (option) {
        case 'v':
          verbose = true;
          break;

        case 'm':
          median = true;
          break;

        case 'i':
          traderInfo = true;
          break;

        case 't':
          timeTravelers = true;
          break;

        default:
          string error = "Invalid flag";
          throw error;
      }
    }
  }
};

int main(int argc, char *argv[]) {
  ios::sync_with_stdio(false);
  Market m;

  try {
    m.get_options(argc, argv);
    m.read();
  } catch (string error) {
    cout << error << '\n';
    return 1;
  }

  return 0;
}