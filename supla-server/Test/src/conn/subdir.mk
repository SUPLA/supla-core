################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/conn/abstract_connection_object.cpp \
../src/conn/connection.cpp \
../src/conn/connection_objects.cpp 

CPP_DEPS += \
./src/conn/abstract_connection_object.d \
./src/conn/connection.d \
./src/conn/connection_objects.d 

OBJS += \
./src/conn/abstract_connection_object.o \
./src/conn/connection.o \
./src/conn/connection_objects.o 


# Each subdirectory must supply rules for building sources it contributes
src/conn/%.o: ../src/conn/%.cpp src/conn/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-conn

clean-src-2f-conn:
	-$(RM) ./src/conn/abstract_connection_object.d ./src/conn/abstract_connection_object.o ./src/conn/connection.d ./src/conn/connection.o ./src/conn/connection_objects.d ./src/conn/connection_objects.o

.PHONY: clean-src-2f-conn

