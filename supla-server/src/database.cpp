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

#include <my_global.h>
#include <mysql.h>
#include <string.h>


#include "database.h"
#include "svrcfg.h"
#include "log.h"
#include "tools.h"

char *database::cfg_get_host(void) {

	return scfg_string(CFG_MYSQL_HOST);
}

char *database::cfg_get_user(void) {

	return scfg_string(CFG_MYSQL_USER);
}
char *database::cfg_get_password(void) {

	return scfg_string(CFG_MYSQL_PASSWORD);
}
char *database::cfg_get_database(void) {

	return scfg_string(CFG_MYSQL_DB);
}
int database::cfg_get_port(void) {

	return scfg_int(CFG_MYSQL_PORT);
}

bool database::auth(const char *query, int ID, char *_PWD, int _PWD_HEXSIZE, int *UserID, bool *is_enabled) {

	if ( _mysql == NULL
			|| ID == 0
			|| strnlen(_PWD, 64) < 1 )
		return false;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	char PWD[_PWD_HEXSIZE];
	st_str2hex(PWD, _PWD, 64);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ID;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)PWD;
	pbind[1].buffer_length = strnlen(PWD, 64);

	int _is_enabled = 0;

	int __ID = 0;
	MYSQL_STMT *stmt;
	if ( !stmt_get_int((void**)&stmt, &__ID, UserID, &_is_enabled, NULL, query, pbind, 2) ) {
		__ID = 0;
		_is_enabled = 0;
	}

	*is_enabled = _is_enabled == 1;

	return __ID != 0;

}

bool database::location_auth(int LocationID, char *LocationPWD, int *UserID, bool *is_enabled, int *limit_iodev) {

	if ( LocationID == 0 )
		return false;

	return auth("SELECT id, user_id, enabled FROM `supla_location` WHERE id = ? AND password = unhex(?)",
			    LocationID,
			    LocationPWD,
			    SUPLA_LOCATION_PWDHEX_MAXSIZE,
			    UserID,
			    is_enabled);
}

bool database::accessid_auth(int AccessID, char *AccessIDpwd, int *UserID, bool *is_enabled) {

	if ( AccessID == 0 )
		return false;

	return auth("SELECT id, user_id, enabled FROM `supla_accessid` WHERE id = ? AND password = unhex(?)",
				AccessID,
				AccessIDpwd,
				SUPLA_ACCESSID_PWDHEX_MAXSIZE,
			    UserID,
			    is_enabled);

}

int database::get_user_id_by_email(const char Email[SUPLA_EMAIL_MAXSIZE]) {

	if ( _mysql == NULL || strnlen(Email, SUPLA_EMAIL_MAXSIZE) == 0 )
		return 0;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	char EmailHEX[SUPLA_EMAILHEX_MAXSIZE];
	memset(EmailHEX, 0, SUPLA_EMAILHEX_MAXSIZE);

	st_str2hex(EmailHEX, Email, SUPLA_EMAILHEX_MAXSIZE);

	pbind[0].buffer_type= MYSQL_TYPE_STRING;
	pbind[0].buffer= (char *)EmailHEX;
	pbind[0].buffer_length= strnlen(EmailHEX, SUPLA_EMAILHEX_MAXSIZE);

	int UserID = 0;
	MYSQL_STMT *stmt;

	if ( stmt_get_int((void**)&stmt, &UserID, NULL, NULL, NULL, "SELECT id FROM supla_user WHERE email = unhex(?)", pbind, 1) ) {
        return UserID;
	}


	return 0;
}

bool database::get_authkey_hash(int ID, char *buffer, int buffer_size, bool *is_null, const char *sql) {

	MYSQL_STMT *stmt = NULL;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ID;

	bool result = false;

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		my_bool       _is_null = false;
		unsigned long size = 0;

		MYSQL_BIND rbind[1];
		memset(rbind, 0, sizeof(rbind));

		rbind[0].buffer_type= MYSQL_TYPE_STRING;
		rbind[0].buffer= buffer;
		rbind[0].buffer_length =  buffer_size;
		rbind[0].length = &size;
		rbind[0].is_null= &_is_null;

		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0
				 && !mysql_stmt_fetch(stmt)
				 && size <= buffer_size ) {

				buffer[size] = 0;
				*is_null = _is_null > 0;

				result = true;
			}

		}

		mysql_stmt_close(stmt);
	}

	return result;

}

bool database::client_get_authkey_hash(int ID, char *buffer, int buffer_size, bool *is_null) {

	return get_authkey_hash(ID, buffer, buffer_size, is_null, "SELECT auth_key FROM supla_client WHERE id = ?");

}

bool database::client_authkey_auth(const char GUID[SUPLA_GUID_SIZE], const char Email[SUPLA_EMAIL_MAXSIZE], const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID) {

	if ( _mysql == NULL ) {
		return false;
	}

	int _UserID = get_user_id_by_email(Email);

	if ( _UserID == 0 )
		return false;

	int ClientID = get_client_id(_UserID, GUID);

	if ( ClientID == 0 ) { // Yes. When client not exists then is authorized
		*UserID = _UserID;
		return true;
	};

	bool is_null = false;
	char ClientAuthKeyHash[BCRYPT_HASH_MAXSIZE];
	memset(ClientAuthKeyHash, 0, BCRYPT_HASH_MAXSIZE);

	if ( !client_get_authkey_hash(ClientID, ClientAuthKeyHash, BCRYPT_HASH_MAXSIZE, &is_null) ) {
		return false;
	}

	if ( is_null ) { // Yes. When is null then is authorized
		*UserID = _UserID;
		return true;
	}

	char AuthKeyHEX[SUPLA_AUTHKEY_HEXSIZE];
	memset(AuthKeyHEX, 0, SUPLA_AUTHKEY_HEXSIZE);

	st_authkey2hex(AuthKeyHEX, AuthKey);

	if ( st_bcrypt_check(AuthKeyHEX, ClientAuthKeyHash, strnlen(ClientAuthKeyHash, BCRYPT_HASH_MAXSIZE)) ) {

		*UserID = _UserID;
		return true;
	}


	return false;
}

int database::get_device_id(const char GUID[SUPLA_GUID_SIZE], int *location_id, int *original_location_id, bool *is_enabled) {

	if ( _mysql == NULL )
		return false;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	char GUIDHEX[SUPLA_GUID_HEXSIZE];
	st_guid2hex(GUIDHEX, GUID);

	pbind[0].buffer_type= MYSQL_TYPE_STRING;
	pbind[0].buffer= (char *)GUIDHEX;
	pbind[0].buffer_length= SUPLA_GUID_SIZE*2;

	int _is_enabled = 0;

	MYSQL_STMT *stmt;
	int dev_id;
	if ( stmt_get_int((void**)&stmt, &dev_id, location_id, original_location_id, &_is_enabled, "SELECT id, location_id, original_location_id, CAST(`enabled` AS unsigned integer) `enabled` FROM supla_iodevice WHERE guid = unhex(?)", pbind, 1) ) {
		*is_enabled = _is_enabled == 1;
        return dev_id;
	}

	*is_enabled = _is_enabled == 1;
	return 0;

}

bool database::on_newdevice(int DeviceID) {

	char sql[51];
	snprintf(sql, 50, "CALL `supla_on_newdevice`(%i)", DeviceID);


	return query(sql) != 0;

}

bool database::on_channeladded(int DeviceID, int ChannelID) {

	char sql[51];
	snprintf(sql, 50, "CALL `supla_on_channeladded`(%i, %i)", DeviceID, ChannelID);


	return query(sql) != 0;
}


int database::get_device_count(int UserID) {
    return get_count(UserID, "SELECT COUNT(*) FROM supla_iodevice WHERE user_id = ?");
}

int database::add_device(int *LocationID, const char GUID[SUPLA_GUID_SIZE], const char Name[SUPLA_DEVICE_NAME_MAXSIZE],
		                   unsigned int ipv4, char softver[SUPLA_SOFTVER_MAXSIZE], int proto_version,
		                   int UserID, bool *new_device, bool *is_enabled, int *Limit) {

	int _LocationID = 0, _OriginalLocationID = 0;
	bool _is_enabled;

	if ( is_enabled == NULL )
		is_enabled = & _is_enabled;

	*is_enabled = true;

	int device_id = get_device_id(GUID, &_LocationID, &_OriginalLocationID, is_enabled);

	if ( device_id != 0
			&& is_enabled
			&& *is_enabled == false )
		return 0;


	char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];

	st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

	if ( device_id == 0 ) {


		//In this case (without db locks) it's possible to over the limit by 1 but it's not the problem
		(*Limit) -= get_device_count(UserID);


		if ( *Limit <= 0 )
			return 0;

		MYSQL_BIND pbind[8];
		memset(pbind, 0, sizeof(pbind));

		char GUIDHEX[SUPLA_GUID_HEXSIZE];
		st_guid2hex(GUIDHEX, GUID);

		pbind[0].buffer_type= MYSQL_TYPE_LONG;
		pbind[0].buffer= (char *)LocationID;

		pbind[1].buffer_type= MYSQL_TYPE_STRING;
		pbind[1].buffer= (char *)NameHEX;
		pbind[1].buffer_length = strnlen(NameHEX, 256);

		pbind[2].buffer_type= MYSQL_TYPE_LONG;
		pbind[2].buffer= (char *)&UserID;

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&ipv4;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&ipv4;

		pbind[5].buffer_type= MYSQL_TYPE_STRING;
		pbind[5].buffer= (char *)GUIDHEX;
		pbind[5].buffer_length= (SUPLA_GUID_SIZE*2);

		pbind[6].buffer_type= MYSQL_TYPE_STRING;
		pbind[6].buffer= (char *)softver;
		pbind[6].buffer_length = strnlen(softver, 32);

		pbind[7].buffer_type= MYSQL_TYPE_LONG;
		pbind[7].buffer= (char *)&proto_version;

		const char sql[] = "INSERT INTO `supla_iodevice`(`location_id`, `original_location_id`, `name`, `enabled`, `reg_date`, `last_connected`, `user_id`, `reg_ipv4`, `last_ipv4`, `guid`, `software_version`, `protocol_version`) VALUES (?,NULL,unhex(?),1,NOW(),NOW(),?,?,?,unhex(?),?, ?)";

		MYSQL_STMT *stmt;
		stmt_execute((void**)&stmt, sql, pbind, 8, false);

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);

		device_id = get_device_id(GUID, &_LocationID, &_OriginalLocationID, is_enabled);

		if ( device_id != 0
			 && _LocationID != *LocationID
			 && _OriginalLocationID != *LocationID
			 && ( is_enabled == NULL || *is_enabled == true ) ) {

				 *LocationID = 0;
				 device_id = 0;
		} else if ( new_device != NULL ) {

			*new_device = true;

		}


	} else if ( _LocationID != *LocationID
			    && _OriginalLocationID != *LocationID ) {

		*LocationID = 0;
		device_id = 0;

	} else {

		if ( *LocationID == _LocationID ) {
			_OriginalLocationID = 0;
		}

		MYSQL_BIND pbind[7];
		memset(pbind, 0, sizeof(pbind));

		pbind[0].buffer_type= MYSQL_TYPE_STRING;
		pbind[0].buffer= (char *)NameHEX;
		pbind[0].buffer_length = strnlen(NameHEX, 256);

		pbind[1].buffer_type= MYSQL_TYPE_LONG;
		pbind[1].buffer= (char *)&ipv4;

		pbind[2].buffer_type= MYSQL_TYPE_STRING;
		pbind[2].buffer= (char *)softver;
		pbind[2].buffer_length = strnlen(softver, 32);

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&proto_version;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&_LocationID;

		pbind[5].buffer_type= MYSQL_TYPE_LONG;
		pbind[5].buffer= (char *)&_OriginalLocationID;

		pbind[6].buffer_type= MYSQL_TYPE_LONG;
		pbind[6].buffer= (char *)&device_id;

		const char sql[] = "UPDATE `supla_iodevice` SET `name` = unhex(?), `last_connected` = NOW(), `last_ipv4` = ?, `software_version` = ?, `protocol_version` = ?, location_id = ?, original_location_id = NULLIF(?, 0) WHERE id = ?";

		MYSQL_STMT *stmt;
		if ( !stmt_execute((void**)&stmt, sql, pbind, 7) ) {
			device_id = 0;
		}

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);
	}

	return device_id;
}

int database::get_device_channel_count(int DeviceID) {

	if ( _mysql == NULL )
		return 0;


	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	MYSQL_STMT *stmt;
	int count = 0;

	stmt_get_int((void**)&stmt, &count, NULL, NULL, NULL, "SELECT COUNT(*) FROM `supla_dev_channel` WHERE `iodevice_id` = ?", pbind, 1);

	return count;
}

int database::get_device_channel_id(int DeviceID, int ChannelNumber, int *Type) {

	if ( _mysql == NULL )
		return 0;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&ChannelNumber;

	MYSQL_STMT *stmt;
	int id = 0;
	int _type;

	if ( Type == NULL )
		Type = &_type;

	stmt_get_int((void**)&stmt, &id, Type, NULL, NULL, "SELECT `id`, `type` FROM `supla_dev_channel` WHERE `iodevice_id` = ? AND `channel_number` = ?", pbind, 2);

	return id;
}


int database::add_device_channel(int DeviceID, int ChannelNumber, int Type, int Func, int FList, int UserID, bool *new_channel) {

	MYSQL_BIND pbind[6];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&Type;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&Func;

	pbind[2].buffer_type= MYSQL_TYPE_LONG;
	pbind[2].buffer= (char *)&UserID;

	pbind[3].buffer_type= MYSQL_TYPE_LONG;
	pbind[3].buffer= (char *)&ChannelNumber;

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&DeviceID;

	pbind[5].buffer_type= MYSQL_TYPE_LONG;
	pbind[5].buffer= (char *)&FList;

	{
		const char sql[] = "INSERT INTO `supla_dev_channel` (`type`, `func`, `param1`, `param2`, `param3`, `user_id`, `channel_number`, `iodevice_id`, `flist`) VALUES (?,?,0,0,0,?,?,?,?)";

		MYSQL_STMT *stmt;
		if ( !stmt_execute((void**)&stmt, sql, pbind, 6, false) ) {
			mysql_stmt_close(stmt);
			return 0;
		} else if ( new_channel ) {
			*new_channel = true;
		}

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);
	}

	return get_device_channel_id(DeviceID, ChannelNumber, NULL);

}

void database::get_device_channels(int DeviceID, supla_device_channels *channels) {


	MYSQL_STMT *stmt;
	const char sql[] = "SELECT `type`, `func`, `param1`, `param2`, `param3`, `channel_number`, `id` FROM `supla_dev_channel` WHERE `iodevice_id` = ? ORDER BY `channel_number`";

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		my_bool       is_null[5];

		MYSQL_BIND rbind[7];
		memset(rbind, 0, sizeof(rbind));

		int type, func, param1, param2, param3, number, id;

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&type;
		rbind[0].is_null= &is_null[0];

		rbind[1].buffer_type= MYSQL_TYPE_LONG;
		rbind[1].buffer= (char *)&func;
		rbind[1].is_null= &is_null[1];

		rbind[2].buffer_type= MYSQL_TYPE_LONG;
		rbind[2].buffer= (char *)&param1;
		rbind[2].is_null= &is_null[2];

		rbind[3].buffer_type= MYSQL_TYPE_LONG;
		rbind[3].buffer= (char *)&param2;
		rbind[3].is_null= &is_null[3];

		rbind[4].buffer_type= MYSQL_TYPE_LONG;
		rbind[4].buffer= (char *)&param3;
		rbind[4].is_null= &is_null[4];

		rbind[5].buffer_type= MYSQL_TYPE_LONG;
		rbind[5].buffer= (char *)&number;

		rbind[6].buffer_type= MYSQL_TYPE_LONG;
		rbind[6].buffer= (char *)&id;



		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {

				while (!mysql_stmt_fetch(stmt)) {


					if ( is_null[0] == true ) type = 0;
					if ( is_null[1] == true ) func = 0;
					if ( is_null[2] == true ) param1 = 0;
					if ( is_null[3] == true ) param2 = 0;
					if ( is_null[4] == true ) param3 = 0;

					channels->add_channel(id, number, type, func, param1, param2, param3);
				}

			}

		}


		mysql_stmt_close(stmt);
	}

}

int database::get_client_id(int UserID, const char GUID[SUPLA_GUID_SIZE]) {

	MYSQL_STMT *stmt;
	int Result = 0;

	char GUIDHEX[SUPLA_GUID_HEXSIZE];
	st_guid2hex(GUIDHEX, GUID);

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&UserID;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)GUIDHEX;
	pbind[1].buffer_length= SUPLA_GUID_SIZE*2;

    if ( !stmt_get_int((void**)&stmt, &Result, NULL, NULL, NULL, "SELECT id FROM supla_client WHERE user_id = ? AND guid = unhex(?)", pbind, 2))
    	return 0;

    return Result;

}

int database::get_client(int ClientID, bool *client_enabled, int *access_id, bool *accessid_enabled) {

	if ( _mysql == NULL || ClientID == 0 )
		return 0;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ClientID;

	int _client_enabled = 0;
	int _access_id = 0;
	int _accessid_enabled = 0;

	MYSQL_STMT *stmt;

	if ( stmt_get_int((void**)&stmt, &_client_enabled, &_access_id, &_accessid_enabled, NULL, "SELECT CAST(c.`enabled` AS unsigned integer) `c_enabled`, IFNULL(c.access_id, 0), IFNULL(CAST(a.`enabled` AS unsigned integer), 0) `a_enabled` FROM supla_client c LEFT JOIN supla_accessid a ON a.id = c.access_id WHERE c.id = ?", pbind, 1) ) {
		*client_enabled = _client_enabled == 1;
		*access_id = _access_id;
		*accessid_enabled = _accessid_enabled == 1;
        return ClientID;
	}

	return 0;

}

int database::get_client_limit_left(int UserID) {
    return get_int(UserID, 0, "SELECT IFNULL(limit_client, 0) - IFNULL(( SELECT COUNT(*) FROM supla_client WHERE user_id = supla_user.id ), 0) FROM supla_user WHERE id = ?");
}

int database::get_client_count(int UserID) {
    return get_count(UserID, "SELECT COUNT(*) FROM supla_client WHERE user_id = ?");
}

int database::get_access_id(int UserID, bool enabled) {

	MYSQL_STMT *stmt;
	int Result = 0;
	int _enabled = enabled ? 1 : 0;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&UserID;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&_enabled;

    if ( !stmt_get_int((void**)&stmt, &Result, NULL, NULL, NULL, "SELECT id FROM `supla_accessid` WHERE user_id = ? AND enabled = ?", pbind, 2))
    	return 0;

    return Result;
}

bool database::get_client_reg_enabled(int UserID) {
	return get_count(UserID, "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND client_reg_enabled IS NOT NULL AND client_reg_enabled >= UTC_TIMESTAMP()") > 0 ? true : false;
}

int database::add_client(int AccessID, const char *GUID, const char *AuthKey, const char *Name,
							unsigned int ipv4, char *softver, int proto_version, int UserID) {


	int ClientID = 0;

	char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];
	st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

	char *AuthKeyHashHEX = NULL;

	MYSQL_BIND pbind[9];
	memset(pbind, 0, sizeof(pbind));

	char GUIDHEX[SUPLA_GUID_HEXSIZE];
	st_guid2hex(GUIDHEX, GUID);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&AccessID;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)GUIDHEX;
	pbind[1].buffer_length= (SUPLA_GUID_SIZE*2);

	pbind[2].buffer_type= MYSQL_TYPE_STRING;
	pbind[2].buffer= (char *)NameHEX;
	pbind[2].buffer_length = strnlen(NameHEX, 256);

	pbind[3].buffer_type= MYSQL_TYPE_LONG;
	pbind[3].buffer= (char *)&ipv4;

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&ipv4;

	pbind[5].buffer_type= MYSQL_TYPE_STRING;
	pbind[5].buffer= (char *)softver;
	pbind[5].buffer_length = strnlen(softver, 32);

	pbind[6].buffer_type= MYSQL_TYPE_LONG;
	pbind[6].buffer= (char *)&proto_version;

	pbind[7].buffer_type= MYSQL_TYPE_LONG;
	pbind[7].buffer= (char *)&UserID;

	if ( AuthKey == NULL ) {
		pbind[8].buffer_type= MYSQL_TYPE_NULL;
	} else {

		AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

		if ( AuthKeyHashHEX == NULL )
			return 0;

		pbind[8].buffer_type= MYSQL_TYPE_STRING;
		pbind[8].buffer= (char *)AuthKeyHashHEX;
		pbind[8].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE*2);

	}


	const char sql[] = "INSERT INTO `supla_client`(`access_id`, `guid`, `name`, `enabled`, `reg_ipv4`, `reg_date`, `last_access_ipv4`, `last_access_date`, `software_version`, `protocol_version`, `user_id`, `auth_key`) VALUES (?,unhex(?),unhex(?),1,?,NOW(),?,NOW(),?,?,?,unhex(?))";


	MYSQL_STMT *stmt;
	if ( stmt_execute((void**)&stmt, sql, pbind, 9, false) ) {
		ClientID = get_last_insert_id();
	}

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

	if ( AuthKeyHashHEX ) {

		free(AuthKeyHashHEX);
		AuthKeyHashHEX = NULL;

	}

	return ClientID;

}

bool database::update_client(int ClientID, int AccessID, const char *AuthKey, const char *Name,
		unsigned int ipv4, char *softver, int proto_version) {

	bool result = false;

	char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];
	st_str2hex(NameHEX, Name, SUPLA_DEVICE_NAME_MAXSIZE);

	MYSQL_BIND pbind[7];
	memset(pbind, 0, sizeof(pbind));

	char *AuthKeyHashHEX = NULL;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&AccessID;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)NameHEX;
	pbind[1].buffer_length = strnlen(NameHEX, 256);

	pbind[2].buffer_type= MYSQL_TYPE_LONG;
	pbind[2].buffer= (char *)&ipv4;

	pbind[3].buffer_type= MYSQL_TYPE_STRING;
	pbind[3].buffer= (char *)softver;
	pbind[3].buffer_length = strnlen(softver, 32);

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&proto_version;

	if ( AuthKey == NULL ) {
		pbind[5].buffer_type= MYSQL_TYPE_NULL;
	} else {

		AuthKeyHashHEX = st_get_authkey_hash_hex(AuthKey);

		if ( AuthKeyHashHEX == NULL )
			return 0;

		pbind[5].buffer_type= MYSQL_TYPE_STRING;
		pbind[5].buffer= (char *)AuthKeyHashHEX;
		pbind[5].buffer_length = strnlen(AuthKeyHashHEX, BCRYPT_HASH_MAXSIZE*2);

	}

	pbind[6].buffer_type= MYSQL_TYPE_LONG;
	pbind[6].buffer= (char *)&ClientID;

	const char sql[] = "UPDATE `supla_client` SET `access_id` = ?, `name` = unhex(?), `last_access_date` = NOW(), `last_access_ipv4` = ?, `software_version` = ?, `protocol_version` = ?, `auth_key` = unhex(?) WHERE id = ?";

	MYSQL_STMT *stmt;
	if ( stmt_execute((void**)&stmt, sql, pbind, 7) ) {
		result = true;
	}

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

	if ( AuthKeyHashHEX ) {

		free(AuthKeyHashHEX);
		AuthKeyHashHEX = NULL;

	}

	return result;

}

void database::get_client_locations(int ClientID, supla_client_locations *locs) {


	MYSQL_STMT *stmt;

	const char sql[] = "SELECT `id`, `caption` FROM `supla_v_client_location`  WHERE `client_id` = ?";

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ClientID;

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		my_bool       is_null[2];

		MYSQL_BIND rbind[2];
		memset(rbind, 0, sizeof(rbind));

		int id;
		unsigned long size;
		char caption[401]; // utf8

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&id;
		rbind[0].is_null= &is_null[0];

		rbind[1].buffer_type= MYSQL_TYPE_STRING;
		rbind[1].buffer= caption;
		rbind[1].buffer_length =  401;
		rbind[1].length = &size;
		rbind[1].is_null= &is_null[1];


		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {


				while (!mysql_stmt_fetch(stmt)) {

					caption[size] = 0;
					locs->add_location(id, caption);

				}

			}

		}

		mysql_stmt_close(stmt);
	}

}

void database::get_client_channels(int ClientID, int *DeviceID, supla_client_channels *channels) {

	MYSQL_STMT *stmt;
	const char sql1[] = "SELECT `id`, `func`, `param1`, `iodevice_id`, `location_id`, `caption` FROM `supla_v_client_channel` WHERE `client_id` = ? ORDER BY `iodevice_id`, `channel_number`";
	const char sql2[] = "SELECT `id`, `func`, `param1`, `iodevice_id`, `location_id`, `caption` FROM `supla_v_client_channel` WHERE `client_id` = ? AND `iodevice_id` = ? ORDER BY `channel_number`";


	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ClientID;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)DeviceID;

	if ( stmt_execute((void**)&stmt, DeviceID ? sql2 : sql1, pbind, DeviceID ? 2 : 1, true) ) {

		my_bool       is_null;

		MYSQL_BIND rbind[6];
		memset(rbind, 0, sizeof(rbind));

		int id, func, param1, iodevice_id, location_id;
		unsigned long size;
		char caption[401];

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&id;

		rbind[1].buffer_type= MYSQL_TYPE_LONG;
		rbind[1].buffer= (char *)&func;

		rbind[2].buffer_type= MYSQL_TYPE_LONG;
		rbind[2].buffer= (char *)&param1;

		rbind[3].buffer_type= MYSQL_TYPE_LONG;
		rbind[3].buffer= (char *)&iodevice_id;

		rbind[4].buffer_type= MYSQL_TYPE_LONG;
		rbind[4].buffer= (char *)&location_id;

		rbind[5].buffer_type= MYSQL_TYPE_STRING;
		rbind[5].buffer= caption;
		rbind[5].is_null= &is_null;
		rbind[5].buffer_length = 401;
		rbind[5].length = &size;

		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {

				while (!mysql_stmt_fetch(stmt)) {

					if ( is_null == false )
						caption[size] = 0;

					channels->update_channel(id, iodevice_id, location_id, func, param1, is_null ? NULL : caption);
				}

			}

		}


		mysql_stmt_close(stmt);
	}

}

void database::add_temperature(int ChannelID, double temperature) {

	char buff[20];
	memset(buff, 0, sizeof(buff));

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	snprintf(buff, 20, "%04.4f", temperature);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ChannelID;

	pbind[1].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[1].buffer= (char *)buff;
	pbind[1].buffer_length = strnlen(buff, 20);


	const char sql[] = "INSERT INTO `supla_temperature_log`(`channel_id`, `date`, `temperature`) VALUES (?,NOW(),?)";

	MYSQL_STMT *stmt;
	stmt_execute((void**)&stmt, sql, pbind, 2, false);

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

}

void database::add_temperature_and_humidity(int ChannelID, double temperature, double humidity) {

	char buff1[20];
	memset(buff1, 0, sizeof(buff1));

	char buff2[20];
	memset(buff2, 0, sizeof(buff2));

	MYSQL_BIND pbind[3];
	memset(pbind, 0, sizeof(pbind));

	snprintf(buff1, 20, "%04.3f", temperature);
	snprintf(buff2, 20, "%04.3f", humidity);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ChannelID;

	pbind[1].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[1].buffer= (char *)buff1;
	pbind[1].buffer_length = strnlen(buff1, 20);

	pbind[2].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[2].buffer= (char *)buff2;
	pbind[2].buffer_length = strnlen(buff2, 20);

	const char sql[] = "INSERT INTO `supla_temphumidity_log`(`channel_id`, `date`, `temperature`, `humidity`) VALUES (?,NOW(),?,?)";

	MYSQL_STMT *stmt;
	stmt_execute((void**)&stmt, sql, pbind, 3, false);

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

}

bool database::get_oauth_user(char *access_token, int *OAuthUserID, int *UserID, int *expires_at) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	bool result = false;

	pbind[0].buffer_type= MYSQL_TYPE_STRING;
	pbind[0].buffer= (char *)access_token;
	pbind[0].buffer_length = strnlen(access_token, 512);

	const char sql[] = "SELECT  t.user_id, c.parent_id, t.expires_at FROM `supla_oauth_access_tokens` AS t, `supla_oauth_clients` AS c WHERE c.id = t.client_id AND c.parent_id != 0 AND t.expires_at > UNIX_TIMESTAMP(NOW()) AND t.scope = 'restapi' AND token = ? LIMIT 1";

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		mysql_stmt_store_result(stmt);

		if ( mysql_stmt_num_rows(stmt) > 0 ) {

			MYSQL_BIND rbind[3];
			memset(rbind, 0, sizeof(rbind));

			int _OAuthUserID, _UserID, _expires_at;

			rbind[0].buffer_type= MYSQL_TYPE_LONG;
			rbind[0].buffer= (char *)&_OAuthUserID;

			rbind[1].buffer_type= MYSQL_TYPE_LONG;
			rbind[1].buffer= (char *)&_UserID;

			rbind[2].buffer_type= MYSQL_TYPE_LONG;
			rbind[2].buffer= (char *)&_expires_at;

			if ( mysql_stmt_bind_result(stmt, rbind) ) {
				supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));

			} else if ( mysql_stmt_fetch(stmt) == 0 ) {

				if ( OAuthUserID != NULL )
					*OAuthUserID = _OAuthUserID;

				if ( UserID != NULL )
					*UserID = _UserID;

				if ( expires_at != NULL ) {
					*expires_at = _expires_at;
				}

				result = true;

			}

		}

		mysql_stmt_free_result(stmt);
		mysql_stmt_close(stmt);
	}

	return result;
}

bool database::get_device_firmware_update_url(int DeviceID, TDS_FirmwareUpdateParams *params, TSD_FirmwareUpdate_UrlResult *url) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[6];
	memset(pbind, 0, sizeof(pbind));
	memset(url, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

	bool result = false;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	pbind[1].buffer_type= MYSQL_TYPE_TINY;
	pbind[1].buffer= (char *)&params->Platform;

	pbind[2].buffer_type= MYSQL_TYPE_LONG;
	pbind[2].buffer= (char *)&params->Param1;

	pbind[3].buffer_type= MYSQL_TYPE_LONG;
	pbind[3].buffer= (char *)&params->Param2;

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&params->Param3;

	pbind[5].buffer_type= MYSQL_TYPE_LONG;
	pbind[5].buffer= (char *)&params->Param4;

	const char sql1[] = "SET @p0=?, @p1=?, @p2=?, @p3=?, @p4=?, @p5=?";
	const char sql2[] = "CALL `supla_get_device_firmware_url`(@p0, @p1, @p2, @p3, @p4, @p5, @p6, @p7, @p8, @p9)";
	const char sql3[] = "SELECT @p6 AS `protocols`, @p7 AS `host`, @p8 AS `port`, @p9 AS `path`";

	char q_executed = 0;

	if ( stmt_execute((void**)&stmt, sql1, pbind, 6, true) ) {

		 mysql_stmt_close((MYSQL_STMT *)stmt);
		 q_executed++;
	}

	if ( stmt_execute((void**)&stmt, sql2, NULL, 0, true) ) {

		 mysql_stmt_close((MYSQL_STMT *)stmt);
		 q_executed++;
	}

	if ( q_executed == 2
		 && stmt_execute((void**)&stmt, sql3, NULL, 0, true) ) {

		mysql_stmt_store_result(stmt);

		if ( mysql_stmt_num_rows(stmt) > 0 ) {

			MYSQL_BIND rbind[4];
			memset(rbind, 0, sizeof(rbind));

			unsigned long host_size;
			my_bool       host_is_null;

			unsigned long path_size;
			my_bool       path_is_null;

			rbind[0].buffer_type= MYSQL_TYPE_TINY;
			rbind[0].buffer= (char *)&url->url.available_protocols;

			rbind[1].buffer_type= MYSQL_TYPE_STRING;
			rbind[1].buffer= url->url.host;
			rbind[1].is_null= &host_is_null;
			rbind[1].buffer_length = SUPLA_URL_HOST_MAXSIZE;
			rbind[1].length = &host_size;

			rbind[2].buffer_type= MYSQL_TYPE_LONG;
			rbind[2].buffer= (char *)&url->url.port;

			rbind[3].buffer_type= MYSQL_TYPE_STRING;
			rbind[3].buffer= url->url.path;
			rbind[3].is_null= &path_is_null;
			rbind[3].buffer_length = SUPLA_URL_PATH_MAXSIZE;
			rbind[3].length = &path_size;


			if ( mysql_stmt_bind_result(stmt, rbind) ) {
				supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));

			} else if ( mysql_stmt_fetch(stmt) == 0 ) {

				if ( host_is_null || host_size == 0 || host_size >= SUPLA_URL_HOST_MAXSIZE )
					url->url.host[0] = 0;

				if ( path_is_null || path_size == 0 || path_size >= SUPLA_URL_PATH_MAXSIZE )
					url->url.path[0] = 0;


				if ( url->url.available_protocols > 0
					 && strnlen(url->url.host, SUPLA_URL_HOST_MAXSIZE) > 0
					 && strnlen(url->url.path, SUPLA_URL_PATH_MAXSIZE) > 0 )
					url->exists = 1;

				result = true;

			}

		}

		mysql_stmt_free_result(stmt);
		mysql_stmt_close(stmt);
	}

	return result;
}

