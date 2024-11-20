################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/device/ChannelAvailabilityStatusTest.cpp \
../src/test/device/ChannelFragmentTest.cpp \
../src/test/device/ChannelPropertyGetterTest.cpp \
../src/test/device/ChannelRelationTest.cpp \
../src/test/device/ChannelStateTest.cpp \
../src/test/device/CommonChannelPropertiesTest.cpp \
../src/test/device/RegisterDeviceEssentialTest.cpp \
../src/test/device/RegisterDeviceTest.cpp \
../src/test/device/RegisterDeviceWithEmailAuthTest.cpp \
../src/test/device/RegisterDeviceWithLocationAuthTest.cpp 

CPP_DEPS += \
./src/test/device/ChannelAvailabilityStatusTest.d \
./src/test/device/ChannelFragmentTest.d \
./src/test/device/ChannelPropertyGetterTest.d \
./src/test/device/ChannelRelationTest.d \
./src/test/device/ChannelStateTest.d \
./src/test/device/CommonChannelPropertiesTest.d \
./src/test/device/RegisterDeviceEssentialTest.d \
./src/test/device/RegisterDeviceTest.d \
./src/test/device/RegisterDeviceWithEmailAuthTest.d \
./src/test/device/RegisterDeviceWithLocationAuthTest.d 

OBJS += \
./src/test/device/ChannelAvailabilityStatusTest.o \
./src/test/device/ChannelFragmentTest.o \
./src/test/device/ChannelPropertyGetterTest.o \
./src/test/device/ChannelRelationTest.o \
./src/test/device/ChannelStateTest.o \
./src/test/device/CommonChannelPropertiesTest.o \
./src/test/device/RegisterDeviceEssentialTest.o \
./src/test/device/RegisterDeviceTest.o \
./src/test/device/RegisterDeviceWithEmailAuthTest.o \
./src/test/device/RegisterDeviceWithLocationAuthTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/device/%.o: ../src/test/device/%.cpp src/test/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-device

clean-src-2f-test-2f-device:
	-$(RM) ./src/test/device/ChannelAvailabilityStatusTest.d ./src/test/device/ChannelAvailabilityStatusTest.o ./src/test/device/ChannelFragmentTest.d ./src/test/device/ChannelFragmentTest.o ./src/test/device/ChannelPropertyGetterTest.d ./src/test/device/ChannelPropertyGetterTest.o ./src/test/device/ChannelRelationTest.d ./src/test/device/ChannelRelationTest.o ./src/test/device/ChannelStateTest.d ./src/test/device/ChannelStateTest.o ./src/test/device/CommonChannelPropertiesTest.d ./src/test/device/CommonChannelPropertiesTest.o ./src/test/device/RegisterDeviceEssentialTest.d ./src/test/device/RegisterDeviceEssentialTest.o ./src/test/device/RegisterDeviceTest.d ./src/test/device/RegisterDeviceTest.o ./src/test/device/RegisterDeviceWithEmailAuthTest.d ./src/test/device/RegisterDeviceWithEmailAuthTest.o ./src/test/device/RegisterDeviceWithLocationAuthTest.d ./src/test/device/RegisterDeviceWithLocationAuthTest.o

.PHONY: clean-src-2f-test-2f-device

