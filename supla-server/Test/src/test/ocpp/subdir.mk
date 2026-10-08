################################################################################
# Automatically-generated file. Do not edit!
# SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
# SPDX-License-Identifier: GPL-2.0-or-later
################################################################################

CPP_SRCS += \
../src/test/ocpp/OcppDeviceTest.cpp \
../src/test/ocpp/OcppConnectionTest.cpp \
../src/test/ocpp/OcppLoggerTest.cpp

CPP_DEPS += \
./src/test/ocpp/OcppDeviceTest.d \
./src/test/ocpp/OcppConnectionTest.d \
./src/test/ocpp/OcppLoggerTest.d

OBJS += \
./src/test/ocpp/OcppDeviceTest.o \
./src/test/ocpp/OcppConnectionTest.o \
./src/test/ocpp/OcppLoggerTest.o

src/test/ocpp/%.o: ../src/test/ocpp/%.cpp src/test/ocpp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -I/usr/include/cjson -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean: clean-src-2f-test-2f-ocpp

clean-src-2f-test-2f-ocpp:
	-$(RM) ./src/test/ocpp/OcppDeviceTest.d ./src/test/ocpp/OcppDeviceTest.o
	-$(RM) ./src/test/ocpp/OcppConnectionTest.d ./src/test/ocpp/OcppConnectionTest.o
	-$(RM) ./src/test/ocpp/OcppLoggerTest.d ./src/test/ocpp/OcppLoggerTest.o

.PHONY: clean-src-2f-test-2f-ocpp
