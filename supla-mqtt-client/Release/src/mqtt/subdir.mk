################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mqtt/mqtt_pal.c 

CPP_SRCS += \
../src/mqtt/mqtt.cpp \
../src/mqtt/mqtt_client.cpp 

OBJS += \
./src/mqtt/mqtt.o \
./src/mqtt/mqtt_client.o \
./src/mqtt/mqtt_pal.o 

C_DEPS += \
./src/mqtt/mqtt_pal.d 

CPP_DEPS += \
./src/mqtt/mqtt.d \
./src/mqtt/mqtt_client.d 


# Each subdirectory must supply rules for building sources it contributes
src/mqtt/%.o: ../src/mqtt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__NO_DATABASE -D__NO_USER -I$(SSLDIR)/include -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/mqtt/%.o: ../src/mqtt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__NO_USER -D__NO_DATABASE -I$(SSLDIR)/include -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


