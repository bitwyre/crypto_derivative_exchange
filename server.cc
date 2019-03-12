#include <string>
#include <iomanip>
#include <ostream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>

#include <cpp_redis/cpp_redis>

#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using order::OrderReceive;
using order::OrderRequest;
using order::OrderReply;


class OrderServiceImplementation final : public OrderReceive::Service {
    Status sendRequest(
        ServerContext* context, 
        const OrderRequest* request, 
        OrderReply* reply
    ) override {
        //! Enable logging
        cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);
        cpp_redis::client client;
        client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
            if (status == cpp_redis::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
                }
        });

        // Example computation on server
        // timestamp received order
        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);
        std::ostringstream ss;
        ss << std::put_time(gmtime(&itt), "%FT%TZ");
        
        // requesting value of quantity
        int quantity = request->quantity();
        // requesting value of price
        double price = request->price();
        // requesting type
        bool type = request->type();
        // requesting instrument
        std::string inst = request->inst();
        // requesting user
        std::string user = request->user();
        // requesting pass
        std::string pass = request->pass();
        
        // create json from values obtained

        // Dump values to Redis
        client.set(ss.str(), std::to_string(price), [](cpp_redis::reply& reply) {});
        
        // Show orders
        std::cout << ss.str()
                  << " timed. Order consists of "
                  << "quantity " << quantity
                  << " price " << price
                  << " type " << type
                  << " instrument " << inst
                  << " username " << user
                  << " password " << pass
                  << std::endl;

        int result = 0;
        
        // send result back to client
        reply->set_result(result);
        // return an OK status
        return Status::OK;
    } 
};

void Run() {
    std::string address("0.0.0.0:5000");
    OrderServiceImplementation service;

    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    Run();

    return 0;
}
