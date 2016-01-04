################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Android/jni/supla-client_jni.c 

OBJS += \
./Android/jni/supla-client_jni.o 

C_DEPS += \
./Android/jni/supla-client_jni.d 


# Each subdirectory must supply rules for building sources it contributes
Android/jni/%.o: ../Android/jni/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(GCC) $(PARAMS) -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


