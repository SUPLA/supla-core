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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "tools.h"
#include "ipcsocket.h"


typedef struct {

	int sfd;
	struct sockaddr_un saun, fsaun;

}TSuplaIPC_socket;

char *ipc_sauth_key = NULL;
int ipc_shmid = -1;

void ipcsauth_create_key(const char *address) {

	int  a;
	key_t key;
	key = ftok(address, 'S');

	ipc_sauth_key = NULL;

	if((ipc_shmid = shmget(key, IPC_SAUTH_KEY_SIZE, IPC_CREAT|0600)) == -1)
		return;

	if ( (ipc_sauth_key = (char *)shmat(ipc_shmid, 0, 0)) == (char*)-1 )
		ipc_sauth_key = NULL;

	if ( ipc_sauth_key != NULL ) {

		struct timeval tv;
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec);

		for(a=0;a<IPC_SAUTH_KEY_SIZE;a++) {
			ipc_sauth_key[a] = (unsigned char)(rand());

			if ( ipc_sauth_key[a] == 0 || ipc_sauth_key[a] == '\n' )
				ipc_sauth_key[a] = 1;
		}

	}


}

void *ipcsocket_init(const char *address) {

	int sfd;
	TSuplaIPC_socket *ipc;

	if ( address == 0 || strnlen(address, 110) == 0 ) {
		supla_log(LOG_ERR, "IPC unknown address");
	    return 0;
	}

	if ( strnlen(address, 110) > 107 ) {
		supla_log(LOG_ERR, "IPC address too long");
	    return 0;
	}

	if ( ( sfd = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
		supla_log(LOG_ERR, "Can't create ipc socket");
	    return 0;
	}


	if ( st_file_exists(address) )
		unlink(address);

	ipc = malloc(sizeof(TSuplaIPC_socket));
	memset(ipc, 0, sizeof(TSuplaIPC_socket));
	ipc->sfd = -1;

	ipc->saun.sun_family = AF_UNIX;
	strcpy(ipc->saun.sun_path, address);

    if (bind(sfd, (struct sockaddr *)&ipc->saun, sizeof(ipc->saun)) == -1) {
    	free(ipc);
    	close(sfd);
    	supla_log(LOG_ERR, "Can't bind ipc socket");
    	return 0;
    }

	if ( listen(sfd, 5) == -1 ) {
		free(ipc);
		close(sfd);
		supla_log(LOG_ERR, "IPC listen error");
	    return 0;
	} else {
		chmod(address, 0777);
	}

    ipc->sfd = sfd;

    ipcsauth_create_key(address);

	return ipc;
}

int ipcsocket_accept(void *_ipc) {
	assert(_ipc != 0);

	socklen_t fromlen = 0;
	TSuplaIPC_socket *ipc = (TSuplaIPC_socket *)_ipc;
	int client_sd = accept(ipc->sfd, (struct sockaddr *)&ipc->fsaun, &fromlen);

	if ( client_sd != -1 )
		fcntl(client_sd, F_SETFL, O_NONBLOCK);

	return client_sd;
}

void ipcsocket_close(void *_ipc) {

	TSuplaIPC_socket *ipc = (TSuplaIPC_socket *)_ipc;

	assert(_ipc != 0);

    if ( ipc->sfd != -1 ) {
    	 close(ipc->sfd);
    	 ipc->sfd = -1;

    	 if ( st_file_exists(ipc->saun.sun_path) )
    		 unlink(ipc->saun.sun_path);
    }

}

void ipcsocket_free(void *_ipc) {

	assert(_ipc != 0);

	ipcsocket_close(_ipc);
    free(_ipc);

}
