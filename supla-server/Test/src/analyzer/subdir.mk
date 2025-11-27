################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/analyzer/abstract_data_analyzer.cpp \
../src/analyzer/data_analyzer_factory.cpp \
../src/analyzer/electricity_analyzer.cpp \
../src/analyzer/general_purpose_measurement_analyzer.cpp \
../src/analyzer/simple_statistics.cpp \
../src/analyzer/voltage_aberration_analyzer.cpp 

CPP_DEPS += \
./src/analyzer/abstract_data_analyzer.d \
./src/analyzer/data_analyzer_factory.d \
./src/analyzer/electricity_analyzer.d \
./src/analyzer/general_purpose_measurement_analyzer.d \
./src/analyzer/simple_statistics.d \
./src/analyzer/voltage_aberration_analyzer.d 

OBJS += \
./src/analyzer/abstract_data_analyzer.o \
./src/analyzer/data_analyzer_factory.o \
./src/analyzer/electricity_analyzer.o \
./src/analyzer/general_purpose_measurement_analyzer.o \
./src/analyzer/simple_statistics.o \
./src/analyzer/voltage_aberration_analyzer.o 


# Each subdirectory must supply rules for building sources it contributes
src/analyzer/%.o: ../src/analyzer/%.cpp src/analyzer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++14 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-analyzer

clean-src-2f-analyzer:
	-$(RM) ./src/analyzer/abstract_data_analyzer.d ./src/analyzer/abstract_data_analyzer.o ./src/analyzer/data_analyzer_factory.d ./src/analyzer/data_analyzer_factory.o ./src/analyzer/electricity_analyzer.d ./src/analyzer/electricity_analyzer.o ./src/analyzer/general_purpose_measurement_analyzer.d ./src/analyzer/general_purpose_measurement_analyzer.o ./src/analyzer/simple_statistics.d ./src/analyzer/simple_statistics.o ./src/analyzer/voltage_aberration_analyzer.d ./src/analyzer/voltage_aberration_analyzer.o

.PHONY: clean-src-2f-analyzer

