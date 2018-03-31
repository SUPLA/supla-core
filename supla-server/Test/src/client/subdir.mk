################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client/client.cpp \
../src/client/clientchannel.cpp \
../src/client/clientchannelgroup.cpp \
../src/client/clientchannelgroups.cpp \
../src/client/clientchannels.cpp \
../src/client/clientlocation.cpp \
../src/client/clientobjcontainer.cpp \
../src/client/clientobjcontaineritem.cpp 

OBJS += \
./src/client/client.o \
./src/client/clientchannel.o \
./src/client/clientchannelgroup.o \
./src/client/clientchannelgroups.o \
./src/client/clientchannels.o \
./src/client/clientlocation.o \
./src/client/clientobjcontainer.o \
./src/client/clientobjcontaineritem.o 

CPP_DEPS += \
./src/client/client.d \
./src/client/clientchannel.d \
./src/client/clientchannelgroup.d \
./src/client/clientchannelgroups.d \
./src/client/clientchannels.d \
./src/client/clientlocation.d \
./src/client/clientobjcontainer.d \
./src/client/clientobjcontaineritem.d 


# Each subdirectory must supply rules for building sources it contributes
src/client/%.o: ../src/client/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__OPEN_SSL=1 -D__DEBUG=1 -D__BCRYPT=1 -I/usr/include/mysql -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


