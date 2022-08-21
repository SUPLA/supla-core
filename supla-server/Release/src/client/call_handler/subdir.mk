################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client/call_handler/register_client.cpp \
../src/client/call_handler/register_client_b.cpp 

CPP_DEPS += \
./src/client/call_handler/register_client.d \
./src/client/call_handler/register_client_b.d 

OBJS += \
./src/client/call_handler/register_client.o \
./src/client/call_handler/register_client_b.o 


# Each subdirectory must supply rules for building sources it contributes
src/client/call_handler/%.o: ../src/client/call_handler/%.cpp src/client/call_handler/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-client-2f-call_handler

clean-src-2f-client-2f-call_handler:
	-$(RM) ./src/client/call_handler/register_client.d ./src/client/call_handler/register_client.o ./src/client/call_handler/register_client_b.d ./src/client/call_handler/register_client_b.o

.PHONY: clean-src-2f-client-2f-call_handler

