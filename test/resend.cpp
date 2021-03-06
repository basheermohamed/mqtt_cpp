// Copyright Takatoshi Kondo 2015
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_main.hpp"
#include "combi_test.hpp"

BOOST_AUTO_TEST_SUITE(test_resend)

BOOST_AUTO_TEST_CASE( publish_qos1 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_client_id("cid1");
        c->set_clean_session(true);

        std::uint16_t pid_pub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack1",
            // disconnect
            "h_close1",
            // connect
            "h_connack2",
            // publish topic1 QoS1
            // force_disconnect
            "h_error",
            // connect
            "h_connack3",
            "h_puback",
            // disconnect
            "h_close2",
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
            [&order, &current, &c, &pid_pub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                switch (order) {
                case 0: // clean session
                    BOOST_TEST(current() == "h_connack1");
                    ++order;
                    BOOST_TEST(sp == false);
                    c->disconnect();
                    break;
                case 2:
                    BOOST_TEST(current() == "h_connack2");
                    ++order;
                    BOOST_TEST(sp == false);
                    pid_pub = c->publish_at_least_once("topic1", "topic1_contents");
                    c->force_disconnect();
                    break;
                case 4:
                    BOOST_TEST(current() == "h_connack3");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_close_handler(
            [&order, &current, &c, &s]
            () {
                switch (order) {
                case 1:
                    BOOST_TEST(current() == "h_close1");
                    ++order;
                    c->set_clean_session(false);
                    c->connect();
                    break;
                case 6:
                    BOOST_TEST(current() == "h_close2");
                    ++order;
                    s.close();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_error_handler(
            [&order, &current, &c]
            (boost::system::error_code const&) {
                BOOST_TEST(current() == "h_error");
                ++order;
                c->connect();
            });
        c->set_puback_handler(
            [&order, &current, &c, &pid_pub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_puback");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                c->disconnect();
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( publish_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_client_id("cid1");
        c->set_clean_session(true);

        std::uint16_t pid_pub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack1",
            // disconnect
            "h_close1",
            // connect
            "h_connack2",
            // publish topic1 QoS2
            // force_disconnect
            "h_error",
            // connect
            "h_connack3",
            "h_pubrec",
            "h_pubcomp",
            // disconnect
            "h_close2",
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
            [&order, &current, &c, &pid_pub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                switch (order) {
                case 0: // clean session
                    BOOST_TEST(current() == "h_connack1");
                    ++order;
                    BOOST_TEST(sp == false);
                    c->disconnect();
                    break;
                case 2:
                    BOOST_TEST(current() == "h_connack2");
                    ++order;
                    BOOST_TEST(sp == false);
                    pid_pub = c->publish_exactly_once("topic1", "topic1_contents");
                    c->force_disconnect();
                    break;
                case 4:
                    BOOST_TEST(current() == "h_connack3");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_close_handler(
            [&order, &current, &c, &s]
            () {
                switch (order) {
                case 1:
                    BOOST_TEST(current() == "h_close1");
                    ++order;
                    c->set_clean_session(false);
                    c->connect();
                    break;
                case 7:
                    BOOST_TEST(current() == "h_close2");
                    ++order;
                    s.close();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_error_handler(
            [&order, &current, &c]
            (boost::system::error_code const&) {
                BOOST_TEST(current() == "h_error");
                ++order;
                c->connect();
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
            [&order, &current, &c, &pid_pub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pubcomp");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                c->disconnect();
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( pubrel_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_client_id("cid1");
        c->set_clean_session(true);

        std::uint16_t pid_pub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack1",
            // disconnect
            "h_close1",
            // connect
            "h_connack2",
            // publish topic1 QoS2
            "h_pubrec",
            // force_disconnect
            "h_error",
            // connect
            "h_connack3",
            "h_pubcomp",
            // disconnect
            "h_close2",
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
            [&order, &current, &c, &pid_pub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                switch (order) {
                case 0: // clean session
                    BOOST_TEST(current() == "h_connack1");
                    ++order;
                    BOOST_TEST(sp == false);
                    c->disconnect();
                    break;
                case 2:
                    BOOST_TEST(current() == "h_connack2");
                    ++order;
                    BOOST_TEST(sp == false);
                    pid_pub = c->publish_exactly_once("topic1", "topic1_contents");
                    break;
                case 5:
                    BOOST_TEST(current() == "h_connack3");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_close_handler(
            [&order, &current, &c, &s]
            () {
                switch (order) {
                case 1:
                    BOOST_TEST(current() == "h_close1");
                    ++order;
                    c->set_clean_session(false);
                    c->connect();
                    break;
                case 7:
                    BOOST_TEST(current() == "h_close2");
                    ++order;
                    s.close();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_error_handler(
            [&order, &current, &c]
            (boost::system::error_code const&) {
                BOOST_TEST(current() == "h_error");
                ++order;
                c->connect();
            });
        c->set_pubrec_handler(
            [&order, &current, &c, &pid_pub]
            (packet_id_t packet_id) {
                switch (order) {
                case 3:
                    BOOST_TEST(current() == "h_pubrec");
                    ++order;
                    BOOST_TEST(packet_id == pid_pub);
                    c->force_disconnect();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_pubcomp_handler(
            [&order, &current, &c]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pubcomp");
                ++order;
                BOOST_TEST(packet_id == 1);
                c->disconnect();
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( publish_pubrel_qos2 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_client_id("cid1");
        c->set_clean_session(true);

        std::uint16_t pid_pub;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack1",
            // disconnect
            "h_close1",
            // connect
            "h_connack2",
            // publish topic1 QoS2
            // force_disconnect
            "h_error1",
            // connect
            "h_connack3",
            "h_pubrec",
            // force_disconnect
            "h_error2",
            // connect
            "h_connack4",
            "h_pubcomp",
            // disconnect
            "h_close2",
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
            [&order, &current, &c, & pid_pub]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                switch (order) {
                case 0: // clean session
                    BOOST_TEST(current() == "h_connack1");
                    ++order;
                    BOOST_TEST(sp == false);
                    c->disconnect();
                    break;
                case 2:
                    BOOST_TEST(current() == "h_connack2");
                    ++order;
                    BOOST_TEST(sp == false);
                    pid_pub = c->publish_exactly_once("topic1", "topic1_contents");
                    c->force_disconnect();
                    break;
                case 4:
                    BOOST_TEST(current() == "h_connack3");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                case 7:
                    BOOST_TEST(current() == "h_connack4");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_close_handler(
            [&order, &current, &c, &s]
            () {
                switch (order) {
                case 1:
                    BOOST_TEST(current() == "h_close1");
                    ++order;
                    c->set_clean_session(false);
                    c->connect();
                    break;
                case 9:
                    BOOST_TEST(current() == "h_close2");
                    ++order;
                    s.close();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_error_handler(
            [&order, &current, &c]
            (boost::system::error_code const&) {
                switch (order) {
                case 3:
                    BOOST_TEST(current() == "h_error1");
                    ++order;
                    c->connect();
                    break;
                case 6:
                    BOOST_TEST(current() == "h_error2");
                    ++order;
                    c->connect();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_pubrec_handler(
            [&order, &current, &c, &pid_pub]
            (packet_id_t packet_id) {
                switch (order) {
                case 5:
                    BOOST_TEST(current() == "h_pubrec");
                    ++order;
                    BOOST_TEST(packet_id == pid_pub);
                    c->force_disconnect();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_pubcomp_handler(
            [&order, &current, &c, &pid_pub]
            (packet_id_t packet_id) {
                BOOST_TEST(current() == "h_pubcomp");
                ++order;
                BOOST_TEST(packet_id == pid_pub);
                c->disconnect();
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_CASE( multi_publish_qos1 ) {
    auto test = [](boost::asio::io_service& ios, auto& c, auto& s) {
        using packet_id_t = typename std::remove_reference_t<decltype(*c)>::packet_id_t;
        c->set_client_id("cid1");
        c->set_clean_session(true);

        std::uint16_t pid_pub1;
        std::uint16_t pid_pub2;

        int order = 0;

        std::vector<std::string> const expected = {
            // connect
            "h_connack1",
            // disconnect
            "h_close1",
            // connect
            "h_connack2",
            // publish topic1 QoS1
            // publish topic1 QoS1
            // force_disconnect
            "h_error1",
            // connect
            "h_connack3",
            "h_puback1",
            "h_puback2",
            // disconnect
            "h_close2",
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
            [&order, &current, &c, &pid_pub1, &pid_pub2]
            (bool sp, std::uint8_t connack_return_code) {
                BOOST_TEST(connack_return_code == mqtt::connect_return_code::accepted);
                switch (order) {
                case 0: // clean session
                    BOOST_TEST(current() == "h_connack1");
                    ++order;
                    BOOST_TEST(sp == false);
                    c->disconnect();
                    break;
                case 2:
                    BOOST_TEST(current() == "h_connack2");
                    ++order;
                    BOOST_TEST(sp == false);
                    pid_pub1 = c->publish_at_least_once("topic1", "topic1_contents1");
                    pid_pub2 = c->publish_at_least_once("topic1", "topic1_contents2");
                    c->force_disconnect();
                    break;
                case 4:
                    BOOST_TEST(current() == "h_connack3");
                    ++order;
                    BOOST_TEST(sp == true);
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
                return true;
            });
        c->set_close_handler(
            [&order, &current, &c, &s]
            () {
                switch (order) {
                case 1:
                    BOOST_TEST(current() == "h_close1");
                    ++order;
                    c->set_clean_session(false);
                    c->connect();
                    break;
                case 7:
                    BOOST_TEST(current() == "h_close2");
                    ++order;
                    s.close();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_error_handler(
            [&order, &current, &c]
            (boost::system::error_code const&) {
                switch (order) {
                case 3:
                    BOOST_TEST(current() == "h_error1");
                    ++order;
                    c->connect();
                    break;
                default:
                    BOOST_CHECK(false);
                    break;
                }
            });
        c->set_puback_handler(
            [&order, &current, &c, &pid_pub1, &pid_pub2]
            (packet_id_t packet_id) {
                switch (order) {
                case 5:
                    BOOST_TEST(current() == "h_puback1");
                    ++order;
                    BOOST_TEST(packet_id == pid_pub1);
                    break;
                case 6:
                    BOOST_TEST(current() == "h_puback2");
                    ++order;
                    BOOST_TEST(packet_id == pid_pub2);
                    c->disconnect();
                    break;
                }
                return true;
            });
        c->connect();
        ios.run();
        BOOST_TEST(current() == "finish");
    };
    do_combi_test(test);
}

BOOST_AUTO_TEST_SUITE_END()
