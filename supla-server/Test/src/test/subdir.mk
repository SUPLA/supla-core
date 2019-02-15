################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/AllTests.cpp \
../src/test/DeviceChannelTest.cpp \
../src/test/ProtoTest.cpp \
../src/test/STTrivialHttp.cpp \
../src/test/STUserSpace.cpp \
../src/test/SafeArrayTest.cpp \
../src/test/SrpcTest.cpp \
../src/test/ToolsTest.cpp 

OBJS += \
./src/test/AllTests.o \
./src/test/DeviceChannelTest.o \
./src/test/ProtoTest.o \
./src/test/STTrivialHttp.o \
./src/test/STUserSpace.o \
./src/test/SafeArrayTest.o \
./src/test/SrpcTest.o \
./src/test/ToolsTest.o 

CPP_DEPS += \
./src/test/AllTests.d \
./src/test/DeviceChannelTest.d \
./src/test/ProtoTest.d \
./src/test/STTrivialHttp.d \
./src/test/STUserSpace.d \
./src/test/SafeArrayTest.d \
./src/test/SrpcTest.d \
./src/test/ToolsTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I/usr/include/mysql -I../src -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


