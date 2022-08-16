################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/user/user.cpp \
../src/user/userchannelgroup.cpp \
../src/user/userchannelgroups.cpp \
../src/user/userclients.cpp \
../src/user/userdevices.cpp 

CPP_DEPS += \
./src/user/user.d \
./src/user/userchannelgroup.d \
./src/user/userchannelgroups.d \
./src/user/userclients.d \
./src/user/userdevices.d 

OBJS += \
./src/user/user.o \
./src/user/userchannelgroup.o \
./src/user/userchannelgroups.o \
./src/user/userclients.o \
./src/user/userdevices.o 


# Each subdirectory must supply rules for building sources it contributes
src/user/%.o: ../src/user/%.cpp src/user/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-user

clean-src-2f-user:
	-$(RM) ./src/user/user.d ./src/user/user.o ./src/user/userchannelgroup.d ./src/user/userchannelgroup.o ./src/user/userchannelgroups.d ./src/user/userchannelgroups.o ./src/user/userclients.d ./src/user/userclients.o ./src/user/userdevices.d ./src/user/userdevices.o

.PHONY: clean-src-2f-user

