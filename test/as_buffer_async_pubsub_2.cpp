// Copyright Takatoshi Kondo 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_main.hpp"
#include "combi_test.hpp"

#include <vector>
#include <string>

BOOST_AUTO_TEST_SUITE(test_as_buffer_async_pubsub_2)

BOOST_AUTO_TEST_CASE( pub_qos0_sub_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_clean_session(true);

        std::uint16_t pid_sub;
        std::uint16_t pid_unsub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack",
            // subscribe topic1 QoS2
            "h_suback",
            // publish topic1 QoS0
            "h_publish",
            "h_unsuback",
            // disconnect
            "h_close",
            "finish",
        };

        auto current =
            [&order, &expected]() -> std::string {
                try {
                    return expected.at(order);
                }
                catch (std::out_of_range const& e) {
                    return e.what();
                }
            };

        c->set_connack_handler(
            [&order, &current, &c, &pid_sub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(current() == "h_connack");
                ++order;
                BOOST_TEST(sp == false);
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_sub = c->async_subscribe(
                    as::buffer(*topic1),
                    mqtt::qos::exactly_once,
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_close_handler(
            [&order, &current, &s]
            () {
                BOOST_TEST(current() == "h_close");
                ++order;
                s.close();
            });
        c->set_error_handler(
            []
            (boost::system::error_code const&) {
                BOOST_CHECK(false);
            });
        c->set_puback_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubrec_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubcomp_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pub_res_sent_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
            });
        c->set_suback_handler(
            [&order, &current, &c, &pid_sub]
            (packet_id_t packet_id, std::vector<boost::optional<std::uint8_t>> results) {
                BOOST_TEST(current() == "h_suback");
                ++order;
                BOOST_TEST(packet_id == pid_sub);
                BOOST_TEST(results.size() == 1U);
                BOOST_TEST(*results[0] == mqtt::qos::exactly_once);
                auto topic1 = std::make_shared<std::string>("topic1");
                auto contents = std::make_shared<std::string>("topic1_contents");
                c->async_publish_at_most_once(
                    as::buffer(*topic1),
                    as::buffer(*contents),
                    false,
                    [topic1, contents](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_unsuback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_unsuback");
                ++order;
                BOOST_TEST(packet_id == pid_unsub);
                c->async_disconnect();
                return true;
            });
        c->set_publish_handler(
            [&order, &current, &c, &pid_unsub]
            (std::uint8_t header,
             boost::optional<packet_id_t> packet_id,
             std::string topic,
             std::string contents) {
                BOOST_TEST(current() == "h_publish");
                ++order;
                BOOST_TEST(mqtt::publish::is_dup(header) == false);
                BOOST_TEST(mqtt::publish::get_qos(header) == mqtt::qos::at_most_once);
                BOOST_TEST(mqtt::publish::is_retain(header) == false);
                BOOST_CHECK(!packet_id);
                BOOST_TEST(topic == "topic1");
                BOOST_TEST(contents == "topic1_contents");
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_unsub = c->async_unsubscribe(
                    as::buffer(*topic1),
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( pub_qos1_sub_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_clean_session(true);

        std::uint16_t pid_pub;
        std::uint16_t pid_sub;
        std::uint16_t pid_unsub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack",
            // subscribe topic1 QoS0
            "h_suback",
            // publish topic1 QoS1
            "h_publish",
            "h_pub_res_sent",
            "h_puback",
            "h_unsuback",
            // disconnect
            "h_close",
            "finish",
        };

        auto current =
            [&order, &expected]() -> std::string {
                try {
                    return expected.at(order);
                }
                catch (std::out_of_range const& e) {
                    return e.what();
                }
            };

        c->set_connack_handler(
            [&order, &current, &c, &pid_sub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(current() == "h_connack");
                ++order;
                BOOST_TEST(sp == false);
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_sub = c->async_subscribe(
                    as::buffer(*topic1),
                    mqtt::qos::exactly_once,
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_close_handler(
            [&order, &current, &s]
            () {
                BOOST_TEST(current() == "h_close");
                ++order;
                s.close();
            });
        c->set_error_handler(
            []
            (boost::system::error_code const&) {
                BOOST_CHECK(false);
            });
        c->set_puback_handler(
            [&order, &current, &c, &pid_pub, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_CHECK(current() == "h_puback");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_unsub = c->async_unsubscribe(
                    as::buffer(*topic1),
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_pubrec_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubcomp_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        boost::optional<std::uint16_t> recv_packet_id;
        c->set_pub_res_sent_handler(
            [&order, &current, &recv_packet_id]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pub_res_sent");
                ++order;
                BOOST_TEST(*recv_packet_id == packet_id);
            });
        c->set_suback_handler(
            [&order, &current, &c, &pid_sub, &pid_pub]
            (packet_id_t packet_id, std::vector<boost::optional<std::uint8_t>> results) {
                BOOST_TEST(current() == "h_suback");
                ++order;
                BOOST_TEST(packet_id == pid_sub);
                BOOST_TEST(results.size() == 1U);
                BOOST_TEST(*results[0] == mqtt::qos::exactly_once);
                auto topic1 = std::make_shared<std::string>("topic1");
                auto contents = std::make_shared<std::string>("topic1_contents");
                pid_pub = c->async_publish_at_least_once(
                    as::buffer(*topic1),
                    as::buffer(*contents),
                    [topic1, contents] {}
                );
                return true;
            });
        c->set_unsuback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_unsuback");
                ++order;
                BOOST_TEST(packet_id == pid_unsub);
                c->async_disconnect();
                return true;
            });
        c->set_publish_handler(
            [&order, &current, &recv_packet_id]
            (std::uint8_t header,
             boost::optional<packet_id_t> packet_id,
             std::string topic,
             std::string contents) {
                BOOST_CHECK(current() == "h_publish");
                ++order;
                BOOST_TEST(mqtt::publish::is_dup(header) == false);
                BOOST_TEST(mqtt::publish::get_qos(header) == mqtt::qos::at_least_once);
                BOOST_TEST(mqtt::publish::is_retain(header) == false);
                BOOST_TEST(*packet_id != 0);
                recv_packet_id = packet_id;
                BOOST_TEST(topic == "topic1");
                BOOST_TEST(contents == "topic1_contents");
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}


BOOST_AUTO_TEST_CASE( pub_qos2_sub_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_clean_session(true);

        std::uint16_t pid_pub;
        std::uint16_t pid_sub;
        std::uint16_t pid_unsub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack",
            // subscribe topic1 QoS0
            "h_suback",
            // publish topic1 QoS2
            "h_publish",
            "h_pubrec",
            "h_pub_res_sent",
            "h_pubcomp",
            "h_unsuback",
            // disconnect
            "h_close",
            "finish",
        };

        auto current =
            [&order, &expected]() -> std::string {
                try {
                    return expected.at(order);
                }
                catch (std::out_of_range const& e) {
                    return e.what();
                }
            };

        c->set_connack_handler(
            [&order, &current, &c, &pid_sub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(current() == "h_connack");
                ++order;
                BOOST_TEST(sp == false);
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_sub = c->async_subscribe(
                    as::buffer(*topic1),
                    mqtt::qos::exactly_once,
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_close_handler(
            [&order, &current, &s]
            () {
                BOOST_TEST(current() == "h_close");
                ++order;
                s.close();
            });
        c->set_error_handler(
            []
            (boost::system::error_code const&) {
                BOOST_CHECK(false);
            });
        c->set_puback_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubrec_handler(
            [&order, &current, &pid_pub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pubrec");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                return true;
            });
        c->set_pubcomp_handler(
            [&order, &current, &c, &pid_pub, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pubcomp");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_unsub = c->async_unsubscribe(
                    as::buffer(*topic1),
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        boost::optional<std::uint16_t> recv_packet_id;
        c->set_pub_res_sent_handler(
            [&order, &current, &recv_packet_id]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pub_res_sent");
                ++order;
                BOOST_TEST(*recv_packet_id == packet_id);
            });
        c->set_suback_handler(
            [&order, &current, &c, &pid_sub, &pid_pub]
            (packet_id_t packet_id, std::vector<boost::optional<std::uint8_t>> results) {
                BOOST_TEST(current() == "h_suback");
                ++order;
                BOOST_TEST(packet_id == pid_sub);
                BOOST_TEST(results.size() == 1U);
                BOOST_TEST(*results[0] == mqtt::qos::exactly_once);
                auto topic1 = std::make_shared<std::string>("topic1");
                auto contents = std::make_shared<std::string>("topic1_contents");
                pid_pub = c->async_publish_exactly_once(
                    as::buffer(*topic1),
                    as::buffer(*contents),
                    [topic1, contents] {}
                );
                return true;
            });
        c->set_unsuback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_unsuback");
                ++order;
                BOOST_TEST(packet_id == pid_unsub);
                c->async_disconnect();
                return true;
            });
        c->set_publish_handler(
            [&order, &current, &recv_packet_id]
            (std::uint8_t header,
             boost::optional<packet_id_t> packet_id,
             std::string topic,
             std::string contents) {
                BOOST_TEST(current() == "h_publish");
                ++order;
                BOOST_TEST(mqtt::publish::is_dup(header) == false);
                BOOST_TEST(mqtt::publish::get_qos(header) == mqtt::qos::exactly_once);
                BOOST_TEST(mqtt::publish::is_retain(header) == false);
                BOOST_TEST(*packet_id != 0);
                recv_packet_id = packet_id;
                BOOST_TEST(topic == "topic1");
                BOOST_TEST(contents == "topic1_contents");
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( publish_function ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_clean_session(true);

        std::uint16_t pid_sub;
        std::uint16_t pid_unsub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack",
            // subscribe topic1 QoS0
            "h_suback",
            // publish topic1 QoS0
            "h_publish",
            "h_unsuback",
            // disconnect
            "h_close",
            "finish",
        };

        auto current =
            [&order, &expected]() -> std::string {
                try {
                    return expected.at(order);
                }
                catch (std::out_of_range const& e) {
                    return e.what();
                }
            };

        c->set_connack_handler(
            [&order, &current, &c, &pid_sub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(current() == "h_connack");
                ++order;
                BOOST_TEST(sp == false);
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_sub = c->async_subscribe(
                    as::buffer(*topic1),
                    mqtt::qos::at_most_once,
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_close_handler(
            [&order, &current, &s]
            () {
                BOOST_TEST(current() == "h_close");
                ++order;
                s.close();
            });
        c->set_error_handler(
            []
            (boost::system::error_code const&) {
                BOOST_CHECK(false);
            });
        c->set_puback_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubrec_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubcomp_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_suback_handler(
            [&order, &current, &c, &pid_sub]
            (packet_id_t packet_id, std::vector<boost::optional<std::uint8_t>> results) {
                BOOST_TEST(current() == "h_suback");
                ++order;
                BOOST_TEST(packet_id == pid_sub);
                BOOST_TEST(results.size() == 1U);
                BOOST_TEST(*results[0] == mqtt::qos::at_most_once);
                auto topic1 = std::make_shared<std::string>("topic1");
                auto contents = std::make_shared<std::string>("topic1_contents");
                c->async_publish(
                    as::buffer(*topic1),
                    as::buffer(*contents),
                    [topic1, contents] {},
                    mqtt::qos::at_most_once
                );
                return true;
            });
        c->set_unsuback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_unsuback");
                ++order;
                BOOST_TEST(packet_id == pid_unsub);
                c->async_disconnect();
                return true;
            });
        c->set_publish_handler(
            [&order, &current, &c, &pid_unsub]
            (std::uint8_t header,
             boost::optional<packet_id_t> packet_id,
             std::string topic,
             std::string contents) {
                BOOST_TEST(current() == "h_publish");
                ++order;
                BOOST_TEST(mqtt::publish::is_dup(header) == false);
                BOOST_TEST(mqtt::publish::get_qos(header) == mqtt::qos::at_most_once);
                BOOST_TEST(mqtt::publish::is_retain(header) == false);
                BOOST_CHECK(!packet_id);
                BOOST_TEST(topic == "topic1");
                BOOST_TEST(contents == "topic1_contents");
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_unsub = c->async_unsubscribe(
                    as::buffer(*topic1),
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( publish_dup_function ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_clean_session(true);

        std::uint16_t pid_sub;
        std::uint16_t pid_unsub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack",
            // subscribe topic1 QoS1
            "h_suback",
            // publish topic1 QoS1
            "h_publish",
            "h_puback",
            "h_unsuback",
            // disconnect
            "h_close",
            "finish",
        };

        auto current =
            [&order, &expected]() -> std::string {
                try {
                    return expected.at(order);
                }
                catch (std::out_of_range const& e) {
                    return e.what();
                }
            };

        c->set_connack_handler(
            [&order, &current, &c, &pid_sub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(current() == "h_connack");
                ++order;
                BOOST_TEST(sp == false);
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_sub = c->async_subscribe(
                    as::buffer(*topic1),
                    mqtt::qos::at_least_once,
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_close_handler(
            [&order, &current, &s]
            () {
                BOOST_TEST(current() == "h_close");
                ++order;
                s.close();
            });
        c->set_error_handler(
            []
            (boost::system::error_code const&) {
                BOOST_CHECK(false);
            });
        c->set_puback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_puback");
                ++order;
                BOOST_TEST(packet_id == 1);
                auto topic1 = std::make_shared<std::string>("topic1");
                pid_unsub = c->async_unsubscribe(
                    as::buffer(*topic1),
                    [topic1](boost::system::error_code const&) {}
                );
                return true;
            });
        c->set_pubrec_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_pubcomp_handler(
            []
            (std::uint16_t) {
                BOOST_CHECK(false);
                return true;
            });
        c->set_suback_handler(
            [&order, &current, &c, &pid_sub]
            (packet_id_t packet_id, std::vector<boost::optional<std::uint8_t>> results) {
                BOOST_TEST(current() == "h_suback");
                ++order;
                BOOST_TEST(packet_id == pid_sub);
                BOOST_TEST(results.size() == 1U);
                BOOST_TEST(*results[0] == mqtt::qos::at_least_once);
                auto topic1 = std::make_shared<std::string>("topic1");
                auto contents = std::make_shared<std::string>("topic1_contents");
                auto ret = c->async_publish_dup(
                    1,
                    as::buffer(*topic1),
                    as::buffer(*contents),
                    [topic1, contents] {},
                    mqtt::qos::at_least_once
                );
                BOOST_TEST(ret == true);
                return true;
            });
        c->set_unsuback_handler(
            [&order, &current, &c, &pid_unsub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_unsuback");
                ++order;
                BOOST_TEST(packet_id == pid_unsub);
                c->async_disconnect();
                return true;
            });
        c->set_publish_handler(
            [&order, &current]
            (std::uint8_t header,
             boost::optional<packet_id_t> packet_id,
             std::string topic,
             std::string contents) {
                BOOST_TEST(current() == "h_publish");
                ++order;
                BOOST_TEST(mqtt::publish::is_dup(header) == false); // not propagated
                BOOST_TEST(mqtt::publish::get_qos(header) == mqtt::qos::at_least_once);
                BOOST_TEST(mqtt::publish::is_retain(header) == false);
                BOOST_CHECK(packet_id.get() == 1);
                BOOST_TEST(topic == "topic1");
                BOOST_TEST(contents == "topic1_contents");
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_SUITE_END()
