################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/AllTests.cpp \
../src/test/CallerClassTest.cpp \
../src/test/DCPairTest.cpp \
../src/test/DeviceChannelTest.cpp \
../src/test/ProtoTest.cpp \
../src/test/SafeArrayTest.cpp \
../src/test/SrpcTest.cpp \
../src/test/TestHelper.cpp \
../src/test/ToolsTest.cpp 

CPP_DEPS += \
./src/test/AllTests.d \
./src/test/CallerClassTest.d \
./src/test/DCPairTest.d \
./src/test/DeviceChannelTest.d \
./src/test/ProtoTest.d \
./src/test/SafeArrayTest.d \
./src/test/SrpcTest.d \
./src/test/TestHelper.d \
./src/test/ToolsTest.d 

OBJS += \
./src/test/AllTests.o \
./src/test/CallerClassTest.o \
./src/test/DCPairTest.o \
./src/test/DeviceChannelTest.o \
./src/test/ProtoTest.o \
./src/test/SafeArrayTest.o \
./src/test/SrpcTest.o \
./src/test/TestHelper.o \
./src/test/ToolsTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp src/test/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test

clean-src-2f-test:
	-$(RM) ./src/test/AllTests.d ./src/test/AllTests.o ./src/test/CallerClassTest.d ./src/test/CallerClassTest.o ./src/test/DCPairTest.d ./src/test/DCPairTest.o ./src/test/DeviceChannelTest.d ./src/test/DeviceChannelTest.o ./src/test/ProtoTest.d ./src/test/ProtoTest.o ./src/test/SafeArrayTest.d ./src/test/SafeArrayTest.o ./src/test/SrpcTest.d ./src/test/SrpcTest.o ./src/test/TestHelper.d ./src/test/TestHelper.o ./src/test/ToolsTest.d ./src/test/ToolsTest.o

.PHONY: clean-src-2f-test

