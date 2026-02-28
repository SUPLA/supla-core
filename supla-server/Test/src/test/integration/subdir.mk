################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/integration/IntegrationTest.cpp \
../src/test/integration/MySqlShell.cpp \
../src/test/integration/PgSqlShell.cpp \
../src/test/integration/SqlShell.cpp \
../src/test/integration/TSDB_IntegrationTest.cpp 

CPP_DEPS += \
./src/test/integration/IntegrationTest.d \
./src/test/integration/MySqlShell.d \
./src/test/integration/PgSqlShell.d \
./src/test/integration/SqlShell.d \
./src/test/integration/TSDB_IntegrationTest.d 

OBJS += \
./src/test/integration/IntegrationTest.o \
./src/test/integration/MySqlShell.o \
./src/test/integration/PgSqlShell.o \
./src/test/integration/SqlShell.o \
./src/test/integration/TSDB_IntegrationTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/integration/%.o: ../src/test/integration/%.cpp src/test/integration/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/external/cJSON -I../src/external/MQTT-C/include -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-integration

clean-src-2f-test-2f-integration:
	-$(RM) ./src/test/integration/IntegrationTest.d ./src/test/integration/IntegrationTest.o ./src/test/integration/MySqlShell.d ./src/test/integration/MySqlShell.o ./src/test/integration/PgSqlShell.d ./src/test/integration/PgSqlShell.o ./src/test/integration/SqlShell.d ./src/test/integration/SqlShell.o ./src/test/integration/TSDB_IntegrationTest.d ./src/test/integration/TSDB_IntegrationTest.o

.PHONY: clean-src-2f-test-2f-integration

