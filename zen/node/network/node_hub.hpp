/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <iostream>
#include <memory>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <zen/node/common/settings.hpp>
#include <zen/node/common/stopwatch.hpp>
#include <zen/node/concurrency/stoppable.hpp>
#include <zen/node/network/node.hpp>

namespace zen::network {

class NodeHub {
  public:
    explicit NodeHub(NodeSettings& node_settings)
        : node_settings_{node_settings},
          io_strand_{*node_settings.asio_context},
          socket_acceptor_{*node_settings.asio_context},
          service_timer_{*node_settings.asio_context} {};

    // Not copyable or movable
    NodeHub(NodeHub& other) = delete;
    NodeHub(NodeHub&& other) = delete;
    NodeHub& operator=(const NodeHub& other) = delete;
    ~NodeHub() = default;

    void start();
    void stop();

  private:
    void initialize_acceptor();  // Initialize the socket acceptor with local endpoint

    void start_accept();
    void handle_accept(const std::shared_ptr<Node>& new_node, const boost::system::error_code& ec);

    void on_node_disconnected(const std::shared_ptr<Node>& node);
    void on_node_data(DataDirectionMode direction, size_t bytes_transferred);

    void start_service_timer();
    void print_info();

    NodeSettings& node_settings_;  // Reference to global config settings

    boost::asio::io_context::strand io_strand_;  // Serialized execution of handlers
    boost::asio::ip::tcp::acceptor socket_acceptor_;
    boost::asio::steady_timer service_timer_;                // Service scheduler for this instance
    static const uint32_t kServiceTimerIntervalSeconds_{1};  // Delay interval for service_timer_

    SSL_CTX* ssl_server_context_{nullptr};  // For dial-in connections
    SSL_CTX* ssl_client_context_{nullptr};  // For dial-out connections

    std::atomic_uint32_t current_active_connections_{0};
    std::atomic_uint32_t current_active_inbound_connections_{0};
    std::atomic_uint32_t current_active_outbound_connections_{0};

    std::unordered_set<std::shared_ptr<Node>> nodes_;
    std::mutex nodes_mutex_;

    size_t total_connections_{0};
    size_t total_disconnections_{0};
    size_t total_rejected_connections_{0};
    std::atomic<size_t> total_bytes_received_{0};
    std::atomic<size_t> total_bytes_sent_{0};
    std::atomic<size_t> last_info_total_bytes_received_{0};
    std::atomic<size_t> last_info_total_bytes_sent_{0};
    StopWatch info_stopwatch_{/*auto_start=*/false};  // To measure the effective elapsed amongst two service_timer_
                                                      // events (for bandwidth calculation)
};
}  // namespace zen::network
