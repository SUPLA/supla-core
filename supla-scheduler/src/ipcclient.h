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

#ifndef IPCCLIENT_H_
#define IPCCLIENT_H_

#include "supla-scheduler.h"

#define IPC_BUFFER_SIZE 256

class ipc_client  {
private:
	int sfd;
	char buffer[IPC_BUFFER_SIZE];
	int read(void);

	char *ipc_sauth_key;

public:

	ipc_client();
	~ipc_client();
	bool ipc_connect(void);
	bool ipc_disconnect(void);
	bool auth(void);

};

#endif /* IPCCLIENT_H_ */
