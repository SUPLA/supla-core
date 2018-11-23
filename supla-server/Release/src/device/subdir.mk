################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/device.cpp \
../src/device/devicechannel.cpp 

OBJS += \
./src/device/device.o \
./src/device/devicechannel.o 

CPP_DEPS += \
./src/device/device.d \
./src/device/devicechannel.d 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__OPENSSL_TOOLS=1 -I/usr/include/mysql -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


