################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/db/abstract_db_access_provider.cpp \
../src/db/database.cpp \
../src/db/dbcommon.cpp \
../src/db/mariadb_access_provider.cpp \
../src/db/server_settings_dao.cpp \
../src/db/svrdb.cpp 

CPP_DEPS += \
./src/db/abstract_db_access_provider.d \
./src/db/database.d \
./src/db/dbcommon.d \
./src/db/mariadb_access_provider.d \
./src/db/server_settings_dao.d \
./src/db/svrdb.d 

OBJS += \
./src/db/abstract_db_access_provider.o \
./src/db/database.o \
./src/db/dbcommon.o \
./src/db/mariadb_access_provider.o \
./src/db/server_settings_dao.o \
./src/db/svrdb.o 


# Each subdirectory must supply rules for building sources it contributes
src/db/%.o: ../src/db/%.cpp src/db/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__BCRYPT=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/external/cJSON -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-db

clean-src-2f-db:
	-$(RM) ./src/db/abstract_db_access_provider.d ./src/db/abstract_db_access_provider.o ./src/db/database.d ./src/db/database.o ./src/db/dbcommon.d ./src/db/dbcommon.o ./src/db/mariadb_access_provider.d ./src/db/mariadb_access_provider.o ./src/db/server_settings_dao.d ./src/db/server_settings_dao.o ./src/db/svrdb.d ./src/db/svrdb.o

.PHONY: clean-src-2f-db

