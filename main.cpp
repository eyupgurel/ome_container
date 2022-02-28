//#include "test/churn.h"
#include <matching_engine.h>

#include <zmq_addon.hpp>
#include <zmqpp/zmqpp.hpp>
#include "rxcpp/rx.hpp"
#include <fmt/format.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace Rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}
using namespace Rx;
using namespace nlohmann;

namespace po = boost::program_options;


inline void to_json(json &j, const order &o) {
    j = json{
            {"price",      o.price},
            {"epochMilli", o.epochMilli},
            {"quantity",   o.quantity},
            {"id",         o.id},
            {"ot",         o.ot},
            {"cud",        o.cud},
    };
}

inline void from_json(const json &j, order &o) {
    o.price = j.at("price").get<double>();
    o.epochMilli = j.at("epochMilli").get<long>();
    o.quantity = j.at("quantity").get<double>();
    o.id = j.at("id").get<unsigned long>();
    o.ot = j.at("ot").get<int>();
    o.cud = j.at("cud").get<int>();
}

inline void to_json(json &j, const match &m) {
    j = json{
            {"requestingOrderId", m.requestingOrderId},
            {"respondingOrderId", m.respondingOrderId},
            {"matchAmount",       m.matchAmount}
    };
}

inline void from_json(const json &j, match &m) {
    m.requestingOrderId = j.at("requestingOrderId").get<unsigned long>();
    m.respondingOrderId = j.at("respondingOrderId").get<unsigned long>();
    m.matchAmount = j.at("matchAmount").get<double>();
}

inline void to_json(json &j, const engine_state &es) {
    j = json{
            {"ask_orders", es.ask_orders},
            {"bid_orders", es.bid_orders},
            {"asks",       es.asks},
            {"bids",       es.bids},
            {"matches",    es.matches}
    };
}

inline void from_json(const json &j, engine_state &es) {
    auto ask_orders = j.at("ask_orders").get<json::array_t>();
    for (auto ask_order: ask_orders) {
        order ord(ask_order["price"], ask_order["epochMilli"], ask_order["quantity"], ask_order["id"], ask_order["ot"],
                  ask_order["cud"]);
        es.ask_orders.emplace_back(ord);
    }
    auto bid_orders = j.at("bid_orders").get<json::array_t>();
    for (auto bid_order: bid_orders) {
        order ord(bid_order["price"], bid_order["epochMilli"], bid_order["quantity"], bid_order["id"], bid_order["ot"],
                  bid_order["cud"]);
        es.bid_orders.emplace_back(ord);
    }

    auto asks = j.at("asks").get<json::array_t>();
    for (auto ask: asks) {
        es.asks.push_back({ask[0], ask[1]});
    }
    auto bids = j.at("bids").get<json::array_t>();
    for (auto bid: bids) {
        es.bids.push_back({bids[0], bids[1]});
    }
    auto matches = j.at("matches").get<json::array_t>();
    for (auto j_match: matches) {
        match match(j_match["requestingOrderId"], j_match["respondingOrderId"], j_match["matchAmount"]);
        es.matches.emplace_back(match);
    }
}




inline void erase_last(TOrders &orders) {
    auto ordersEnd = orders.get<PriceTimeIdx>().cend();
    auto _last = std::next(ordersEnd, -1);
    orders.get<PriceTimeIdx>().erase(_last, ordersEnd);
    //print_orders(orders);
}

inline void erase_first(TOrders &orders) {
    auto ordersBeg = orders.get<PriceTimeIdx>().begin();
    auto next = std::next(ordersBeg, 1);
    orders.get<PriceTimeIdx>().erase(ordersBeg, next);
    //print_orders(orders);
}

inline void write_sell_orders(TOrders &sell_orders, std::vector<order> &v_sell_orders) {
    auto ordersBeg = sell_orders.get<PriceTimeIdx>().cbegin();
    auto ordersEnd = sell_orders.get<PriceTimeIdx>().cend();
    auto counter = 0;
    for (auto it = ordersBeg; it != ordersEnd && counter < 20; ++it) {
        v_sell_orders.emplace_back(*it);
        counter++;
    }

}

inline void write_buy_orders(TOrders &buy_orders, std::vector<order> &v_buy_orders) {
    auto ordersRBeg = buy_orders.get<PriceTimeIdx>().crbegin();
    auto ordersREnd = buy_orders.get<PriceTimeIdx>().crend();
    auto counter = 0;
    for (auto it = ordersRBeg; it != ordersREnd && counter < 20; ++it) {
        v_buy_orders.emplace_back(*it);
        counter++;
    }
}

inline void write_sell_order_book(TOrders &sell_orders, std::vector<std::array<double, 2>> &order_book) {
    auto ordersBeg = sell_orders.get<PriceTimeIdx>().cbegin();
    auto ordersEnd = sell_orders.get<PriceTimeIdx>().cend();
    auto counter = 0;
    for (auto it = ordersBeg; it != ordersEnd && counter < 20; ++it) {
        if (order_book.empty() || order_book[order_book.size() - 1][0] != it->price) {
            order_book.push_back({it->price, it->quantity});
            ++counter;
        } else {
            order_book[order_book.size() - 1][1] += it->quantity;
        }
    }
}

inline void write_buy_order_book(TOrders &buy_orders, std::vector<std::array<double, 2>> &order_book) {
    auto ordersRBeg = buy_orders.get<PriceTimeIdx>().crbegin();
    auto ordersREnd = buy_orders.get<PriceTimeIdx>().crend();
    auto counter = 0;
    for (auto it = ordersRBeg; it != ordersREnd && counter < 20; ++it) {
        if (order_book.empty() || order_book[order_book.size() - 1][0] != it->price) {
            order_book.push_back({it->price, it->quantity});
            ++counter;
        } else {
            order_book[order_book.size() - 1][1] += it->quantity;
        }
    }
}



#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main(int argc, char *argv[]) {

    std::string zmq_sock_uri;
    int zmq_resp_sock_port;
    int zmq_pub_sock_port;
    int max_order_book_size;
    int trimmed_order_book_size;

    try {
      po::options_description desc("Allowed options");
        desc.add_options()
                ("help", "produce help message")
                ("zmq-sock-uri", po::value<std::string>(&zmq_sock_uri), "ZeroMQ response socket uri")
                ("zmq-resp-sock-port", po::value<int>(&zmq_resp_sock_port), "ZeroMQ response socket port")
                ("zmq-pub-sock-port", po::value<int>(&zmq_pub_sock_port), "ZeroMQ publish socket port")
                ("max-order-book-size", po::value<int>(&max_order_book_size), "Maximum order book size bid or ask")
                ("trimmed-order-book-size", po::value<int>(&trimmed_order_book_size), "Trimmed order book size bid or ask")
                ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 0;
        }

        if (vm.count("zmq-sock-uri")) {
            cout << "ZeroMQ socket endpoint is set to "
                 << zmq_sock_uri << ".\n";
        } else {
            cout << "ZeroMQ socket endpoint was not set.\n";
        }

        if (vm.count("zmq-resp-sock-port")) {
            cout << "ZeroMQ response socket port is set to "
                 << zmq_resp_sock_port << ".\n";
        } else {
            cout << "ZeroMQ response socket port was not set.\n";
        }

        if (vm.count("zmq-pub-sock-port")) {
            cout << "ZeroMQ publish socket port is set to "
                 << zmq_pub_sock_port << ".\n";
        } else {
            cout << "ZeroMQ publish socket port was not set.\n";
        }

        if (vm.count("max-order-book-size")) {
            cout << "Max order book size is set to "
                 << max_order_book_size << ".\n";
        } else {
            cout << "Max order book size was not set.\n";
        }

        if (vm.count("trimmed-order-book-size")) {
            cout << "Trimmed order book size is set to "
                 << trimmed_order_book_size << ".\n";
        } else {
            cout << "Trimmed order book size was not set.\n";
        }
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
        return 1;
    }

    TOrders asks;
    TOrders bids;
    std::vector<match> matches;

    static zmq::context_t resp_context;
    zmq::socket_t resp_socket(resp_context, zmq::socket_type::rep);
    resp_socket.bind(fmt::format("{}:{}!", zmq_sock_uri,zmq_resp_sock_port));

    // Create a publisher publisher_socket
    zmqpp::context publisher_context;
    zmqpp::socket_type type = zmqpp::socket_type::publish;
    zmqpp::socket publisher_socket(publisher_context, type);
    publisher_socket.bind(fmt::format("{}:{}!", zmq_sock_uri,zmq_pub_sock_port));

    while (true) {
        try {

            zmq::message_t z_in;
            auto res = resp_socket.recv(z_in);

            auto jmsg_in = json::parse(z_in.to_string_view());
            for (auto o: jmsg_in) {
                order ord(o["price"], o["epochMilli"], o["quantity"], o["id"], o["ot"], o["cud"]);
                match_order(ord, bids, asks, matches);
            }

            std::vector<order> v_sell_orders;
            write_sell_orders(asks, v_sell_orders);
            std::vector<order> v_buy_orders;
            write_buy_orders(bids, v_buy_orders);

            std::vector<std::array<double, 2>> booked_asks;
            write_sell_order_book(asks, booked_asks);

            std::vector<std::array<double, 2>> booked_bids;
            write_buy_order_book(bids, booked_bids);

            engine_state es(v_sell_orders, v_buy_orders, booked_asks, booked_bids, matches);

            json jmsg_out_engine_state(es);
            zmq::message_t z_out_engine_state(jmsg_out_engine_state.dump());
            resp_socket.send(z_out_engine_state, zmq::send_flags::none);

            // Create a pub pub_message and feed data into it
            zmqpp::message pub_message;
            pub_message << jmsg_out_engine_state.dump();
            publisher_socket.send(pub_message);

            // do trimming and garbage collection
            if (asks.size() > max_order_book_size) {
                for (auto i = 0; i < trimmed_order_book_size; ++i) {
                    erase_last(asks);
                }
            }
            if (bids.size() > max_order_book_size) {
                for (auto i = 0; i < trimmed_order_book_size; ++i) {
                    erase_first(bids);
                }
            }
            matches.clear();
        }
        catch (json::parse_error &ex) {
            std::cerr << "parse error at byte " << ex.byte << std::endl;
        }
        catch (const std::exception& ex) {
            std::cerr << ex.what() << std::endl;
        }
        catch (...) {
            std::cerr << "unknown exception occured" << std::endl;
        }
    }



/*    rxcpp::observable<>::range(1, 2000).
    subscribe_on(rxcpp::observe_on_new_thread()).
            map([](int v) {
                return std::make_tuple(get_pid(), v);}).
            as_blocking().
                subscribe(
            rxcpp::util::apply_to(
                    [](const std::string& pid, int v) {
                        printf("[thread %s] OnNext: %d\n", pid.c_str(), v);
                        churn3();
                    }),
            [](){printf("[thread %s] OnCompleted\n", get_pid().c_str());});*/


    return 0;
}

#pragma clang diagnostic pop


