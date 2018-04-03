################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/crypt_blowfish/crypt_blowfish.c \
../src/crypt_blowfish/crypt_gensalt.c \
../src/crypt_blowfish/wrapper.c 

S_UPPER_SRCS += \
../src/crypt_blowfish/x86.S 

OBJS += \
./src/crypt_blowfish/crypt_blowfish.o \
./src/crypt_blowfish/crypt_gensalt.o \
./src/crypt_blowfish/wrapper.o \
./src/crypt_blowfish/x86.o 

C_DEPS += \
./src/crypt_blowfish/crypt_blowfish.d \
./src/crypt_blowfish/crypt_gensalt.d \
./src/crypt_blowfish/wrapper.d 


# Each subdirectory must supply rules for building sources it contributes
src/crypt_blowfish/%.o: ../src/crypt_blowfish/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__DEBUG=1 -D__BCRYPT=1 -I$(SSLDIR)/include -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/crypt_blowfish/x86.o: ../src/crypt_blowfish/x86.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	gcc -c  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


