#include "cyberspace.hpp"

// TODO proper without delay
int bcd_cyberspace::waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
	struct timeval timeout;
	int rc;
	fd_set fd;
	fd_set *writefd = NULL;
	fd_set *readfd = NULL;
	int dir;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);

	FD_SET(socket_fd, &fd);

	/* now make sure we wait in the correct direction */
	dir = libssh2_session_block_directions(session);

	if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
		readfd = &fd;

	if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		writefd = &fd;

	rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

	return rc;
}

void bcd_cyberspace::_lcd_adjust_draw_offset(ssize16 msg_area_dimensions, const char *m, ssize16 stat_area_dimensions, const char *s, int16_t *o) {
	const font &font = Bm437_Acer_VGA_8x8_FON;
	int16_t msh = font.measure_text(msg_area_dimensions, m).height;
	int16_t ssh = font.measure_text(stat_area_dimensions, s).height;
	*o += msh > ssh ? msh : ssh; 
}

cyberspace_err_t bcd_cyberspace::_establish_ssh_channel(LIBSSH2_SESSION **session, LIBSSH2_CHANNEL **channel, int *sock) {
	int res;
	struct sockaddr_in sin;

	ESP_LOGI(TAG_CYBERSPACE, "libssh2_version is %s", LIBSSH2_VERSION);
	res = libssh2_init(0);
	if(res) {
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_SSH_INIT_FAIL);
		return CYBERSPACE_SSH_INIT_FAIL;
	}

	ESP_LOGI(TAG_CYBERSPACE, "CYBERSPACE_SSH_HOST=%s", CYBERSPACE_SSH_HOST);
	ESP_LOGI(TAG_CYBERSPACE, "CYBERSPACE_SSH_PORT=%d", CYBERSPACE_SSH_PORT);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(CYBERSPACE_SSH_PORT);
	sin.sin_addr.s_addr = inet_addr(CYBERSPACE_SSH_HOST);
	if (sin.sin_addr.s_addr == 0xffffffff) {
		// A domain name was given, need to resolve it
		struct hostent *hp = gethostbyname(CYBERSPACE_SSH_HOST);
		if (hp == NULL) {
			_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_GETHOSTBYNAME_FAIL);
			return CYBERSPACE_GETHOSTBYNAME_FAIL;
		}
		struct ip4_addr *ip4_addr = (struct ip4_addr *)hp->h_addr;
		sin.sin_addr.s_addr = ip4_addr->addr;
	}
	
	// Next we open the socket and connect to it
	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if(*sock == -1) {
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_CREATE_SOCKET_FAIL);
		return CYBERSPACE_CREATE_SOCKET_FAIL;
	}
	if(connect(*sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_CONNECT_FAIL);
		return CYBERSPACE_CONNECT_FAIL;
	}

	// Create session
	//
	// Once the socket is open, we can create a session and start it up. This 
	// will trade welcome banners, exchange keys and setup crypto, compression 
	// and MAC layers.
	*session = libssh2_session_init();
	if(!(*session)) {
		// Failed to initialise session
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_SSH_SESSION_FAIL);
		return CYBERSPACE_SSH_SESSION_FAIL;
	}
	res = libssh2_session_handshake(*session, *sock);
	if(res) {
		// Handshake failed
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_SSH_HANDSHAKE_FAIL);
		return CYBERSPACE_SSH_HANDSHAKE_FAIL;
	} 

	// Authenticate
	//
	// Next we authenticate either by username / password or with a private / 
	// public keypair, depending on the setting in menuconfig.
#if CONFIG_CYBERSPACE_AUTH_PASS
	/* We could authenticate via password */
	if(libssh2_userauth_password(*session, CYBERSPACE_SSH_USER, CYBERSPACE_SSH_PASSWORD)) {
		char *error_message = NULL;
		libssh2_session_last_error(*session, &error_message, NULL, 0);
		ESP_LOGD(TAG_CYBERSPACE, "Authentication by password failed: %s (username : [%s])", error_message, CYBERSPACE_SSH_USER);
		free(error_message);
		
		_terminate_ssh_session(*session, *sock, CYBERSPACE_SSH_AUTH_FAIL);
		return CYBERSPACE_SSH_AUTH_FAIL;
	}
#else //CONFIG_CYBERSPACE_AUTH_PASS
	// Authenticate via privatekey
	char publickey[64];
	char privatekey[64];
	strcpy(publickey, "/spiffs/bcd_id_rsa.pub");
	strcpy(privatekey, "/spiffs/bcd_id_rsa");
	if(libssh2_userauth_publickey_fromfile(*session, CYBERSPACE_SSH_USER, publickey, privatekey, NULL)) {
		char *error_message = NULL;
		libssh2_session_last_error(*session, &error_message, NULL, 0);
		ESP_LOGD(TAG_CYBERSPACE, "Authentication by privatekey failed: %s.", error_message);
		free(error_message);

		// Close session and terminate task
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_SSH_AUTH_FAIL);
		return CYBERSPACE_SSH_AUTH_FAIL;
	}
#endif // CONFIG_CYBERSPACE_AUTH_PASS

	// Open a channel
	//
	// The channel is set to non blocking. Note that receiving an EAGAIN means
	// that no content was received yet and thus the channel cannot yet be
	// established. So we just try to establish the channel until we get the
	// required information or an error occures.
	while((*channel = libssh2_channel_open_session(*session)) == NULL &&
		libssh2_session_last_error(*session, NULL, NULL, 0) ==
		LIBSSH2_ERROR_EAGAIN) {
		waitsocket(*sock, *session);
	}
	if(channel == NULL) {
		// If the channel is NULL, an error occured.
		_terminate_ssh_session(*channel, *session, *sock, CYBERSPACE_SSH_CHANNEL_OPEN_FAIL);
		return CYBERSPACE_SSH_CHANNEL_OPEN_FAIL;
	}
	libssh2_session_set_blocking(*session, 0);

	return CYBERSPACE_OK;
}