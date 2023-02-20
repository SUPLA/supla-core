################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/asynctask/abstract_asynctask.cpp \
../src/asynctask/abstract_asynctask_observer.cpp \
../src/asynctask/abstract_asynctask_search_condition.cpp \
../src/asynctask/abstract_asynctask_thread_pool.cpp \
../src/asynctask/asynctask_default_thread_pool.cpp \
../src/asynctask/asynctask_queue.cpp \
../src/asynctask/asynctask_state.cpp \
../src/asynctask/asynctask_thread_storage.cpp 

CPP_DEPS += \
./src/asynctask/abstract_asynctask.d \
./src/asynctask/abstract_asynctask_observer.d \
./src/asynctask/abstract_asynctask_search_condition.d \
./src/asynctask/abstract_asynctask_thread_pool.d \
./src/asynctask/asynctask_default_thread_pool.d \
./src/asynctask/asynctask_queue.d \
./src/asynctask/asynctask_state.d \
./src/asynctask/asynctask_thread_storage.d 

OBJS += \
./src/asynctask/abstract_asynctask.o \
./src/asynctask/abstract_asynctask_observer.o \
./src/asynctask/abstract_asynctask_search_condition.o \
./src/asynctask/abstract_asynctask_thread_pool.o \
./src/asynctask/asynctask_default_thread_pool.o \
./src/asynctask/asynctask_queue.o \
./src/asynctask/asynctask_state.o \
./src/asynctask/asynctask_thread_storage.o 


# Each subdirectory must supply rules for building sources it contributes
src/asynctask/%.o: ../src/asynctask/%.cpp src/asynctask/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-asynctask

clean-src-2f-asynctask:
	-$(RM) ./src/asynctask/abstract_asynctask.d ./src/asynctask/abstract_asynctask.o ./src/asynctask/abstract_asynctask_observer.d ./src/asynctask/abstract_asynctask_observer.o ./src/asynctask/abstract_asynctask_search_condition.d ./src/asynctask/abstract_asynctask_search_condition.o ./src/asynctask/abstract_asynctask_thread_pool.d ./src/asynctask/abstract_asynctask_thread_pool.o ./src/asynctask/asynctask_default_thread_pool.d ./src/asynctask/asynctask_default_thread_pool.o ./src/asynctask/asynctask_queue.d ./src/asynctask/asynctask_queue.o ./src/asynctask/asynctask_state.d ./src/asynctask/asynctask_state.o ./src/asynctask/asynctask_thread_storage.d ./src/asynctask/asynctask_thread_storage.o

.PHONY: clean-src-2f-asynctask

