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
	$(CC) -D__OPENSSL_TOOLS=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
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

