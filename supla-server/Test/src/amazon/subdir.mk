################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/amazon/alexa.cpp \
../src/amazon/alexaclient.cpp \
../src/amazon/alexaresponserequest.cpp 

OBJS += \
./src/amazon/alexa.o \
./src/amazon/alexaclient.o \
./src/amazon/alexaresponserequest.o 

CPP_DEPS += \
./src/amazon/alexa.d \
./src/amazon/alexaclient.d \
./src/amazon/alexaresponserequest.d 


# Each subdirectory must supply rules for building sources it contributes
src/amazon/%.o: ../src/amazon/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__OPENSSL_TOOLS=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I/usr/include/mysql -I../src -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


