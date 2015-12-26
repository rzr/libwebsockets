/*
 * libwebsockets-test-server - libwebsockets test implementation
 *
 * Copyright (C) 2010-2015 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
#include "test-server.h"

/* dumb_increment protocol */

int
callback_dumb_increment(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512];
	struct per_session_data__dumb_increment *pss =
			(struct per_session_data__dumb_increment *)user;
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	int n, m;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		pss->number = 0;
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		n = sprintf((char *)p, "%d", pss->number++);
		m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
		if (m < n) {
			lwsl_err("ERROR %d writing to di socket\n", n);
			return -1;
		}
		if (close_testing && pss->number == 50) {
			lwsl_info("close tesing limit, closing\n");
			return -1;
 		}
		break;

	case LWS_CALLBACK_RECEIVE:
		if (len < 6)
			break;
		if (strcmp((const char *)in, "reset\n") == 0)
			pss->number = 0;
		break;
	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info(wsi);
		/* you could return non-zero here and kill the connection */
		break;

	/*
	 * this just demonstrates how to handle
	 * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
	 * code and auxiliary data.  You can just not handle it if you don't
	 * have a use for this.
	 */
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		lwsl_notice("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %d\n",
			    len);
		for (n = 0; n < (int)len; n++)
			lwsl_notice(" %d: 0x%02X\n", n,
				    ((unsigned char *)in)[n]);
		break;

	default:
		break;
	}

	return 0;
}
