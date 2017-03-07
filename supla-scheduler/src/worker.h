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

#ifndef WORKER_H_
#define WORKER_H_

#include "database.h"


class s_worker {
private:
	s_exec_t s_exec;
	database *db;
public:

	s_worker(s_exec_t *s_exec);
	~s_worker();
	void execute(void *sthread);
};



#endif /* QUEUE_H_ */
