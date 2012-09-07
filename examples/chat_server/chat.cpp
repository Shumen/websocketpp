/*
 * Copyright (c) 2011, Peter Thorson. All rights reserved.
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

#include "chat.hpp"

#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace websocketchat;
//using chat_server_handler::connection_ptr;

void chat_server_handler::validate(connection_ptr con) {
    std::stringstream err;
    
    // We only know about the chat resource
    if (con->get_resource() != "/chat") {
        err << "Request for unknown resource " << con->get_resource();
        throw(websocketpp::http::exception(err.str(),websocketpp::http::status_code::NOT_FOUND));
    }
    
    // Require specific origin example
    if (con->get_origin() != "http://127.0.0.1") {
        err << "Request from unrecognized origin: " << con->get_origin();
        //throw(websocketpp::http::exception(err.str(),websocketpp::http::status_code::FORBIDDEN));
    }
}


void chat_server_handler::on_open(connection_ptr con) {
    std::cout << "client " << con << " joined the lobby." << std::endl;
    m_connections.insert(std::pair<connection_ptr,std::string>(con,get_con_id(con)));

    // send user list and signon message to all clients
    send_to_all(serialize_state());
    con->send(encode_message("server","Welcome, use the /alias command to set a name, /help for a list of other commands."));
    send_to_all(encode_message("server",m_connections[con]+" has joined the chat."));
}

void chat_server_handler::on_close(connection_ptr con) {
    std::map<connection_ptr,std::string>::iterator it = m_connections.find(con);
    
    if (it == m_connections.end()) {
        // this client has already disconnected, we can ignore this.
        // this happens during certain types of disconnect where there is a
        // deliberate "soft" disconnection preceeding the "hard" socket read
        // fail or disconnect ack message.
        return;
    }
    
    std::cout << "client " << con << " left the lobby." << std::endl;
    
    const std::string alias = it->second;
    m_connections.erase(it);

    // send user list and signoff message to all clients
    send_to_all(serialize_state());
    send_to_all(encode_message("server",alias+" has left the chat."));
}

void chat_server_handler::on_message(connection_ptr con, message_ptr msg) {
    if (msg->get_opcode() != websocketpp::frame::opcode::TEXT) {
        return;
    }
    
    std::cout << "message from client " << con << ": " << msg->get_payload() << std::endl;
    
    
    
    // check for special command messages
    if (msg->get_payload() == "/help") {
        // print command list
        con->send(encode_message("server","avaliable commands:<br />&nbsp;&nbsp;&nbsp;&nbsp;/help - show this help<br />&nbsp;&nbsp;&nbsp;&nbsp;/alias foo - set alias to foo",false));
        return;
    }
    
    if (msg->get_payload().substr(0,7) == "/alias ") {
        std::string response;
        std::string alias;
        
        if (msg->get_payload().size() == 7) {
            response = "You must enter an alias.";
            con->send(encode_message("server",response));
            return;
        } else {
            alias = msg->get_payload().substr(7);
        }
        
        response = m_connections[con] + " is now known as "+alias;

        // store alias pre-escaped so we don't have to do this replacing every time this
        // user sends a message
        
        // escape JSON characters
        boost::algorithm::replace_all(alias,"\\","\\\\");
        boost::algorithm::replace_all(alias,"\"","\\\"");
        
        // escape HTML characters
        boost::algorithm::replace_all(alias,"&","&amp;");
        boost::algorithm::replace_all(alias,"<","&lt;");
        boost::algorithm::replace_all(alias,">","&gt;");
        
        m_connections[con] = alias;
        
        // set alias
        send_to_all(serialize_state());
        send_to_all(encode_message("server",response));
        return;
    }
    
    // catch other slash commands
    if ((msg->get_payload())[0] == '/') {
        con->send(encode_message("server","unrecognized command"));
        return;
    }
    
    // create JSON message to send based on msg
    send_to_all(encode_message(m_connections[con],msg->get_payload()));
}

void chat_server_handler::http(connection_ptr con)
{
	boost::filesystem::path p(boost::filesystem::current_path());
	p /= (boost::ends_with(con->get_resource(), "/") ? "chat_client.html" : con->get_resource());

	if (boost::filesystem::exists(p)) {
		boost::filesystem::ifstream f(p, std::ios::binary);
		if (!f.fail()) {
			std::string body;
			body.reserve(boost::filesystem::file_size(p));
			body.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
			con->set_body(body);
			con->replace_response_header("Content-Type", websocketpp::http::mime::extension_to_mime(p.extension().string()));
			f.close();
		} else { // including directory or regular file without read permissions
			throw (websocketpp::http::exception("Request for invalid resource",
						websocketpp::http::status_code::FORBIDDEN,
						"Request for invalid resource",
						"<html><head><title>403</title></head><body>Invalid Request</body></html>"));
		}

	} else {
		throw (websocketpp::http::exception("Request for unknown resource",
					websocketpp::http::status_code::NOT_FOUND,
					"Request for unknown resource",
					"<html><head><title>404</title></head><body>File Not Found!</body></html>"));
	}

	return;
}

// {"type":"participants","value":[<participant>,...]}
std::string chat_server_handler::serialize_state() {
    std::stringstream s;
    
    s << "{\"type\":\"participants\",\"value\":[";
    
    std::map<connection_ptr,std::string>::iterator it;
    
    for (it = m_connections.begin(); it != m_connections.end(); it++) {
        s << "\"" << (*it).second << "\"";
        if (++it != m_connections.end()) {
            s << ",";
        }
        it--;
    }
    
    s << "]}";
    
    return s.str();
}

// {"type":"msg","sender":"<sender>","value":"<msg>" }
std::string chat_server_handler::encode_message(std::string sender,std::string msg,bool escape) {
    std::stringstream s;
    
    // escape JSON characters
    boost::algorithm::replace_all(msg,"\\","\\\\");
    boost::algorithm::replace_all(msg,"\"","\\\"");
    
    // escape HTML characters
    if (escape) {
        boost::algorithm::replace_all(msg,"&","&amp;");
        boost::algorithm::replace_all(msg,"<","&lt;");
        boost::algorithm::replace_all(msg,">","&gt;");
    }
    
    s << "{\"type\":\"msg\",\"sender\":\"" << sender 
      << "\",\"value\":\"" << msg << "\"}";
    
    return s.str();
}

std::string chat_server_handler::get_con_id(connection_ptr con) {
    std::stringstream endpoint;
    //endpoint << con->get_endpoint();
    endpoint << con;
    return endpoint.str();
}

void chat_server_handler::send_to_all(std::string data) {
    std::map<connection_ptr,std::string>::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); it++) {
        (*it).first->send(data);
    }
}
