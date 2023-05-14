################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/extended_value/channel_billing_value.cpp \
../src/device/extended_value/channel_em_extended_value.cpp \
../src/device/extended_value/channel_extended_value.cpp \
../src/device/extended_value/channel_extended_value_envelope.cpp \
../src/device/extended_value/channel_ic_extended_value.cpp 

CPP_DEPS += \
./src/device/extended_value/channel_billing_value.d \
./src/device/extended_value/channel_em_extended_value.d \
./src/device/extended_value/channel_extended_value.d \
./src/device/extended_value/channel_extended_value_envelope.d \
./src/device/extended_value/channel_ic_extended_value.d 

OBJS += \
./src/device/extended_value/channel_billing_value.o \
./src/device/extended_value/channel_em_extended_value.o \
./src/device/extended_value/channel_extended_value.o \
./src/device/extended_value/channel_extended_value_envelope.o \
./src/device/extended_value/channel_ic_extended_value.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/extended_value/%.o: ../src/device/extended_value/%.cpp src/device/extended_value/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-extended_value

clean-src-2f-device-2f-extended_value:
	-$(RM) ./src/device/extended_value/channel_billing_value.d ./src/device/extended_value/channel_billing_value.o ./src/device/extended_value/channel_em_extended_value.d ./src/device/extended_value/channel_em_extended_value.o ./src/device/extended_value/channel_extended_value.d ./src/device/extended_value/channel_extended_value.o ./src/device/extended_value/channel_extended_value_envelope.d ./src/device/extended_value/channel_extended_value_envelope.o ./src/device/extended_value/channel_ic_extended_value.d ./src/device/extended_value/channel_ic_extended_value.o

.PHONY: clean-src-2f-device-2f-extended_value

