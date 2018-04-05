################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/eh.c \
../src/lck.c \
../src/log.c \
../src/proto.c \
../src/srpc.c 

CPP_SRCS += \
../src/supla-afl.cpp 

OBJS += \
./src/eh.o \
./src/lck.o \
./src/log.o \
./src/proto.o \
./src/srpc.o \
./src/supla-afl.o 

C_DEPS += \
./src/eh.d \
./src/lck.d \
./src/log.d \
./src/proto.d \
./src/srpc.d 

CPP_DEPS += \
./src/supla-afl.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	/usr/src/afl/afl-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	/usr/src/afl/afl-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


