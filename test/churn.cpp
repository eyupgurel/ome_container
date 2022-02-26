/*
//
// Created by egl on 13.01.2022.
//

#include "churn.h"
#include <data_stream.h>

[[noreturn]] void churn(){
    const size_t order_count = 5;
    const size_t market_order_count = order_count / 5;

    std::vector<order> v_bids;
    prepareOrderVector(order_count, 0, 0, 3.02, 3.29, 12.01, 1242.02, v_bids);
    cout << "bids:" << endl;
    std::copy(v_bids.cbegin(), v_bids.cend(), std::ostream_iterator<order>(std::cout << "\n"));

    std::vector<order> v_asks;
    prepareOrderVector(order_count, 1, 0, 3.33, 3.48, 11.45, 1242.02, v_asks);

    cout << "asks:" << endl;
    std::copy(v_asks.cbegin(), v_asks.cend(), std::ostream_iterator<order>(std::cout << "\n"));

    TOrders bids;
    TOrders asks;

    std::vector<match> matches;

    match_order(v_asks, bids, asks, matches);
    match_order(v_bids, bids, asks, matches);

    print_orders(bids);
    print_orders(asks);

    std::vector<order> v_buy_market_orders;
    prepareOrderVector(market_order_count, 0, 0, DBL_MAX, 12.01, 1242.02, v_buy_market_orders);

    std::vector<order> v_sell_market_orders;
    prepareOrderVector(market_order_count, 1, 0, 0.0, 11.45, 1242.02, v_sell_market_orders);

    match_order(v_buy_market_orders, bids, asks, matches);
    match_order(v_sell_market_orders, bids, asks, matches);

    print_orders(bids);
    print_orders(asks);

    while(true){
        vector<order> v_deleted_bids;

        for(auto order: bids.get<PriceTimeIdx>()){
            if(v_deleted_bids.empty()){
                v_deleted_bids.emplace_back(order.price, order.epochMilli, order.quantity, order.id, order.ot, 2);
                break;
            }
        }

        vector<order> v_deleted_asks;

        for(auto order: asks.get<PriceTimeIdx>()){
            if(v_deleted_asks.empty()){
                v_deleted_asks.emplace_back(order.price, order.epochMilli, order.quantity, order.id, order.ot, 2);
                break;
            }
        }

        match_order(v_deleted_bids, bids, asks, matches);
        match_order(v_deleted_asks, bids, asks, matches);

        print_orders(bids);
        print_orders(asks);


        size_t missing_bid_count = order_count - bids.size();
        std::vector<order> v_missing_bids;
        if (missing_bid_count > 0){
            prepareOrderVector(missing_bid_count, 0, 0, 3.02, 3.29, 12.01, 1242.02, v_missing_bids);
            cout << "missing bids:" << endl;
            std::copy(v_missing_bids.cbegin(), v_missing_bids.cend(), std::ostream_iterator<order>(std::cout << "\n"));
        }

        size_t missing_ask_count = order_count - asks.size();
        std::vector<order> v_missing_asks;
        if (missing_ask_count > 0) {
            prepareOrderVector(missing_ask_count, 1, 0, 3.33, 3.48, 11.45, 1242.02, v_missing_asks);
            cout << "missing asks:" << endl;
            std::copy(v_missing_asks.cbegin(), v_missing_asks.cend(), std::ostream_iterator<order>(std::cout << "\n"));
        }
        match_order(v_missing_bids, bids, asks, matches);
        match_order(v_missing_asks, bids, asks, matches);

        print_orders(bids);
        print_orders(asks);

        std::vector<order> v_buy_market_orders;
        prepareOrderVector(market_order_count, 0, 0, DBL_MAX, 12.01, 1242.02, v_buy_market_orders);

        std::vector<order> v_sell_market_orders;
        prepareOrderVector(market_order_count, 1, 0, 0.0, 11.45, 1242.02, v_sell_market_orders);

        match_order(v_buy_market_orders, bids, asks, matches);
        match_order(v_sell_market_orders, bids, asks, matches);

        print_orders(bids);
        print_orders(asks);

    }
}


void churn2(){

    const size_t order_count = 5;
    const int market_order_count = order_count / 5;

    TOrders asks;
    TOrders bids;

    std::vector<order> v_various_orders;
    prepareOrderVector(market_order_count,1,0,0.0,11.45, 1242.02,v_various_orders);
    prepareOrderVector(order_count, 0,0,3.02, 3.29,12.01, 1242.02, v_various_orders);
    prepareOrderVector(market_order_count, 0,0,DBL_MAX, 12.01, 1242.02,v_various_orders);
    prepareOrderVector(order_count,1,0,3.33, 3.48,11.45, 1242.02,v_various_orders);

    TOrders various_orders;

    various_orders.insert(begin(v_various_orders), end(v_various_orders));
    cout << "various_orders.size() " << various_orders.size() << endl;

    print_orders(various_orders);

    std::vector<match> matches;
    long epoch_milli_various_order_start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    for(auto ord : v_various_orders){
        match_order(ord, bids, asks, matches);
    }

    long epoch_milli_various_order_end = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    cout << "bids left " << bids.size() << endl;
    print_orders(bids);

    cout << "asks left " << asks.size() << endl;
    print_orders(asks);

    auto duration = epoch_milli_various_order_end - epoch_milli_various_order_start;

    cout << "time_elapsed_various_order_processed: " << duration << endl;

    cout << "match size " << matches.size() << endl;

    std::copy(matches.cbegin(), matches.cend(), std::ostream_iterator<match>(std::cout << "\n"));



    random_device rd;   // non-deterministic generator
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 2);
    uniform_real_distribution<double> reprice_dist(0.0, 2.0);
    uniform_real_distribution<double> price_dist_bid_diff(3.02, 3.29);
    uniform_real_distribution<double> price_dist_ask_diff(3.33, 3.48);
    uniform_real_distribution<double> quantity_dist(12.01, 1242.02);

    while(true) {
        //rearrange bids
        cout << "bids left " << bids.size() << endl;
        print_orders(bids);

        vector<order> v_bids_diff;
        for (const auto &bidsIt: bids.get<PriceTimeIdx>()) {
            auto cud = dist(gen);
            switch (cud) {
                case 0:
                    prepareOrderVector(1, 0, cud, 3.02, 3.29, 12.01, 1242.02, v_bids_diff);
                    break;
                case 1:
                    v_bids_diff.emplace_back(price_dist_bid_diff(gen),
                                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()).count(),
                                             quantity_dist(gen),
                                             bidsIt.id,
                                             0,
                                             cud);
                    break;
                case 2:
                    v_bids_diff.emplace_back(bidsIt.price,
                                             bidsIt.epochMilli,
                                             bidsIt.quantity,
                                             bidsIt.id,
                                             0,
                                             cud);
                    break;
                default:
                    throw std::invalid_argument("Invalid order type.");
            }
        }

        if(v_bids_diff.size() < order_count){
            prepareOrderVector(order_count - v_bids_diff.size(), 0, 0, 3.02, 3.29, 12.01, 1242.02, v_bids_diff);
        }

        print_orders(bids);
        std::copy(v_bids_diff.cbegin(), v_bids_diff.cend(), std::ostream_iterator<order>(std::cout << "\n"));

        //rearrange asks
        vector<order> v_asks_diff;
        for (const auto &asksIt: asks.get<PriceTimeIdx>()) {
            auto cud = dist(gen);
            switch (cud) {
                case 0:
                    prepareOrderVector(1, 1, cud, 3.33, 3.48, 11.45, 1242.02, v_asks_diff);
                    break;
                case 1:
                    v_asks_diff.emplace_back(price_dist_ask_diff(gen),
                                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()).count(),
                                             quantity_dist(gen),
                                             asksIt.id,
                                             1,
                                             cud);
                    break;
                case 2:
                    v_asks_diff.emplace_back(asksIt.price,
                                             asksIt.epochMilli,
                                             asksIt.quantity,
                                             asksIt.id,
                                             1,
                                             cud);
                    break;
                default:
                    throw std::invalid_argument("Invalid order type.");
            }
        }



*/
/*        while(v_asks_diff.size() < order_count){
            prepareOrderVector(1, 1, 0, 3.33, 3.48, 11.45, 1242.02, v_asks_diff);
        }*//*


        print_orders(asks);
        std::copy(v_asks_diff.cbegin(), v_asks_diff.cend(), std::ostream_iterator<order>(std::cout << "\n"));

        vector<order> v_market_diff;

        prepareOrderVector(market_order_count, 1, 0, 0.0, 11.45, 1242.02, v_market_diff);
        prepareOrderVector(market_order_count, 0, 0, DBL_MAX, 12.01, 1242.02, v_market_diff);

        std::copy(v_market_diff.cbegin(), v_market_diff.cend(), std::ostream_iterator<order>(std::cout << "\n"));

        cout << "bids:" << endl;
        print_orders(bids);
        cout << "asks:" << endl;
        print_orders(asks);
        cout << "matches:" << endl;
        std::copy(matches.cbegin(), matches.cend(), std::ostream_iterator<match>(std::cout << "\n"));

        for (auto ord: v_bids_diff) {
            match_order(ord, bids, asks, matches);
        }

        cout << "_________________________________" << endl;
        std::copy(v_asks_diff.cbegin(), v_asks_diff.cend(), std::ostream_iterator<order>(std::cout << "\n"));
        for (auto ord: v_asks_diff) {
            //assert(ord.cud == 0);
            match_order(ord, bids, asks, matches);
        }

        if(v_asks_diff.size() < order_count){
            vector<order> v_new_asks;
            prepareOrderVector(order_count - v_asks_diff.size(), 1, 0, 3.33, 3.48, 11.45, 1242.02, v_new_asks);
            for (auto ord: v_new_asks) {
                //assert(ord.cud == 0);
                match_order(ord, bids, asks, matches);
            }
        }



        cout << "_________________________________" << endl;
        for (auto ord: v_market_diff) {
            match_order(ord, bids, asks, matches);
        }

        cout << "bids:" << endl;
        print_orders(bids);
        cout << "asks:" << endl;
        print_orders(asks);
        cout << "matches:" << endl;
        std::copy(matches.cbegin(), matches.cend(), std::ostream_iterator<match>(std::cout << "\n"));

        matches.clear();

    }

*/
/*    auto bidsBeg = bids.get<PriceTimeIdx>().begin();
    auto next = std::next(bidsBeg, 2);

    auto orderToBeDeleted = *next;

    order dor(orderToBeDeleted.price,orderToBeDeleted.epochMilli,
              orderToBeDeleted.quantity,orderToBeDeleted.id,orderToBeDeleted.ot,
              2);
    match_order(dor,bids,asks,matches);*//*



    epoch_milli_various_order_end = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    cout << "bids left " << bids.size() << endl;
    cout << "asks left " << asks.size() << endl;

    duration = epoch_milli_various_order_end - epoch_milli_various_order_start;

    cout << "time_elapsed_various_order_processed: " << duration << endl;

    cout << "match size " << matches.size() << endl;


*/
/*    long epoch_milli_various_order_end = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    cout << "bids left " << bids.size() << endl;
    cout << "asks left " << asks.size() << endl;

    auto duration = epoch_milli_various_order_end - epoch_milli_various_order_start;

    cout << "time_elapsed_various_order_processed: " << duration << endl;

    cout << "match size " << matches.size() << endl;

    if(duration == 0)
        duration ++;

    cout << "match per second " << matches.size() * 1000 / duration << endl;
    cout << "order per second " << order_count * 4 * 1000 / duration << endl;*//*

}

void churn3(){
    TOrders asks;
    TOrders bids;

    std::vector<order> v_various_orders;
    prepareOrderVector(250000,1,0,DBL_MIN,11.45, 1242.02,v_various_orders);
    prepareOrderVector(250000,1,0,0.0,11.45, 1242.02,v_various_orders);
    prepareOrderVector(250000, 0,0,3.02, 3.29,12.01, 1242.02,v_various_orders);
    prepareOrderVector(250000, 0,0,DBL_MAX, 12.01, 1242.02,v_various_orders);
    prepareOrderVector(250000,1,0,3.33, 3.48,11.45, 1242.02,v_various_orders);

    TOrders various_orders;

    various_orders.insert(begin(v_various_orders), end(v_various_orders));
    cout << "various_orders.size() " << various_orders.size() << endl;
    std::vector<match> matches;
    long epoch_milli_various_order_start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    for(auto order : various_orders){
        match_order(order, bids, asks, matches);
    }
    long epoch_milli_various_order_end = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    cout << "bids left " << bids.size() << endl;
    cout << "asks left " << asks.size() << endl;

    auto duration = epoch_milli_various_order_end - epoch_milli_various_order_start;

    cout << "time_elapsed_various_order_processed: " << duration << endl;

    cout << "match size " << matches.size() << endl;

    if(duration == 0)
        duration ++;

    cout << "match per second " << matches.size() * 1000 / duration << endl;
}
*/
