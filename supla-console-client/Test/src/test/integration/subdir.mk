################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/integration/ActionExecutionIntegrationTest.cpp \
../src/test/integration/ConnectionIntegrationTest.cpp \
../src/test/integration/GetChannelBasicCfg.cpp \
../src/test/integration/GetChannelBasicCfgIntegrationTest.cpp \
../src/test/integration/GetVersionIntegrationTest.cpp \
../src/test/integration/IntegrationTest.cpp \
../src/test/integration/MySqlShell.cpp \
../src/test/integration/OAuthIntegrationTest.cpp \
../src/test/integration/ProperlyRegistered.cpp \
../src/test/integration/RegistrationIntegrationTest.cpp \
../src/test/integration/SetChannelCaptionIntegrationTest.cpp \
../src/test/integration/SetChannelFunctionIntegrationTest.cpp \
../src/test/integration/SetChannelGroupCaptionIntegrationTest.cpp \
../src/test/integration/SetLocationCaptionIntegrationTest.cpp \
../src/test/integration/SetRegistrationEnabledIntegrationTest.cpp \
../src/test/integration/SetSceneCaptionIntegrationTest.cpp \
../src/test/integration/SuperuserAuthorization.cpp \
../src/test/integration/SuperuserAuthorizationIntegrationTest.cpp 

CPP_DEPS += \
./src/test/integration/ActionExecutionIntegrationTest.d \
./src/test/integration/ConnectionIntegrationTest.d \
./src/test/integration/GetChannelBasicCfg.d \
./src/test/integration/GetChannelBasicCfgIntegrationTest.d \
./src/test/integration/GetVersionIntegrationTest.d \
./src/test/integration/IntegrationTest.d \
./src/test/integration/MySqlShell.d \
./src/test/integration/OAuthIntegrationTest.d \
./src/test/integration/ProperlyRegistered.d \
./src/test/integration/RegistrationIntegrationTest.d \
./src/test/integration/SetChannelCaptionIntegrationTest.d \
./src/test/integration/SetChannelFunctionIntegrationTest.d \
./src/test/integration/SetChannelGroupCaptionIntegrationTest.d \
./src/test/integration/SetLocationCaptionIntegrationTest.d \
./src/test/integration/SetRegistrationEnabledIntegrationTest.d \
./src/test/integration/SetSceneCaptionIntegrationTest.d \
./src/test/integration/SuperuserAuthorization.d \
./src/test/integration/SuperuserAuthorizationIntegrationTest.d 

OBJS += \
./src/test/integration/ActionExecutionIntegrationTest.o \
./src/test/integration/ConnectionIntegrationTest.o \
./src/test/integration/GetChannelBasicCfg.o \
./src/test/integration/GetChannelBasicCfgIntegrationTest.o \
./src/test/integration/GetVersionIntegrationTest.o \
./src/test/integration/IntegrationTest.o \
./src/test/integration/MySqlShell.o \
./src/test/integration/OAuthIntegrationTest.o \
./src/test/integration/ProperlyRegistered.o \
./src/test/integration/RegistrationIntegrationTest.o \
./src/test/integration/SetChannelCaptionIntegrationTest.o \
./src/test/integration/SetChannelFunctionIntegrationTest.o \
./src/test/integration/SetChannelGroupCaptionIntegrationTest.o \
./src/test/integration/SetLocationCaptionIntegrationTest.o \
./src/test/integration/SetRegistrationEnabledIntegrationTest.o \
./src/test/integration/SetSceneCaptionIntegrationTest.o \
./src/test/integration/SuperuserAuthorization.o \
./src/test/integration/SuperuserAuthorizationIntegrationTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/integration/%.o: ../src/test/integration/%.cpp src/test/integration/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__TEST=1 -DNOMYSQL=1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -I$(SSLDIR)/include -I../src/supla-client-lib -I../src/test -O0 -g3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-integration

clean-src-2f-test-2f-integration:
	-$(RM) ./src/test/integration/ActionExecutionIntegrationTest.d ./src/test/integration/ActionExecutionIntegrationTest.o ./src/test/integration/ConnectionIntegrationTest.d ./src/test/integration/ConnectionIntegrationTest.o ./src/test/integration/GetChannelBasicCfg.d ./src/test/integration/GetChannelBasicCfg.o ./src/test/integration/GetChannelBasicCfgIntegrationTest.d ./src/test/integration/GetChannelBasicCfgIntegrationTest.o ./src/test/integration/GetVersionIntegrationTest.d ./src/test/integration/GetVersionIntegrationTest.o ./src/test/integration/IntegrationTest.d ./src/test/integration/IntegrationTest.o ./src/test/integration/MySqlShell.d ./src/test/integration/MySqlShell.o ./src/test/integration/OAuthIntegrationTest.d ./src/test/integration/OAuthIntegrationTest.o ./src/test/integration/ProperlyRegistered.d ./src/test/integration/ProperlyRegistered.o ./src/test/integration/RegistrationIntegrationTest.d ./src/test/integration/RegistrationIntegrationTest.o ./src/test/integration/SetChannelCaptionIntegrationTest.d ./src/test/integration/SetChannelCaptionIntegrationTest.o ./src/test/integration/SetChannelFunctionIntegrationTest.d ./src/test/integration/SetChannelFunctionIntegrationTest.o ./src/test/integration/SetChannelGroupCaptionIntegrationTest.d ./src/test/integration/SetChannelGroupCaptionIntegrationTest.o ./src/test/integration/SetLocationCaptionIntegrationTest.d ./src/test/integration/SetLocationCaptionIntegrationTest.o ./src/test/integration/SetRegistrationEnabledIntegrationTest.d ./src/test/integration/SetRegistrationEnabledIntegrationTest.o ./src/test/integration/SetSceneCaptionIntegrationTest.d ./src/test/integration/SetSceneCaptionIntegrationTest.o ./src/test/integration/SuperuserAuthorization.d ./src/test/integration/SuperuserAuthorization.o ./src/test/integration/SuperuserAuthorizationIntegrationTest.d ./src/test/integration/SuperuserAuthorizationIntegrationTest.o

.PHONY: clean-src-2f-test-2f-integration

