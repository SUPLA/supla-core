################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/db/abstract_db_access_provider.cpp \
../src/db/database.cpp \
../src/db/db_access_provider.cpp \
../src/db/dbcommon.cpp \
../src/db/svrdb.cpp 

CPP_DEPS += \
./src/db/abstract_db_access_provider.d \
./src/db/database.d \
./src/db/db_access_provider.d \
./src/db/dbcommon.d \
./src/db/svrdb.d 

OBJS += \
./src/db/abstract_db_access_provider.o \
./src/db/database.o \
./src/db/db_access_provider.o \
./src/db/dbcommon.o \
./src/db/svrdb.o 


# Each subdirectory must supply rules for building sources it contributes
src/db/%.o: ../src/db/%.cpp src/db/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-db

clean-src-2f-db:
	-$(RM) ./src/db/abstract_db_access_provider.d ./src/db/abstract_db_access_provider.o ./src/db/database.d ./src/db/database.o ./src/db/db_access_provider.d ./src/db/db_access_provider.o ./src/db/dbcommon.d ./src/db/dbcommon.o ./src/db/svrdb.d ./src/db/svrdb.o

.PHONY: clean-src-2f-db

