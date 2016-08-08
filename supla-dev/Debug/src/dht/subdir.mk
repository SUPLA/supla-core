################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dht/common_dht_read.c \
../src/dht/pi_2_dht_read.c \
../src/dht/pi_2_mmio.c 

OBJS += \
./src/dht/common_dht_read.o \
./src/dht/pi_2_dht_read.o \
./src/dht/pi_2_mmio.o 

C_DEPS += \
./src/dht/common_dht_read.d \
./src/dht/pi_2_dht_read.d \
./src/dht/pi_2_mmio.d 


# Each subdirectory must supply rules for building sources it contributes
src/dht/%.o: ../src/dht/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -D__W1_SIMULATE=1 -D__GPIO_SIMULATE=1 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


