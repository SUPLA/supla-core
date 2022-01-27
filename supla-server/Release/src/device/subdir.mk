################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/abstractdevicefinder.cpp \
../src/device/device.cpp \
../src/device/devicechannel.cpp \
../src/device/devicefinder.cpp 

CPP_DEPS += \
./src/device/abstractdevicefinder.d \
./src/device/device.d \
./src/device/devicechannel.d \
./src/device/devicefinder.d 

OBJS += \
./src/device/abstractdevicefinder.o \
./src/device/device.o \
./src/device/devicechannel.o \
./src/device/devicefinder.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp src/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device

clean-src-2f-device:
	-$(RM) ./src/device/abstractdevicefinder.d ./src/device/abstractdevicefinder.o ./src/device/device.d ./src/device/device.o ./src/device/devicechannel.d ./src/device/devicechannel.o ./src/device/devicefinder.d ./src/device/devicefinder.o

.PHONY: clean-src-2f-device

