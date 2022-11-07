# Quick and ok!

ARCH?=x86-64

BINNAME=express_router
CXXFLAGS=-std=c++20 -DRESTINIO_USE_BOOST_ASIO=shared
LDDFLAGS=-fmodules-ts

# Debug build? debug is default
ifdef DEBUG
CXXFLAGS+=-g 
DEST_DIR=${ARCH}/debug
MODE=debug
else
CXXFLAGS+=-O2
DEST_DIR=${ARCH}/release
MODE=release
endif
OBJ_DEST_DIR=build/${DEST_DIR}
BIN_DEST_DIR=bin/${DEST_DIR}
LIB_DEST_DIR=lib/${DEST_DIR}

COMMON=main.cpp 
SRC=${COMMON} 
OBJS=$(addprefix ${OBJ_DEST_DIR}/,$(notdir ${SRC:%.cpp=%.o}))
DEPS=$(addprefix ${OBJ_DEST_DIR}/,$(notdir ${SRC:%.cpp=%.d}))

include compiler_setup.${ARCH}

BASE_PACKAGE_PATH=/home/stud/ngk

FMT_PACKAGE=${BASE_PACKAGE_PATH}/fmt
FMT_PACKAGE_INC=${FMT_PACKAGE}/include
FMT_PACKAGE_LIBPATH=${FMT_PACKAGE}/${LIB_DEST_DIR}

HTTP_PARSER_PACKAGE=${BASE_PACKAGE_PATH}/http-parser
HTTP_PARSER_PACKAGE_INC=${HTTP_PARSER_PACKAGE}
HTTP_PARSER_PACKAGE_LIB=${HTTP_PARSER_PACKAGE}/${LIB_DEST_DIR}

JSON_DTO_PACKAGE=${BASE_PACKAGE_PATH}/json_dto
JSON_DTO_PACKAGE_INC=${JSON_DTO_PACKAGE}/dev

RAPIDJSON_PACKAGE=${BASE_PACKAGE_PATH}/rapidjson
RAPIDJSON_PACKAGE_INC=${RAPIDJSON_PACKAGE}/include

RESTINIO_PACKAGE=${BASE_PACKAGE_PATH}/restinio/dev
RESTINIO_PACKAGE_INC=${RESTINIO_PACKAGE}

LIBS=-lfmt -lhttp_parser -lpthread
LIBPATHS=-L${FMT_PACKAGE_LIBPATH} -L${HTTP_PARSER_PACKAGE_LIB}

# Setting compiler flags
CXXFLAGS+= -Wall -Iinclude  -D${OS} -I${RESTINIO_PACKAGE_INC} -I${FMT_PACKAGE_INC} -I${HTTP_PARSER_PACKAGE_INC} -I${JSON_DTO_PACKAGE_INC} -I${RAPIDJSON_PACKAGE_INC}
VPATH=src

all: info ${BIN_DEST_DIR}/${BINNAME}

info:
	@echo "Compiling for '${ARCH}' in '${MODE}' mode..."

${OBJ_DEST_DIR}/%.o: %.cpp
	@echo "Compiling "$<
	@${CXX} -c $< -o $@ ${CXXFLAGS}

${OBJ_DEST_DIR}/%.d: %.cpp
	@echo "Generating dependency for $<"
	@mkdir -p ${OBJ_DEST_DIR}
	@$(CXX) -MT${OBJ_DEST_DIR}/$(@:.d=.o) -MM $(CXXFLAGS) $^ > $@

${BIN_DEST_DIR}:
	@mkdir -p ${BIN_DEST_DIR}

${OBJ_DEST_DIR}:
	@mkdir -p ${OBJ_DEST_DIR}

${BIN_DEST_DIR}/${BINNAME}: ${OBJ_DEST_DIR} ${BIN_DEST_DIR} $(DEPS) ${OBJS} 
	@echo "Linking "${BINNAME}
	@${CXX} -o $@ ${OBJS} ${LIBS} ${LIBPATHS}


clean:
	@echo "Cleaning up..."
	@rm -f ${BIN_DEST_DIR}/${BINNAME} $(OBJS) $(DEPS)

distclean:
	@echo "Remove all build stuff..."
	@rm -rf build bin lib


.PHONY: clean distclean


-include $(DEPS)
