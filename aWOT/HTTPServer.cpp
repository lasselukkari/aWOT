/*
   Webduino, a simple Arduino web server
   Copyright 2009-2014 Ben Combee, Ran Talbott, Christopher Lee, Martin Lormes
   Francisco M Cuenca-Acuna

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <arduino.h>
#include <HTTPserver.h>

HTTPServer::HTTPServer(const char *urlPrefix) : m_urlPrefix(urlPrefix), m_pushbackDepth(
				0), m_cmdCount(0), m_contentLength(0), m_failureCmd(
				&defaultFailCmd), m_defaultCmd(&defaultFailCmd) {
}

void HTTPServer::setDefaultCommand(Command *cmd) {
	m_defaultCmd = cmd;
}

void HTTPServer::setFailureCommand(Command *cmd) {
	m_failureCmd = cmd;
}

void HTTPServer::addCommand(MethodType type, const char *verb,
		ParamCommand *cmd) {
	if (m_cmdCount < SIZE(m_commands)) {
		m_commands[m_cmdCount].verb = verb;
		m_commands[m_cmdCount].cmd = cmd;
		m_commands[m_cmdCount].type = type;
		m_cmdCount++;
	}
}

void HTTPServer::onGET( const char *verb,
		ParamCommand *cmd) {
	this->addCommand(HTTPServer::GET, verb, cmd);
}

void HTTPServer::onPOST(const char *verb,
		ParamCommand *cmd) {
	this->addCommand(HTTPServer::POST, verb, cmd);
}

void HTTPServer::onPUT(const char *verb,
		ParamCommand *cmd) {
	this->addCommand(HTTPServer::PUT, verb, cmd);
}

void HTTPServer::onDELETE( const char *verb,
		ParamCommand *cmd) {
	this->addCommand(HTTPServer::DELETE, verb, cmd);
}

void HTTPServer::close() {
	this->reset();
}


size_t HTTPServer::write(uint8_t ch) {
	return m_client()->write(ch);
}

size_t HTTPServer::write(const char *str) {
	return m_client()->write(str);
}

size_t HTTPServer::write(const uint8_t *buffer, size_t size) {
	return m_client()->write(buffer, size);
}

size_t HTTPServer::write(const char *buffer, size_t length) {
	return m_client()->write((const uint8_t *) buffer, length);
}

void HTTPServer::writeP(const prog_uchar *data, size_t length) {
	// copy data out of program memory into local storage, write out in
	// chunks of 32 bytes to avoid extra short TCP/IP packets
	uint8_t buffer[32];
	size_t bufferEnd = 0;

	while (length--) {
		if (bufferEnd == 32) {
			m_client()->write(buffer, 32);
			bufferEnd = 0;
		}

		buffer[bufferEnd++] = pgm_read_byte(data++);
	}

	if (bufferEnd > 0)
		m_client()->write(buffer, bufferEnd);
}

void HTTPServer::printP(const prog_uchar *str) {
	// copy data out of program memory into local storage, write out in
	// chunks of 32 bytes to avoid extra short TCP/IP packets
	uint8_t buffer[32];
	size_t bufferEnd = 0;

	while (buffer[bufferEnd++] = pgm_read_byte(str++)) {
		if (bufferEnd == 32) {
			m_client()->write(buffer, 32);
			bufferEnd = 0;
		}
	}

	// write out everything left but trailing NUL
	if (bufferEnd > 1)
		m_client()->write(buffer, bufferEnd - 1);
}

void HTTPServer::printCRLF() {
	m_client()->write((const uint8_t *) "\r\n", 2);
}

bool HTTPServer::dispatchCommand(MethodType requestType, char *verb,
		bool tail_complete) {
	// if there is no URL, i.e. we have a prefix and it's requested without a
	// trailing slash or if the URL is just the slash
	if ((verb[0] == 0) || ((verb[0] == '/') && (verb[1] == 0))) {
		m_defaultCmd(*this, requestType, "", tail_complete);
		return true;
	}
	// if the URL is just a slash followed by a question mark
	// we're looking at the default command with GET parameters passed
	if ((verb[0] == '/') && (verb[1] == '?')) {
		verb += 2; // skip over the "/?" part of the url
		m_defaultCmd(*this, requestType, verb, tail_complete);
		return true;
	}
	// We now know that the URL contains at least one character.  And,
	// if the first character is a slash,  there's more after it.
	if (verb[0] == '/') {
		unsigned char i;
		char *qm_loc;
		int verb_len;
		int qm_offset;

		verb++;


		// Look for a "?" separating the filename part of the URL from the
		// parameters.  If it's not there, compare to the whole URL.
		qm_loc = strchr(verb, '?');
		verb_len = (qm_loc == NULL) ? strlen(verb) : (qm_loc - verb);
		qm_offset = (qm_loc == NULL) ? 0 : 1;

		for (i = 0; i < m_cmdCount; ++i) {

			if (requestType == m_commands[i].type) {

				if (patternMatch(verb, m_commands[i].verb)) {

					char *url_path[WEBDUINO_URL_PATH_COMMAND_LENGTH] = { 0 };
								int part = 0;

								// URL path should be terminated with null byte.
								*(verb + verb_len) = 0;

								// First URL path part is at the start of verb.
								url_path[part++] = verb;
								// Replace all slashes ('/') with a null byte so every part of the URL
								// path is a seperate string. Add every char following a '/' as a new
								// part of the URL, even if that char is a '/' (which will be replaced
								// with a null byte).
								for (char * p = verb; p < verb + verb_len; p++) {
									if (*p == '/') {
										*p = 0;
										url_path[part++] = p + 1;
										// Don't try to assign out of array bounds.
										if (part == WEBDUINO_URL_PATH_COMMAND_LENGTH)
											break;
									}
								}

								// Skip over the "verb" part of the URL (and the question
								// mark, if present) when passing it to the "action" routine
					m_commands[i].cmd(*this, requestType, url_path,
							verb + verb_len + qm_offset, tail_complete);
					return true;
				}
			}

		}

	}
	return false;
}

// processConnection with a default buffer
void HTTPServer::processConnection(ServerInterface *serverInterface) {
	char request[WEBDUINO_DEFAULT_REQUEST_LENGTH];
	int request_len = WEBDUINO_DEFAULT_REQUEST_LENGTH;
	processConnection(serverInterface, request, &request_len);
}

void HTTPServer::processConnection(ServerInterface *serverInterface, char *buff, int *bufflen) {
	m_server_obj = serverInterface;
	int urlPrefixLen = strlen(m_urlPrefix);

	m_client_obj = m_server()->available();

	if (m_client_obj != NULL) {

		m_readingContent = false;
		buff[0] = 0;
		MethodType requestType = INVALID;
#if WEBDUINO_SERIAL_DEBUGGING > 1
		Serial.println("*** checking request ***");
#endif
		getRequest(requestType, buff, bufflen);
#if WEBDUINO_SERIAL_DEBUGGING > 1
		Serial.print("*** requestType = ");
		Serial.print((int)requestType);
		Serial.print(", request = \"");
		Serial.print(buff);
		Serial.println("\" ***");
#endif

		// don't even look further at invalid requests.
		// this is done to prevent Webduino from hanging
		// - when there are illegal requests,
		// - when someone contacts it through telnet rather than proper HTTP,
		// - etc.
		if (requestType != INVALID) {
			processHeaders();
#if WEBDUINO_SERIAL_DEBUGGING > 1
			Serial.println("*** headers complete ***");
#endif
			if (strcmp(buff, "/robots.txt") == 0) {
				noRobots(requestType);
			} else if (strcmp(buff, "/favicon.ico") == 0) {
				favicon(requestType);
			}
		}
		// Only try to dispatch command if request type and prefix are correct.
		// Fix by quarencia.
		if (requestType == INVALID
				|| strncmp(buff, m_urlPrefix, urlPrefixLen) != 0) {
			m_failureCmd(*this, requestType, buff, (*bufflen) >= 0);
		} else if (!dispatchCommand(requestType, buff + urlPrefixLen,
				(*bufflen) >= 0)) {
			m_failureCmd(*this, requestType, buff, (*bufflen) >= 0);
		}

#if WEBDUINO_SERIAL_DEBUGGING > 1
		Serial.println("*** stopping connection ***");
#endif
		reset();

	}
}

bool HTTPServer::checkCredentials(const char authCredentials[45]) {
	char basic[7] = "Basic ";
	if ((0 == strncmp(m_authCredentials, basic, 6))
			&& (0 == strcmp(authCredentials, m_authCredentials + 6)))
		return true;
	return false;
}

void HTTPServer::httpFail() {
	P(failMsg) = "HTTP/1.0 400 Bad Request"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Content-Type: text/html"
	CRLF
	CRLF WEBDUINO_FAIL_MESSAGE;

	printP (failMsg);
}

void HTTPServer::defaultFailCmd(HTTPServer &server,
		HTTPServer::MethodType type, char *url_tail, bool tail_complete) {
	server.httpFail();
}

void HTTPServer::noRobots(MethodType type) {
	httpSuccess("text/plain");
	if (type != HEAD) {
		P(allowNoneMsg) = "User-agent: *"
		CRLF
		"Disallow: /"
		CRLF;
		printP (allowNoneMsg);
	}
}

void HTTPServer::favicon(MethodType type) {
	httpSuccess("image/x-icon", "Cache-Control: max-age=31536000\r\n");
	if (type != HEAD) {
		P (faviconIco) = WEBDUINO_FAVICON_DATA;
		writeP(faviconIco, sizeof(faviconIco));
	}
}

void HTTPServer::httpUnauthorized() {
	P(failMsg) = "HTTP/1.0 401 Authorization Required"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Content-Type: text/html"
	CRLF
	"WWW-Authenticate: Basic realm=\""
	WEBDUINO_AUTH_REALM
	"\""
	CRLF
	CRLF WEBDUINO_AUTH_MESSAGE;

	printP (failMsg);
}

void HTTPServer::httpNotFound() {
	P(failMsg) = "HTTP/1.0 404 Not Found"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Content-Type: text/html"
	CRLF
	CRLF WEBDUINO_FAIL_MESSAGE;

	printP (failMsg);
}


void HTTPServer::httpServerError() {
	P(failMsg) = "HTTP/1.0 500 Internal Server Error"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Content-Type: text/html"
	CRLF
	CRLF WEBDUINO_SERVER_ERROR_MESSAGE;

	printP (failMsg);
}

void HTTPServer::httpSuccess(const char *contentType, const char *extraHeaders) {
	P(successMsg1) = "HTTP/1.0 200 OK"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Access-Control-Allow-Origin: *"
	CRLF
	"Pragma: no-cache"
	CRLF
	"Content-Type: ";

	printP (successMsg1);
	print(contentType);
	printCRLF();
	if (extraHeaders)
	print(extraHeaders);
	printCRLF();
}

void HTTPServer::httpNoContent() {
	P(noContentMsg) =
	"HTTP/1.0 204 No Content" CRLF
	WEBDUINO_SERVER_HEADER
	CRLF
	CRLF;

	printP(noContentMsg);
}

void HTTPServer::httpCreated(const char *contentType, const char *extraHeaders) {
	P(successMsg1) = "HTTP/1.0 201 Created"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Access-Control-Allow-Origin: *"
	CRLF
	"Pragma: no-cache"
	CRLF
	"Content-Type: ";

	printP (successMsg1);
	print(contentType);
	printCRLF();
	if (extraHeaders)
	print(extraHeaders);
	printCRLF();
}

void HTTPServer::httpSeeOther(const char *otherURL) {
	P(seeOtherMsg) = "HTTP/1.0 303 See Other"
	CRLF
	WEBDUINO_SERVER_HEADER
	"Location: ";

	printP (seeOtherMsg);
	print(otherURL);
	printCRLF();
	printCRLF();
}

int HTTPServer::read() {
	if (m_client() == NULL)
		return -1;

	if (m_pushbackDepth == 0) {
		unsigned long timeoutTime = millis() + WEBDUINO_READ_TIMEOUT_IN_MS;

		while (m_client()->connected()) {
			// stop reading the socket early if we get to content-length
			// characters in the POST.  This is because some clients leave
			// the socket open because they assume HTTP keep-alive.
			if (m_readingContent) {
				if (m_contentLength == 0) {
#if WEBDUINO_SERIAL_DEBUGGING > 1
					Serial.println("\n*** End of content, terminating connection");
#endif
					return -1;
				}
			}

			int ch = m_client()->read();

			// if we get a character, return it, otherwise continue in while
			// loop, checking connection status
			if (ch != -1) {
				// count character against content-length
				if (m_readingContent) {
					--m_contentLength;
				}

#if WEBDUINO_SERIAL_DEBUGGING
				if (ch == '\r')
				Serial.print("<CR>");
				else if (ch == '\n')
				Serial.println("<LF>");
				else
				Serial.print((char)ch);
#endif

				return ch;
			} else {
				unsigned long now = millis();
				if (now > timeoutTime) {
					// connection timed out, destroy client, return EOF
#if WEBDUINO_SERIAL_DEBUGGING
					Serial.println("*** Connection timed out");
#endif

					reset();
					return -1;
				}
			}
		}

		// connection lost, return EOF
#if WEBDUINO_SERIAL_DEBUGGING
		Serial.println("*** Connection lost");
#endif
		return -1;
	} else
		return m_pushback[--m_pushbackDepth];
}

int HTTPServer::peek() {
	int c = read();
	push(c);
	return c;
}

int HTTPServer::available() {
	return m_client()->available();
}

void HTTPServer::flush() {
	m_client()->flush();
}

void HTTPServer::push(int ch) {
	// don't allow pushing EOF
	if (ch == -1)
		return;

	m_pushback[m_pushbackDepth++] = ch;
	// can't raise error here, so just replace last char over and over
	if (m_pushbackDepth == SIZE(m_pushback))
		m_pushbackDepth = SIZE(m_pushback) - 1;
}

void HTTPServer::reset() {
	m_pushbackDepth = 0;
	m_client()->flush();
	m_client()->stop();
}

bool HTTPServer::expect(const char *str) {
	const char *curr = str;
	while (*curr != 0) {
		int ch = read();
		if (ch != *curr++) {
			// push back ch and the characters we accepted
			push(ch);
			while (--curr != str)
				push(curr[-1]);
			return false;
		}
	}
	return true;
}

bool HTTPServer::readInt(int &number) {
	bool negate = false;
	bool gotNumber = false;
	int ch;
	number = 0;

	// absorb whitespace
	do {
		ch = read();
	} while (ch == ' ' || ch == '\t');

	// check for leading minus sign
	if (ch == '-') {
		negate = true;
		ch = read();
	}

	// read digits to update number, exit when we find non-digit
	while (ch >= '0' && ch <= '9') {
		gotNumber = true;
		number = number * 10 + ch - '0';
		ch = read();
	}

	push(ch);
	if (negate)
		number = -number;
	return gotNumber;
}

void HTTPServer::readHeader(char *value, int valueLen) {
	int ch;
	memset(value, 0, valueLen);
	--valueLen;

	// absorb whitespace
	do {
		ch = read();
	} while (ch == ' ' || ch == '\t');

	// read HTTP of line
	do {
		if (valueLen > 1) {
			*value++ = ch;
			--valueLen;

		}
		ch = read();
	} while (ch != '\r');
	push(ch);
}


bool HTTPServer::readPOSTparam(char *name, int nameLen, char *value,
		int valueLen) {
	// assume name is at current place in stream
	int ch;
	// to not to miss the last parameter
	bool foundSomething = false;

	// clear out name and value so they'll be NUL terminated
	memset(name, 0, nameLen);
	memset(value, 0, valueLen);

	// decrement length so we don't write into NUL terminator
	--nameLen;
	--valueLen;

	while ((ch = read()) != -1) {
		foundSomething = true;
		if (ch == '+') {
			ch = ' ';
		} else if (ch == '=') {
			/* that's end of name, so switch to storing in value */
			nameLen = 0;
			continue;
		} else if (ch == '&') {
			/* that's end of pair, go away */
			return true;
		} else if (ch == '%') {
			/* handle URL encoded characters by converting back to original form */
			int ch1 = read();
			int ch2 = read();
			if (ch1 == -1 || ch2 == -1)
				return false;
			char hex[3] = { ch1, ch2, 0 };
			ch = strtoul(hex, NULL, 16);
		}

		// output the new character into the appropriate buffer or drop it if
		// there's no room in either one.  This code will malfunction in the
		// case where the parameter name is too long to fit into the name buffer,
		// but in that case, it will just overflow into the value buffer so
		// there's no harm.
		if (nameLen > 0) {
			*name++ = ch;
			--nameLen;
		} else if (valueLen > 0) {
			*value++ = ch;
			--valueLen;
		}
	}

	if (foundSomething) {
		// if we get here, we have one last parameter to serve
		return true;
	} else {
		// if we get here, we hit the end-of-file, so POST is over and there
		// are no more parameters
		return false;
	}
}

/* Retrieve a parameter that was encoded as part of the URL, stored in
 * the buffer pointed to by *tail.  tail is updated to point just past
 * the last character read from the buffer. */
URLPARAM_RESULT HTTPServer::nextURLparam(char **tail, char *name, int nameLen,
		char *value, int valueLen) {
	// assume name is at current place in stream
	char ch, hex[3];
	URLPARAM_RESULT result = URLPARAM_OK;
	char *s = *tail;
	bool keep_scanning = true;
	bool need_value = true;

	// clear out name and value so they'll be NUL terminated
	memset(name, 0, nameLen);
	memset(value, 0, valueLen);

	if (*s == 0)
		return URLPARAM_EOS;
	// Read the keyword name
	while (keep_scanning) {
		ch = *s++;
		switch (ch) {
		case 0:
			s--; // Back up to point to terminating NUL
			// Fall through to "stop the scan" code
		case '&':
			/* that's end of pair, go away */
			keep_scanning = false;
			need_value = false;
			break;
		case '+':
			ch = ' ';
			break;
		case '%':
			/* handle URL encoded characters by converting back
			 * to original form */
			if ((hex[0] = *s++) == 0) {
				s--; // Back up to NUL
				keep_scanning = false;
				need_value = false;
			} else {
				if ((hex[1] = *s++) == 0) {
					s--; // Back up to NUL
					keep_scanning = false;
					need_value = false;
				} else {
					hex[2] = 0;
					ch = strtoul(hex, NULL, 16);
				}
			}
			break;
		case '=':
			/* that's end of name, so switch to storing in value */
			keep_scanning = false;
			break;
		}

		// check against 1 so we don't overwrite the final NUL
		if (keep_scanning && (nameLen > 1)) {
			*name++ = ch;
			--nameLen;
		} else if (keep_scanning)
			result = URLPARAM_NAME_OFLO;
	}

	if (need_value && (*s != 0)) {
		keep_scanning = true;
		while (keep_scanning) {
			ch = *s++;
			switch (ch) {
			case 0:
				s--; // Back up to point to terminating NUL
					 // Fall through to "stop the scan" code
			case '&':
				/* that's end of pair, go away */
				keep_scanning = false;
				need_value = false;
				break;
			case '+':
				ch = ' ';
				break;
			case '%':
				/* handle URL encoded characters by converting back to original form */
				if ((hex[0] = *s++) == 0) {
					s--; // Back up to NUL
					keep_scanning = false;
					need_value = false;
				} else {
					if ((hex[1] = *s++) == 0) {
						s--; // Back up to NUL
						keep_scanning = false;
						need_value = false;
					} else {
						hex[2] = 0;
						ch = strtoul(hex, NULL, 16);
					}

				}
				break;
			}

			// check against 1 so we don't overwrite the final NUL
			if (keep_scanning && (valueLen > 1)) {
				*value++ = ch;
				--valueLen;
			} else if (keep_scanning)
				result =
						(result == URLPARAM_OK) ?
								URLPARAM_VALUE_OFLO : URLPARAM_BOTH_OFLO;
		}
	}
	*tail = s;
	return result;
}

// Read and parse the first line of the request header.
// The "command" (GET/HEAD/POST) is translated into a numeric value in type.
// The URL is stored in request,  up to the length passed in length
// NOTE 1: length must include one byte for the terminating NUL.
// NOTE 2: request is NOT checked for NULL,  nor length for a value < 1.
// Reading stops when the code encounters a space, CR, or LF.  If the HTTP
// version was supplied by the client,  it will still be waiting in the input
// stream when we exit.
//
// On return, length contains the amount of space left in request.  If it's
// less than 0,  the URL was longer than the buffer,  and part of it had to
// be discarded.

void HTTPServer::getRequest(HTTPServer::MethodType &type, char *request,
		int *length) {
	--*length; // save room for NUL

	type = INVALID;

	// store the HTTP method line of the request
	if (expect("GET "))
		type = GET;
	else if (expect("HEAD "))
		type = HEAD;
	else if (expect("POST "))
		type = POST;
	else if (expect("PUT "))
		type = PUT;
	else if (expect("DELETE "))
		type = DELETE;
	else if (expect("PATCH "))
		type = PATCH;

	// if it doesn't start with any of those, we have an unknown method
	// so just get out of here
	else
		return;

	int ch;
	while ((ch = read()) != -1) {
		// stop storing at first space or end of line
		if (ch == ' ' || ch == '\n' || ch == '\r') {
			break;
		}
		if (*length > 0) {
			*request = ch;
			++request;
		}
		--*length;
	}
	// NUL terminate
	*request = 0;
}

void HTTPServer::processHeaders() {
	// look for three things: the Content-Length header, the Authorization
	// header, and the double-CRLF that ends the headers.

	// empty the m_authCredentials before every run of this function.
	// otherwise users who don't send an Authorization header would be treated
	// like the last user who tried to authenticate (possibly successful)
	m_authCredentials[0] = 0;

	while (1) {
		if (expect("Content-Length:")) {
			readInt(m_contentLength);
#if WEBDUINO_SERIAL_DEBUGGING > 1
			Serial.print("\n*** got Content-Length of ");
			Serial.print(m_contentLength);
			Serial.print(" ***");
#endif
			continue;
		}

		if (expect("Authorization:")) {
			readHeader(m_authCredentials, 51);
#if WEBDUINO_SERIAL_DEBUGGING > 1
			Serial.print("\n*** got Authorization: of ");
			Serial.print(m_authCredentials);
			Serial.print(" ***");
#endif
			continue;
		}

		if (expect(CRLF CRLF)) {
			m_readingContent = true;
			return;
		}

		// no expect checks hit, so just absorb a character and try again
		if (read() == -1) {
			return;
		}
	}
}

bool HTTPServer::patternMatch(const char *text, const char *pattern) {

	boolean match = false;
	int i = 0;
	int j = 0;

	while (pattern[i] && text[j]) {
		if (pattern[i] == ':') {
			while (pattern[i] && pattern[i] != '/') {
				i++;
			}

			while (text[j] && text[j] != '/') {
				j++;
			}

			match = true;
		}
		else if (pattern[i] == text[j]) {
			j++;
			i++;
			match = true;
		}
		else {
			match = false;
			break;
		}
	}

	if( match && !pattern[i] && text[j]=='/' && !text[j+1]){
		match = true;
	}
	else if(pattern[i] || text[j]){
		match = false;
	}

	return match;
}
