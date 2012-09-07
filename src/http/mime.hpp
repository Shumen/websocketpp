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

#ifndef HTTP_MIME_HPP
#define HTTP_MIME_HPP

#include <map>
#include <string>
#include <utility> // std::pair

namespace websocketpp {
namespace http {
namespace mime {
	typedef std::map<std::string, std::string> mime_type;
	typedef mime_type::value_type mime_pair;

	// this is to initialize mime_types only without exposure to outside
	static const mime_pair pairs[] = {
		std::make_pair(".html", "text/html"),
		std::make_pair(".htm", "text/html"),
		std::make_pair(".css", "text/css"),
		std::make_pair(".js", "text/javascript"),
		std::make_pair(".jpg", "image/jpeg"),
		std::make_pair(".png", "image/png"),
		std::make_pair(".gif", "image/gif"),
		std::make_pair(".ico", "image/x-icon"),
		std::make_pair(".json", "application/json"),
		std::make_pair(".xhtml", "application/xhtml+xml"),
		//std::make_pair(".binary", "application/octet-stream"), // add other
	};

	const mime_type mime_types(pairs, pairs + sizeof(pairs)/sizeof(pairs[0]));

	/// Convert a file extension into a MIME type.
	inline const std::string& extension_to_mime(const std::string& extension) {
		static const std::string mime_default("text/plain");
		mime_type::const_iterator it = mime_types.find(extension);
		if (it != mime_types.end())
			return it->second;
		else
			return mime_default;
	}
} // namespace mime
} // namespace http
} // namespace websocketpp

#endif // HTTP_MIME_HPP
