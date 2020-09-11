################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/webhook/statewebhookclient.cpp \
../src/webhook/statewebhookcredentials.cpp \
../src/webhook/statewebhookrequest.cpp \
../src/webhook/webhookbasicclient.cpp \
../src/webhook/webhookbasiccredentials.cpp 

OBJS += \
./src/webhook/statewebhookclient.o \
./src/webhook/statewebhookcredentials.o \
./src/webhook/statewebhookrequest.o \
./src/webhook/webhookbasicclient.o \
./src/webhook/webhookbasiccredentials.o 

CPP_DEPS += \
./src/webhook/statewebhookclient.d \
./src/webhook/statewebhookcredentials.d \
./src/webhook/statewebhookrequest.d \
./src/webhook/webhookbasicclient.d \
./src/webhook/webhookbasiccredentials.d 


# Each subdirectory must supply rules for building sources it contributes
src/webhook/%.o: ../src/webhook/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


