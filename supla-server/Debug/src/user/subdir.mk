################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/user/clientcontainer.cpp \
../src/user/devicecontainer.cpp \
../src/user/user.cpp \
../src/user/userchannelgroup.cpp \
../src/user/userchannelgroups.cpp 

OBJS += \
./src/user/clientcontainer.o \
./src/user/devicecontainer.o \
./src/user/user.o \
./src/user/userchannelgroup.o \
./src/user/userchannelgroups.o 

CPP_DEPS += \
./src/user/clientcontainer.d \
./src/user/devicecontainer.d \
./src/user/user.d \
./src/user/userchannelgroup.d \
./src/user/userchannelgroups.d 


# Each subdirectory must supply rules for building sources it contributes
src/user/%.o: ../src/user/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I/usr/include/mysql -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


