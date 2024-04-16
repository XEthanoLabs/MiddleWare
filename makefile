NAME_MESSAGEBROKER = MessageBroker.exe
NAME_CLIENTA = ClientA.exe
NAME_CLIENTB = ClientB.exe
NAME_CLIENTC = ClientC.exe

CC = 		g++
CF =		-g -Wall -Wextra
CFI = 		-I . -I ~/repos/boost_1_84_0/

OBJ_PATH = ./obj/
BUILD_PATH = ./build/

SRC_MB = 	MessageBroker/MessageBroker.cpp \
			MessageBroker/ConnectedClient.cpp \
			MessageBroker/MiscFuncs.cpp \
			MessageBroker/TopicChat.cpp

SRC_CA = 	ClientA/ClientA.cpp
SRC_CB = 	ClientB/ClientB.cpp
SRC_CC = 	ClientC/ClientC.cpp

OBJ_MB =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_MB:.cpp=.obj)))
OBJ_CA =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CA:.cpp=.obj)))
OBJ_CB =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CB:.cpp=.obj)))
OBJ_CC =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CC:.cpp=.obj)))

$(OBJ_PATH)%.obj:MessageBroker/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

$(OBJ_PATH)%.obj:ClientA/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

$(OBJ_PATH)%.obj:ClientB/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

$(OBJ_PATH)%.obj:ClientC/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

clean:
	@echo "$(OBJ_MB)"

all: $(NAME_MESSAGEBROKER) $(NAME_CLIENTA) $(NAME_CLIENTB) $(NAME_CLIENTC)

$(NAME_MESSAGEBROKER):	$(OBJ_MB)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_MESSAGEBROKER) $(OBJ_MB)

$(NAME_CLIENTA):	$(OBJ_CA)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTA) $(OBJ_CA)

$(NAME_CLIENTB):	$(OBJ_CB)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTB) $(OBJ_CB)

$(NAME_CLIENTC):	$(OBJ_CC)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTC) $(OBJ_CC)
