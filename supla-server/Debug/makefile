################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/webhook/subdir.mk
-include src/user/subdir.mk
-include src/srpc/subdir.mk
-include src/scene/subdir.mk
-include src/mqtt/subdir.mk
-include src/json/subdir.mk
-include src/ipc/subdir.mk
-include src/http/subdir.mk
-include src/google/subdir.mk
-include src/distributedobjects/subdir.mk
-include src/device/call_handler/subdir.mk
-include src/device/subdir.mk
-include src/crypt_blowfish/subdir.mk
-include src/converter/subdir.mk
-include src/conn/call_handler/subdir.mk
-include src/conn/subdir.mk
-include src/client/call_handler/subdir.mk
-include src/client/subdir.mk
-include src/channeljsonconfig/subdir.mk
-include src/asynctask/subdir.mk
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
	$(CXX) -L/usr/include/mysql -L$(SSLDIR)/lib -pthread -lrt -o "supla-server" $(OBJS) $(USER_OBJS) $(LIBS) -lssl -l$(LIBMYSQL) -lcrypto
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) supla-server
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
