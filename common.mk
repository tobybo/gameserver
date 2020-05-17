
ifeq ($(DEBUG),true)
cc = g++ -std=c++11 -g
VERSION = debug
else
cc = g++ -std=c++11
VERSION = release
endif

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

DEPS = $(SRCS:.cpp=.d)

BIN := $(addprefix $(BUILD_ROOT)/,$(BIN))

LINK_OBJ_DIR = $(BUILD_ROOT)/app/link_obj
DEP_DIR = $(BUILD_ROOT)/app/dep

$(shell mkdir -p $(LINK_OBJ_DIR))
$(shell mkdir -p $(DEP_DIR))

OBJS := $(addprefix $(LINK_OBJ_DIR)/,$(OBJS))
DEPS := $(addprefix $(DEP_DIR)/,$(DEPS))

LINK_OBJ = $(wildcard $(LINK_OBJ_DIR)/*.o)
LINK_OBJ += $(OBJS)

all:$(DEPS) $(OBJS) $(BIN)

ifneq ("$(wildcard $(DEPS))","")
include $(DEPS)
endif

# .bin:.o
$(BIN):$(LINK_OBJ)
	#@echo "do: bin:linkobj $(LINK_OBJ)"
	@echo "--------------build $(VERSION) mode-------------------"
	$(cc) -o $@ $^ -l$(LIB_NAME)

# .o:.cpp
$(LINK_OBJ_DIR)/%.o:%.cpp
	@echo "do: obj:cpp"
	#$(cc) -I$(INCLUDE_PATH) -I /usr/include/mysql/ -o $@ -c $(filter %.cpp,$^)
	$(cc) -I$(INCLUDE_PATH) -o $@ -c $(filter %.cpp,$^)

$(DEP_DIR)/%.d:%.cpp
	@echo "do: dep:cpp"
	#echo -n $(LINK_OBJ_DIR)/ > $@
	#$(cc) -I$(INCLUDE_PATH) -I /usr/include/mysql/ -MM $^ >> $@
	$(cc) -I$(INCLUDE_PATH) -MM $^ >> $@


