/*
 * Copyright (c) 2012, Shumin Huang. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "parser.hpp"
#include "mime.hpp"

#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace websocketpp {
namespace role {
    template <class endpoint> class server;
    template <class endpoint> class client;
}

namespace http {
template <class endpoint, class role_type = typename endpoint::role_type>
struct handler;

/// specialize server http handler
template <class endpoint>
struct handler < endpoint, typename websocketpp::role::server<endpoint> >
{
    typedef typename endpoint::connection_ptr connection_ptr;
    // called by http() to process regular http request(non-websocket)
    inline void process_request(connection_ptr con);
    virtual void do_get(connection_ptr con); // server process get
    virtual void do_post(connection_ptr con); // server process post
    //virtual void do_put(connection_ptr con); // server process put
    //virtual void do_head(connection_ptr con); // server process head
    //virtual void handle_get(connection_ptr con); // asynchronous handle get
    //virtual void handle_post(connection_ptr con); // asynchronous handle post
};

/// specialize client http handler
template <class endpoint>
struct handler < endpoint, typename websocketpp::role::client<endpoint> >
{
    typedef typename endpoint::connection_ptr connection_ptr;
    // called by http() to process regular http response(non-websocket)
    inline void process_response(connection_ptr con);
};

// server process regular http request(non-websocket)
template <typename endpoint>
inline void handler< endpoint, typename websocketpp::role::server<endpoint> >::
process_request(connection_ptr con) {
    const std::string & method = con->get_method();
    if (method == "GET") {
        do_get(con);
    } else if (method == "POST") {
        do_post(con);
    } else { // HEAD PUT DELETE TRACE OPTIONS CONNECT
        con->set_body("Unsupported method: " + method);
        con->replace_response_header("Content-Type", "text/plain");
    }
    // More additional headers
    //con->replace_response_header("Date", "Current Time");
    //con->replace_response_header("Accept-Ranges", "bytes");
    //con->replace_response_header("Vary", "Accept-Encoding");
    // needs to implement cookie authentication
    //con->replace_response_header("Set-Cookie",
    //                             "name=value;"
    //                             "Expires=Fri, 21-Dec-2012 23:59:59 GMT");
}

// server process get
template <typename endpoint>
void handler< endpoint, typename websocketpp::role::server<endpoint> >::
do_get(connection_ptr con) {
    boost::filesystem::path p(boost::filesystem::current_path());
    p /= (boost::ends_with(con->get_resource(), "/") ? "index.html" :
            con->get_resource());

    if (boost::filesystem::exists(p)) {
        boost::filesystem::ifstream f(p, std::ios::binary);
        if (!f.fail()) {
            std::string body;
            body.reserve(boost::filesystem::file_size(p));
            body.assign((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
            con->set_body(body);
            con->replace_response_header("Content-Type",
                    mime::extension_to_mime(p.extension().string()));
            f.close();
        } else { // including directory or regular file without read permissions
            throw (websocketpp::http::exception("Request for invalid resource",
                        websocketpp::http::status_code::FORBIDDEN,
                        "Request for invalid resource",
                        "<html><head><title>403</title></head>"
                        "<body>Invalid Request</body></html>"));
        }

    } else {
        throw (websocketpp::http::exception("Request for unknown resource",
                    websocketpp::http::status_code::NOT_FOUND,
                    "Request for unknown resource",
                    "<html><head><title>404</title></head>"
                    "<body>File Not Found!</body></html>"));
    }
    return;
}

// server process post
template <typename endpoint>
void handler< endpoint, typename websocketpp::role::server<endpoint> >::
do_post(connection_ptr con) {
    // TODO: implement multiform parser and post method reference to cpp-netlib
}


// client process regular http response(non-websocket)
template <typename endpoint>
inline void handler< endpoint, typename websocketpp::role::client<endpoint> >::
process_response(connection_ptr con) {
    // read response to process/write request to send(NOT implement) 
    if (con->m_response.get_status_code() == status_code::OK) {
        std::istream body(con->buffer());
    } else {
        std::string status = con->m_response.get_status_msg();
    }
}

} // namespace http
} // namespace websocketpp

#endif // HTTP_HANDLER_HPP
