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

C_DEPS += \
./src/crypt_blowfish/crypt_blowfish.d \
./src/crypt_blowfish/crypt_gensalt.d \
./src/crypt_blowfish/wrapper.d 

OBJS += \
./src/crypt_blowfish/crypt_blowfish.o \
./src/crypt_blowfish/crypt_gensalt.o \
./src/crypt_blowfish/wrapper.o \
./src/crypt_blowfish/x86.o 


# Each subdirectory must supply rules for building sources it contributes
src/crypt_blowfish/%.o: ../src/crypt_blowfish/%.c src/crypt_blowfish/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(SSLDIR)/include -I../src/mqtt -I../src -I../src/user -I../src/device -I../src/client -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/crypt_blowfish/x86.o: ../src/crypt_blowfish/x86.S src/crypt_blowfish/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	gcc -c  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-crypt_blowfish

clean-src-2f-crypt_blowfish:
	-$(RM) ./src/crypt_blowfish/crypt_blowfish.d ./src/crypt_blowfish/crypt_blowfish.o ./src/crypt_blowfish/crypt_gensalt.d ./src/crypt_blowfish/crypt_gensalt.o ./src/crypt_blowfish/wrapper.d ./src/crypt_blowfish/wrapper.o ./src/crypt_blowfish/x86.o

.PHONY: clean-src-2f-crypt_blowfish

