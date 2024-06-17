/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

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

#include "analyzer/abstract_data_analyzer.h"

#ifndef DATA_ANALYZER_FACTORY_H_
#define DATA_ANALYZER_FACTORY_H_

class supla_data_analyzer_factory {
 public:
  supla_data_analyzer_factory(void);
  virtual ~supla_data_analyzer_factory(void);

  static supla_abstract_data_analyzer *new_analyzer(int channel_id, int func);
};

#endif /*DATA_ANALYZER_FACTORY_H_*/
