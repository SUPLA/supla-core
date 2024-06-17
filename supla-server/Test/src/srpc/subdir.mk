################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/srpc/abstract_srpc_adapter.cpp \
../src/srpc/abstract_srpc_call_handler.cpp \
../src/srpc/abstract_srpc_call_handler_collection.cpp \
../src/srpc/multipart_call.cpp \
../src/srpc/multipart_call_store.cpp \
../src/srpc/srpc_adapter.cpp 

C_SRCS += \
../src/srpc/srpc.c 

CPP_DEPS += \
./src/srpc/abstract_srpc_adapter.d \
./src/srpc/abstract_srpc_call_handler.d \
./src/srpc/abstract_srpc_call_handler_collection.d \
./src/srpc/multipart_call.d \
./src/srpc/multipart_call_store.d \
./src/srpc/srpc_adapter.d 

C_DEPS += \
./src/srpc/srpc.d 

OBJS += \
./src/srpc/abstract_srpc_adapter.o \
./src/srpc/abstract_srpc_call_handler.o \
./src/srpc/abstract_srpc_call_handler_collection.o \
./src/srpc/multipart_call.o \
./src/srpc/multipart_call_store.o \
./src/srpc/srpc.o \
./src/srpc/srpc_adapter.o 


# Each subdirectory must supply rules for building sources it contributes
src/srpc/%.o: ../src/srpc/%.cpp src/srpc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/srpc/%.o: ../src/srpc/%.c src/srpc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/asynctask -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-srpc

clean-src-2f-srpc:
	-$(RM) ./src/srpc/abstract_srpc_adapter.d ./src/srpc/abstract_srpc_adapter.o ./src/srpc/abstract_srpc_call_handler.d ./src/srpc/abstract_srpc_call_handler.o ./src/srpc/abstract_srpc_call_handler_collection.d ./src/srpc/abstract_srpc_call_handler_collection.o ./src/srpc/multipart_call.d ./src/srpc/multipart_call.o ./src/srpc/multipart_call_store.d ./src/srpc/multipart_call_store.o ./src/srpc/srpc.d ./src/srpc/srpc.o ./src/srpc/srpc_adapter.d ./src/srpc/srpc_adapter.o

.PHONY: clean-src-2f-srpc

