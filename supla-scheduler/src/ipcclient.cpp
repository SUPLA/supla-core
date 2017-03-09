/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "ipcclient.h"
#include "log.h"

#define IPC_SAUTH_KEY_SIZE 16

const char socket_path[] = "/tmp/supla-server-ctrl.sock";
const char hello[] = "SUPLA SERVER CTRL\n";

ipc_client::ipc_client() {

	this->sfd = -1;
	ipc_sauth_key = NULL;

	int   ipc_shmid = -1;
	key_t key;
	key = ftok(socket_path, 'S');

	if((ipc_shmid = shmget(key, IPC_SAUTH_KEY_SIZE, 0)) == -1)
		return;

	if ( (ipc_sauth_key = (char *)shmat(ipc_shmid, 0, 0)) == (char*)-1 )
		ipc_sauth_key = NULL;
}

ipc_client::~ipc_client() {

	ipc_disconnect();
}

int ipc_client::read(void) {

    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(sfd, &set);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

	if ( sfd == -1 )
		return 0;

    memset(buffer, 0, IPC_BUFFER_SIZE);

    int rv = select(sfd + 1, &set, NULL, NULL, &timeout);

    if ( rv == 0 ) {
    	return 0;
    };

    ssize_t len = recv(sfd, buffer, IPC_BUFFER_SIZE, 0);

    if ( len > 0 ) {
    	if ( len >= IPC_BUFFER_SIZE )
    		len = IPC_BUFFER_SIZE;

    	buffer[len] = 0;
    	return len;
    }

    return 0;
}

bool ipc_client::ipc_connect(void) {

	struct sockaddr_un remote;
	int len;

	if ( sfd != -1 )
		return true;

	if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	       return false;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, socket_path);

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sfd, (struct sockaddr *)&remote, len) == -1) {
    	ipc_disconnect();
    	return false;
    }

    if ( read() && strcmp(buffer, hello) == 0 )
    	return true;

    supla_log(LOG_DEBUG, "CONNECTED");

    ipc_disconnect();

	return false;
}

bool ipc_client::ipc_disconnect(void) {

	if ( sfd == -1 )
		return false;

	close(sfd);
	sfd = -1;

	return true;
}

bool ipc_client::auth(void) {

	if ( !ipc_connect() || ipc_sauth_key == NULL )
		return false;

	snprintf(buffer, IPC_BUFFER_SIZE, "SAUTH:");
	memcpy(&buffer[6], ipc_sauth_key, IPC_SAUTH_KEY_SIZE);

	buffer[6+IPC_SAUTH_KEY_SIZE] = '\n';

	send(sfd, buffer, strlen(buffer), 0);

	if ( read() && strcmp(buffer, "AUTH_OK\n") == 0 )
		return true;

	return false;
}
