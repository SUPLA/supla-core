################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scene/abstract_scene_operations_dao.cpp \
../src/scene/scene_asynctask.cpp \
../src/scene/scene_operation.cpp \
../src/scene/scene_operations.cpp \
../src/scene/scene_operations_dao.cpp \
../src/scene/scene_search_condition.cpp \
../src/scene/scene_state.cpp 

CPP_DEPS += \
./src/scene/abstract_scene_operations_dao.d \
./src/scene/scene_asynctask.d \
./src/scene/scene_operation.d \
./src/scene/scene_operations.d \
./src/scene/scene_operations_dao.d \
./src/scene/scene_search_condition.d \
./src/scene/scene_state.d 

OBJS += \
./src/scene/abstract_scene_operations_dao.o \
./src/scene/scene_asynctask.o \
./src/scene/scene_operation.o \
./src/scene/scene_operations.o \
./src/scene/scene_operations_dao.o \
./src/scene/scene_search_condition.o \
./src/scene/scene_state.o 


# Each subdirectory must supply rules for building sources it contributes
src/scene/%.o: ../src/scene/%.cpp src/scene/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-scene

clean-src-2f-scene:
	-$(RM) ./src/scene/abstract_scene_operations_dao.d ./src/scene/abstract_scene_operations_dao.o ./src/scene/scene_asynctask.d ./src/scene/scene_asynctask.o ./src/scene/scene_operation.d ./src/scene/scene_operation.o ./src/scene/scene_operations.d ./src/scene/scene_operations.o ./src/scene/scene_operations_dao.d ./src/scene/scene_operations_dao.o ./src/scene/scene_search_condition.d ./src/scene/scene_search_condition.o ./src/scene/scene_state.d ./src/scene/scene_state.o

.PHONY: clean-src-2f-scene

