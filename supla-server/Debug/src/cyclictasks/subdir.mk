################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cyclictasks/abstract_auto_gate_closing.cpp \
../src/cyclictasks/abstract_auto_gate_closing_dao.cpp \
../src/cyclictasks/abstract_cyclictask.cpp \
../src/cyclictasks/agent.cpp \
../src/cyclictasks/auto_gate_closing.cpp \
../src/cyclictasks/auto_gate_closing_dao.cpp 

CPP_DEPS += \
./src/cyclictasks/abstract_auto_gate_closing.d \
./src/cyclictasks/abstract_auto_gate_closing_dao.d \
./src/cyclictasks/abstract_cyclictask.d \
./src/cyclictasks/agent.d \
./src/cyclictasks/auto_gate_closing.d \
./src/cyclictasks/auto_gate_closing_dao.d 

OBJS += \
./src/cyclictasks/abstract_auto_gate_closing.o \
./src/cyclictasks/abstract_auto_gate_closing_dao.o \
./src/cyclictasks/abstract_cyclictask.o \
./src/cyclictasks/agent.o \
./src/cyclictasks/auto_gate_closing.o \
./src/cyclictasks/auto_gate_closing_dao.o 


# Each subdirectory must supply rules for building sources it contributes
src/cyclictasks/%.o: ../src/cyclictasks/%.cpp src/cyclictasks/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-cyclictasks

clean-src-2f-cyclictasks:
	-$(RM) ./src/cyclictasks/abstract_auto_gate_closing.d ./src/cyclictasks/abstract_auto_gate_closing.o ./src/cyclictasks/abstract_auto_gate_closing_dao.d ./src/cyclictasks/abstract_auto_gate_closing_dao.o ./src/cyclictasks/abstract_cyclictask.d ./src/cyclictasks/abstract_cyclictask.o ./src/cyclictasks/agent.d ./src/cyclictasks/agent.o ./src/cyclictasks/auto_gate_closing.d ./src/cyclictasks/auto_gate_closing.o ./src/cyclictasks/auto_gate_closing_dao.d ./src/cyclictasks/auto_gate_closing_dao.o

.PHONY: clean-src-2f-cyclictasks

