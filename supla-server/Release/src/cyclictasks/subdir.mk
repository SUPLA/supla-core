################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cyclictasks/abstract_cyclictask.cpp \
../src/cyclictasks/agent.cpp 

CPP_DEPS += \
./src/cyclictasks/abstract_cyclictask.d \
./src/cyclictasks/agent.d 

OBJS += \
./src/cyclictasks/abstract_cyclictask.o \
./src/cyclictasks/agent.o 


# Each subdirectory must supply rules for building sources it contributes
src/cyclictasks/%.o: ../src/cyclictasks/%.cpp src/cyclictasks/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-cyclictasks

clean-src-2f-cyclictasks:
	-$(RM) ./src/cyclictasks/abstract_cyclictask.d ./src/cyclictasks/abstract_cyclictask.o ./src/cyclictasks/agent.d ./src/cyclictasks/agent.o

.PHONY: clean-src-2f-cyclictasks

