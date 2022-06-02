################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/distributedobjects/dobject.cpp \
../src/distributedobjects/dobject_abstract_remote_updater.cpp \
../src/distributedobjects/dobject_change_indicator.cpp \
../src/distributedobjects/dobjects.cpp 

CPP_DEPS += \
./src/distributedobjects/dobject.d \
./src/distributedobjects/dobject_abstract_remote_updater.d \
./src/distributedobjects/dobject_change_indicator.d \
./src/distributedobjects/dobjects.d 

OBJS += \
./src/distributedobjects/dobject.o \
./src/distributedobjects/dobject_abstract_remote_updater.o \
./src/distributedobjects/dobject_change_indicator.o \
./src/distributedobjects/dobjects.o 


# Each subdirectory must supply rules for building sources it contributes
src/distributedobjects/%.o: ../src/distributedobjects/%.cpp src/distributedobjects/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-distributedobjects

clean-src-2f-distributedobjects:
	-$(RM) ./src/distributedobjects/dobject.d ./src/distributedobjects/dobject.o ./src/distributedobjects/dobject_abstract_remote_updater.d ./src/distributedobjects/dobject_abstract_remote_updater.o ./src/distributedobjects/dobject_change_indicator.d ./src/distributedobjects/dobject_change_indicator.o ./src/distributedobjects/dobjects.d ./src/distributedobjects/dobjects.o

.PHONY: clean-src-2f-distributedobjects

