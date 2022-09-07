################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/srpc/abstract_srpc_adapter.cpp \
../src/srpc/srpc_adapter.cpp 

C_SRCS += \
../src/srpc/srpc.c 

CPP_DEPS += \
./src/srpc/abstract_srpc_adapter.d \
./src/srpc/srpc_adapter.d 

C_DEPS += \
./src/srpc/srpc.d 

OBJS += \
./src/srpc/abstract_srpc_adapter.o \
./src/srpc/srpc.o \
./src/srpc/srpc_adapter.o 


# Each subdirectory must supply rules for building sources it contributes
src/srpc/%.o: ../src/srpc/%.cpp src/srpc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/srpc/%.o: ../src/srpc/%.c src/srpc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-srpc

clean-src-2f-srpc:
	-$(RM) ./src/srpc/abstract_srpc_adapter.d ./src/srpc/abstract_srpc_adapter.o ./src/srpc/srpc.d ./src/srpc/srpc.o ./src/srpc/srpc_adapter.d ./src/srpc/srpc_adapter.o

.PHONY: clean-src-2f-srpc

