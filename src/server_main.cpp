#include <iostream>
#include <memory>
#include <asio.hpp>
#include <chrono>
#include "protocol.h"
#include "order_book.h"

#ifdef _WIN32
    #include <winsock2.h>
    #define ntohll _byteswap_uint64
#else
    #include <arpa/inet.h>
    #define ntohll __builtin_bswap64
#endif

using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
   Session(tcp::socket socket, LimitOrderBook& lob)
        : socket_(std::move(socket)), lob_(lob), total_orders_received_(0) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        // Reads exactly the size of binary struct from the network
        asio::async_read(socket_,
            asio::buffer(&msg_buffer_, sizeof(OrderMessage)),
            [this, self](std::error_code ec, std::size_t) {
                if (!ec) {
                    process_message();
                    do_read();
                } else if (ec != asio::error::eof) {
                    std::cerr << "Read error: " << ec.message() << "\n";
                }
        });
    }

    void process_message() {
        if (total_orders_received_ == 0) {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

        total_orders_received_++;
        
        // Big-Endian to Little-Endian
        uint64_t host_order_id = ntohll(msg_buffer_.order_id);
        uint32_t host_price = ntohl(msg_buffer_.price);
        uint32_t host_qty = ntohl(msg_buffer_.quantity);

        switch(msg_buffer_.type) {
            case 'A':
                lob_.add_order(host_order_id,
                               host_price,
                               host_qty,
                               msg_buffer_.side == 'B');
            case 'C':
                lob_.cancel_order(host_order_id);
                break;
            default:
                break;
        }

        if (total_orders_received_ == 1000000) {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time_;
            
            double mps = 1000000.0 / elapsed.count();
            
            printf("\n--- Server-Side Metrics ---\n");
            printf("Processed 1,000,000 orders in: %.4f seconds\n", elapsed.count());
            printf("True Engine Throughput: %.0f messages/sec\n\n", mps);
            
            total_orders_received_ = 0; 
        }
    }

    tcp::socket socket_;
    LimitOrderBook& lob_;
    OrderMessage msg_buffer_; // Strictly typed to binary protocol

    uint64_t total_orders_received_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

class TcpServer{
public:
    TcpServer(asio::io_context& io_context, short port, LimitOrderBook& lob) 
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          lob_(lob) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](std::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New client connected.\n";
                    std::make_shared<Session>(std::move(socket), lob_)->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    LimitOrderBook& lob_;
};

int main() {
    try {
        LimitOrderBook my_lob(100000, 1500000);

        asio::io_context io_context;
        short port = 8080;

        std::cout << "Starting LOB Matching Engine TCP Server on port " << port << "...\n";

        TcpServer server(io_context, port, my_lob);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exeception: " << e.what() << "\n";
    }

    return 0;
}