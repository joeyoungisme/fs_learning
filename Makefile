TARGET=main

HEADER+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard *.h)))
HEADER+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard libjoe/*.h)))
HEADER+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard fat_core/*.h)))
HEADER+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard littlefs/*.h)))

OBJ+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard *.c)))
OBJ+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard libjoe/*.c)))
OBJ+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard fat_core/*.c)))
OBJ+=$(filter-out $(addsuffix .o,$(TARGET)),$(patsubst %.c,%.o,$(wildcard littlefs/*.c)))

# example : 
# OBJ+=$(patsubst %.c,%.o,$(wildcard Material/*.c))
# OBJ+=$(patsubst %.c,%.o,$(wildcard Material/Library/*.c))

INCLUDE+= -I $(PWD)/
INCLUDE+= -I $(PWD)/libjoe/
INCLUDE+= -I $(PWD)/fat_core/
INCLUDE+= -I $(PWD)/littlefs/

LIBS+= -lpthread -pthread

CFLAG+=
# CFLAG+=-DSTEP_BY_STEP
CFLAG+=-DDEBUG_FLAG

BUILD=build

all: $(BUILD) $(HEADER) $(OBJ) $(TARGET)

build:
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/libjoe
	mkdir -p $(BUILD)/fat_core
	mkdir -p $(BUILD)/littlefs

$(TARGET): $(OBJ)
	$(CC) $(INCLUDE) $(CFLAG) $(addprefix $(BUILD)/,$(OBJ)) -o $@ $@.c $(LIBS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAG) -c -o $(BUILD)/$@ $< $(LIBS)

clean:
	rm -f $(TARGET)
	rm -rf build
	rm -f *.o
