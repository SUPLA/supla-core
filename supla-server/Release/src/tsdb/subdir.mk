################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/tsdb/tsdb_access_provider.cpp 

CPP_DEPS += \
./src/tsdb/tsdb_access_provider.d 

OBJS += \
./src/tsdb/tsdb_access_provider.o 


# Each subdirectory must supply rules for building sources it contributes
src/tsdb/%.o: ../src/tsdb/%.cpp src/tsdb/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__BCRYPT=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-tsdb

clean-src-2f-tsdb:
	-$(RM) ./src/tsdb/tsdb_access_provider.d ./src/tsdb/tsdb_access_provider.o

.PHONY: clean-src-2f-tsdb

