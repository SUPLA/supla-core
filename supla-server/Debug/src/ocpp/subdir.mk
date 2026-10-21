################################################################################
# Automatically-generated file. Do not edit!
# SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
# SPDX-License-Identifier: GPL-2.0-or-later
################################################################################

CPP_SRCS += \
../src/ocpp/ocpp_device.cpp \
../src/ocpp/ocpp_devices.cpp \
../src/ocpp/ocpp_dao.cpp \
../src/ocpp/ocpp_accept_loop.cpp \
../src/ocpp/ocpp_task_queue.cpp \
../src/ocpp/ocpp_worker_pool.cpp \
../src/ocpp/ocpp_gateway.cpp

CPP_DEPS += \
./src/ocpp/ocpp_device.d \
./src/ocpp/ocpp_devices.d \
./src/ocpp/ocpp_dao.d \
./src/ocpp/ocpp_accept_loop.d \
./src/ocpp/ocpp_task_queue.d \
./src/ocpp/ocpp_worker_pool.d \
./src/ocpp/ocpp_gateway.d

OBJS += \
./src/ocpp/ocpp_device.o \
./src/ocpp/ocpp_devices.o \
./src/ocpp/ocpp_dao.o \
./src/ocpp/ocpp_accept_loop.o \
./src/ocpp/ocpp_task_queue.o \
./src/ocpp/ocpp_worker_pool.o \
./src/ocpp/ocpp_gateway.o

src/ocpp/%.o: ../src/ocpp/%.cpp src/ocpp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -I/usr/include/cjson -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean: clean-src-2f-ocpp

clean-src-2f-ocpp:
	-$(RM) ./src/ocpp/ocpp_device.d ./src/ocpp/ocpp_device.o ./src/ocpp/ocpp_devices.d ./src/ocpp/ocpp_devices.o ./src/ocpp/ocpp_dao.d ./src/ocpp/ocpp_dao.o
	-$(RM) ./src/ocpp/ocpp_accept_loop.d ./src/ocpp/ocpp_accept_loop.o ./src/ocpp/ocpp_task_queue.d ./src/ocpp/ocpp_task_queue.o ./src/ocpp/ocpp_worker_pool.d ./src/ocpp/ocpp_worker_pool.o ./src/ocpp/ocpp_gateway.d ./src/ocpp/ocpp_gateway.o

.PHONY: clean-src-2f-ocpp
