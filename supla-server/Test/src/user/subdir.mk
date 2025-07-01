################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/user/abstract_user_dao.cpp \
../src/user/user.cpp \
../src/user/user_dao.cpp \
../src/user/userchannelgroup.cpp \
../src/user/userchannelgroups.cpp \
../src/user/userclients.cpp \
../src/user/userdevices.cpp \
../src/user/virtual_channel_updater_cyclictask.cpp \
../src/user/virtualchannel.cpp 

CPP_DEPS += \
./src/user/abstract_user_dao.d \
./src/user/user.d \
./src/user/user_dao.d \
./src/user/userchannelgroup.d \
./src/user/userchannelgroups.d \
./src/user/userclients.d \
./src/user/userdevices.d \
./src/user/virtual_channel_updater_cyclictask.d \
./src/user/virtualchannel.d 

OBJS += \
./src/user/abstract_user_dao.o \
./src/user/user.o \
./src/user/user_dao.o \
./src/user/userchannelgroup.o \
./src/user/userchannelgroups.o \
./src/user/userclients.o \
./src/user/userdevices.o \
./src/user/virtual_channel_updater_cyclictask.o \
./src/user/virtualchannel.o 


# Each subdirectory must supply rules for building sources it contributes
src/user/%.o: ../src/user/%.cpp src/user/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-user

clean-src-2f-user:
	-$(RM) ./src/user/abstract_user_dao.d ./src/user/abstract_user_dao.o ./src/user/user.d ./src/user/user.o ./src/user/user_dao.d ./src/user/user_dao.o ./src/user/userchannelgroup.d ./src/user/userchannelgroup.o ./src/user/userchannelgroups.d ./src/user/userchannelgroups.o ./src/user/userclients.d ./src/user/userclients.o ./src/user/userdevices.d ./src/user/userdevices.o ./src/user/virtual_channel_updater_cyclictask.d ./src/user/virtual_channel_updater_cyclictask.o ./src/user/virtualchannel.d ./src/user/virtualchannel.o

.PHONY: clean-src-2f-user

