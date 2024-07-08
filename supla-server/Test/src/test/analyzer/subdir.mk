################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/analyzer/ElectricityAnalyzerTest.cpp \
../src/test/analyzer/GeneralPurposeMeasurementAnalyzerTest.cpp \
../src/test/analyzer/SimpleStatisticsTest.cpp \
../src/test/analyzer/VoltageAberrationAnalyzerTest.cpp 

CPP_DEPS += \
./src/test/analyzer/ElectricityAnalyzerTest.d \
./src/test/analyzer/GeneralPurposeMeasurementAnalyzerTest.d \
./src/test/analyzer/SimpleStatisticsTest.d \
./src/test/analyzer/VoltageAberrationAnalyzerTest.d 

OBJS += \
./src/test/analyzer/ElectricityAnalyzerTest.o \
./src/test/analyzer/GeneralPurposeMeasurementAnalyzerTest.o \
./src/test/analyzer/SimpleStatisticsTest.o \
./src/test/analyzer/VoltageAberrationAnalyzerTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/analyzer/%.o: ../src/test/analyzer/%.cpp src/test/analyzer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-analyzer

clean-src-2f-test-2f-analyzer:
	-$(RM) ./src/test/analyzer/ElectricityAnalyzerTest.d ./src/test/analyzer/ElectricityAnalyzerTest.o ./src/test/analyzer/GeneralPurposeMeasurementAnalyzerTest.d ./src/test/analyzer/GeneralPurposeMeasurementAnalyzerTest.o ./src/test/analyzer/SimpleStatisticsTest.d ./src/test/analyzer/SimpleStatisticsTest.o ./src/test/analyzer/VoltageAberrationAnalyzerTest.d ./src/test/analyzer/VoltageAberrationAnalyzerTest.o

.PHONY: clean-src-2f-test-2f-analyzer

