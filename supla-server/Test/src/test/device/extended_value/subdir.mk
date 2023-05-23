################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/device/extended_value/ChannelAndTimerStateExtendedValueTest.cpp \
../src/test/device/extended_value/ChannelBillingValueTest.cpp \
../src/test/device/extended_value/ChannelEmExtendedValueTest.cpp \
../src/test/device/extended_value/ChannelIcExtendedValueTest.cpp \
../src/test/device/extended_value/ChannelStateExtendedValueTest.cpp \
../src/test/device/extended_value/TimerStateExtendedValueTest.cpp 

CPP_DEPS += \
./src/test/device/extended_value/ChannelAndTimerStateExtendedValueTest.d \
./src/test/device/extended_value/ChannelBillingValueTest.d \
./src/test/device/extended_value/ChannelEmExtendedValueTest.d \
./src/test/device/extended_value/ChannelIcExtendedValueTest.d \
./src/test/device/extended_value/ChannelStateExtendedValueTest.d \
./src/test/device/extended_value/TimerStateExtendedValueTest.d 

OBJS += \
./src/test/device/extended_value/ChannelAndTimerStateExtendedValueTest.o \
./src/test/device/extended_value/ChannelBillingValueTest.o \
./src/test/device/extended_value/ChannelEmExtendedValueTest.o \
./src/test/device/extended_value/ChannelIcExtendedValueTest.o \
./src/test/device/extended_value/ChannelStateExtendedValueTest.o \
./src/test/device/extended_value/TimerStateExtendedValueTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/device/extended_value/%.o: ../src/test/device/extended_value/%.cpp src/test/device/extended_value/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-device-2f-extended_value

clean-src-2f-test-2f-device-2f-extended_value:
	-$(RM) ./src/test/device/extended_value/ChannelAndTimerStateExtendedValueTest.d ./src/test/device/extended_value/ChannelAndTimerStateExtendedValueTest.o ./src/test/device/extended_value/ChannelBillingValueTest.d ./src/test/device/extended_value/ChannelBillingValueTest.o ./src/test/device/extended_value/ChannelEmExtendedValueTest.d ./src/test/device/extended_value/ChannelEmExtendedValueTest.o ./src/test/device/extended_value/ChannelIcExtendedValueTest.d ./src/test/device/extended_value/ChannelIcExtendedValueTest.o ./src/test/device/extended_value/ChannelStateExtendedValueTest.d ./src/test/device/extended_value/ChannelStateExtendedValueTest.o ./src/test/device/extended_value/TimerStateExtendedValueTest.d ./src/test/device/extended_value/TimerStateExtendedValueTest.o

.PHONY: clean-src-2f-test-2f-device-2f-extended_value

