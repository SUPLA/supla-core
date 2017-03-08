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

#ifndef QUEUE_H_
#define QUEUE_H_

#include "database.h"


class queue  {
private:
	void *workers_thread_arr;
	int max_workers;
	s_exec_t *s_exec;
protected:
	database *db;
public:

	queue();
    ~queue();

    void load(void);
    void set_overdue_result(void);
    void set_zombie_result(void);

};

void queue_loop(void *ssd, void *sthread);


#endif /* QUEUE_H_ */
