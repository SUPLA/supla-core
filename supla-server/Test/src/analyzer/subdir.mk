################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/analyzer/simple_statistics.cpp \
../src/analyzer/voltage_analyzer.cpp \
../src/analyzer/voltage_analyzers.cpp 

CPP_DEPS += \
./src/analyzer/simple_statistics.d \
./src/analyzer/voltage_analyzer.d \
./src/analyzer/voltage_analyzers.d 

OBJS += \
./src/analyzer/simple_statistics.o \
./src/analyzer/voltage_analyzer.o \
./src/analyzer/voltage_analyzers.o 


# Each subdirectory must supply rules for building sources it contributes
src/analyzer/%.o: ../src/analyzer/%.cpp src/analyzer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-analyzer

clean-src-2f-analyzer:
	-$(RM) ./src/analyzer/simple_statistics.d ./src/analyzer/simple_statistics.o ./src/analyzer/voltage_analyzer.d ./src/analyzer/voltage_analyzer.o ./src/analyzer/voltage_analyzers.d ./src/analyzer/voltage_analyzers.o

.PHONY: clean-src-2f-analyzer

