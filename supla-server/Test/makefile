################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/webhook/subdir.mk
-include src/vbt/subdir.mk
-include src/user/subdir.mk
-include src/test/webhook/subdir.mk
-include src/test/vbt/subdir.mk
-include src/test/srpc/subdir.mk
-include src/test/scene/subdir.mk
-include src/test/push/subdir.mk
-include src/test/mqtt/subdir.mk
-include src/test/jsonconfig/device/subdir.mk
-include src/test/jsonconfig/channel/subdir.mk
-include src/test/jsonconfig/subdir.mk
-include src/test/ipc/subdir.mk
-include src/test/integration/webhook/subdir.mk
-include src/test/integration/vbt/subdir.mk
-include src/test/integration/schedule/subdir.mk
-include src/test/integration/scene/subdir.mk
-include src/test/integration/push/subdir.mk
-include src/test/integration/mqtt/subdir.mk
-include src/test/integration/google/subdir.mk
-include src/test/integration/datalogger/subdir.mk
-include src/test/integration/cyclictasks/subdir.mk
-include src/test/integration/client/subdir.mk
-include src/test/integration/amazon/subdir.mk
-include src/test/integration/subdir.mk
-include src/test/http/subdir.mk
-include src/test/google/subdir.mk
-include src/test/doubles/webhook/subdir.mk
-include src/test/doubles/scene/subdir.mk
-include src/test/doubles/push/subdir.mk
-include src/test/doubles/mqtt/subdir.mk
-include src/test/doubles/jsonconfig/channel/subdir.mk
-include src/test/doubles/jsonconfig/subdir.mk
-include src/test/doubles/ipc/subdir.mk
-include src/test/doubles/integration/mqtt/subdir.mk
-include src/test/doubles/integration/asynctask/subdir.mk
-include src/test/doubles/http/subdir.mk
-include src/test/doubles/google/subdir.mk
-include src/test/doubles/distributedobjects/subdir.mk
-include src/test/doubles/device/extended_value/subdir.mk
-include src/test/doubles/device/subdir.mk
-include src/test/doubles/db/subdir.mk
-include src/test/doubles/cyclictasks/subdir.mk
-include src/test/doubles/conn/subdir.mk
-include src/test/doubles/client/subdir.mk
-include src/test/doubles/asynctask/subdir.mk
-include src/test/doubles/amazon/subdir.mk
-include src/test/doubles/actions/subdir.mk
-include src/test/doubles/subdir.mk
-include src/test/distributedobjects/subdir.mk
-include src/test/device/value/subdir.mk
-include src/test/device/extended_value/subdir.mk
-include src/test/device/subdir.mk
-include src/test/cyclictasks/subdir.mk
-include src/test/conn/subdir.mk
-include src/test/client/subdir.mk
-include src/test/asynctask/subdir.mk
-include src/test/analyzer/subdir.mk
-include src/test/amazon/subdir.mk
-include src/test/actions/subdir.mk
-include src/test/subdir.mk
-include src/srpc/subdir.mk
-include src/schedule/subdir.mk
-include src/scene/subdir.mk
-include src/push/subdir.mk
-include src/mqtt/subdir.mk
-include src/jsonconfig/device/subdir.mk
-include src/jsonconfig/channel/subdir.mk
-include src/jsonconfig/subdir.mk
-include src/json/subdir.mk
-include src/ipc/subdir.mk
-include src/http/subdir.mk
-include src/google/subdir.mk
-include src/distributedobjects/subdir.mk
-include src/device/value/subdir.mk
-include src/device/extended_value/subdir.mk
-include src/device/call_handler/subdir.mk
-include src/device/subdir.mk
-include src/db/subdir.mk
-include src/datalogger/subdir.mk
-include src/cyclictasks/subdir.mk
-include src/crypt_blowfish/subdir.mk
-include src/converter/subdir.mk
-include src/conn/call_handler/subdir.mk
-include src/conn/subdir.mk
-include src/client/call_handler/subdir.mk
-include src/client/subdir.mk
-include src/asynctask/subdir.mk
-include src/analyzer/subdir.mk
-include src/amazon/subdir.mk
-include src/actions/subdir.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := supla-server
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: supla-server

# Tool invocations
supla-server: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++ -L/usr/include/mysql -L$(SSLDIR)/lib -pthread -lrt -o "supla-server" $(OBJS) $(USER_OBJS) $(LIBS) -lssl -lcurl -lgtest -lgmock -l$(LIBMYSQL) -lcrypto
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) supla-server
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
