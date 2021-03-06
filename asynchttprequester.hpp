/*
 *  Copyright (C) 2013 Alexander Pokluda
 *
 *  This file is part of pWeb.
 *
 *  pWeb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  pWeb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with pWeb.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ASYNCHTTPREQUESTER_HPP_
#define ASYNCHTTPREQUESTER_HPP_

#include <curl/curl.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

namespace curl
{
    void init();
    void cleanup();

    class AsyncHTTPRequester;

    class socket_map_t
    {
        typedef curl_socket_t key_t;
        typedef boost::asio::ip::tcp::socket* value_t;
        typedef std::map<key_t, value_t> map_t;

    public:
        void insert(const key_t k, const value_t v)
        {
            map_impl_.insert(std::pair<key_t, value_t>(k, v));
        }

        value_t find(const key_t k)
        {
            map_t::iterator it = map_impl_.find(k);
            if ( it == map_impl_.end() ) return 0;
            return it->second;
        }

        bool erase_and_delete(const key_t k)
        {
            map_t::iterator it = map_impl_.find(k);
            if ( it != map_impl_.end() )
            {
                delete it->second;
                map_impl_.erase(it);
                return true;
            }
            return false;
        }

    private:
        map_t map_impl_;
    };

    class Context : boost::noncopyable
    {
        friend class AsyncHTTPRequester;
        friend curl_socket_t opensocket(Context*, curlsocktype, struct curl_sockaddr*);
        friend int closesocket(Context*c, curl_socket_t);
        friend void setsock(int*, curl_socket_t, CURL*, int, Context*);
        friend void addsock(curl_socket_t, CURL*, int, Context*);
        friend void timer_cb(const boost::system::error_code&, Context*);
        friend void event_cb(Context*, boost::asio::ip::tcp::socket*, int);
        friend void check_multi_info(Context*);
        friend int multi_timer_cb(CURLM*, long, Context*);
        friend void context_multi_add_handle(Context&, CURL*);

    public:
        Context(boost::asio::io_service& io_service, bool verify_ssl_peer = true);
        ~Context();

        inline boost::asio::io_service& get_io_service()
        {
            return io_service_;
        }

        inline boost::asio::io_service const& get_io_service() const
        {
            return io_service_;
        }

    private:
        socket_map_t socket_map_;
        boost::asio::deadline_timer timer_;
        boost::asio::strand strand_;
        boost::asio::io_service& io_service_;
        CURLM* multi_;
        int still_running_;
        bool verify_ssl_peer_;
    };

    class AsyncHTTPRequester : public boost::enable_shared_from_this< AsyncHTTPRequester >
    {
        friend size_t write_cb(char*, size_t, size_t, AsyncHTTPRequester*);
        friend void check_multi_info(Context*);

    public:
        AsyncHTTPRequester(Context& c, bool const selfmanage = true);

        Context& get_context()
        {
            return c_;
        }

        Context const& get_context() const
        {
            return c_;
        }

        ~AsyncHTTPRequester();

        void fetch(std::string const& url, boost::function< void(CURLcode, std::string const&) > cb, std::string const& = std::string() );

    private:
        void done(CURLcode const rc);

        static const std::size_t MAX_BUF_SIZE = 1024*1024; // Max amount of data we are willing to receive/buffer (1 MiB)

        std::ostringstream buf_;
        boost::function< void(CURLcode, std::string const&) > cb_;
        boost::shared_ptr< AsyncHTTPRequester > ptr_to_this_;
        CURLMcode rc_;
        CURL* easy_;
        curl_slist* headers_;
        Context& c_;
        bool const selfmanage_;
    };
}

#endif /* ASIOHELPER_HPP_ */
