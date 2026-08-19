// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPCSOCKET_H_
#define IPCSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_SAUTH_KEY_SIZE 16

extern char *ipc_sauth_key;

void *ipcsocket_init(const char *address);
void ipcsocket_close(void *ipc);
void ipcsocket_free(void *ipc);
int ipcsocket_accept(void *ipc);

#ifdef __cplusplus
}
#endif

#endif /* IPCSOCKET_H_ */
