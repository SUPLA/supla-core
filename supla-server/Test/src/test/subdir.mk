################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/AllTests.cpp \
../src/test/DeviceChannelTest.cpp \
../src/test/SafeArrayTest.cpp \
../src/test/ToolsTest.cpp 

OBJS += \
./src/test/AllTests.o \
./src/test/DeviceChannelTest.o \
./src/test/SafeArrayTest.o \
./src/test/ToolsTest.o 

CPP_DEPS += \
./src/test/AllTests.d \
./src/test/DeviceChannelTest.d \
./src/test/SafeArrayTest.d \
./src/test/ToolsTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__OPEN_SSL=1 -D__DEBUG=1 -D__BCRYPT=1 -I/usr/include/mysql -I/usr/src/openssl/include -I../src/test -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


