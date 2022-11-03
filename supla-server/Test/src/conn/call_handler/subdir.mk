################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/conn/call_handler/abstract_register_object.cpp \
../src/conn/call_handler/get_registration_enabled.cpp \
../src/conn/call_handler/get_user_localtime.cpp \
../src/conn/call_handler/get_version.cpp \
../src/conn/call_handler/ping_server.cpp \
../src/conn/call_handler/set_activity_timeout.cpp 

CPP_DEPS += \
./src/conn/call_handler/abstract_register_object.d \
./src/conn/call_handler/get_registration_enabled.d \
./src/conn/call_handler/get_user_localtime.d \
./src/conn/call_handler/get_version.d \
./src/conn/call_handler/ping_server.d \
./src/conn/call_handler/set_activity_timeout.d 

OBJS += \
./src/conn/call_handler/abstract_register_object.o \
./src/conn/call_handler/get_registration_enabled.o \
./src/conn/call_handler/get_user_localtime.o \
./src/conn/call_handler/get_version.o \
./src/conn/call_handler/ping_server.o \
./src/conn/call_handler/set_activity_timeout.o 


# Each subdirectory must supply rules for building sources it contributes
src/conn/call_handler/%.o: ../src/conn/call_handler/%.cpp src/conn/call_handler/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-conn-2f-call_handler

clean-src-2f-conn-2f-call_handler:
	-$(RM) ./src/conn/call_handler/abstract_register_object.d ./src/conn/call_handler/abstract_register_object.o ./src/conn/call_handler/get_registration_enabled.d ./src/conn/call_handler/get_registration_enabled.o ./src/conn/call_handler/get_user_localtime.d ./src/conn/call_handler/get_user_localtime.o ./src/conn/call_handler/get_version.d ./src/conn/call_handler/get_version.o ./src/conn/call_handler/ping_server.d ./src/conn/call_handler/ping_server.o ./src/conn/call_handler/set_activity_timeout.d ./src/conn/call_handler/set_activity_timeout.o

.PHONY: clean-src-2f-conn-2f-call_handler

