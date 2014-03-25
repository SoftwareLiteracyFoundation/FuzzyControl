
CC   = g++
OBJ  = FuzzyControl.o ParseFCL.o FCL_AccessoryFunc.o FCL_IO_Func.o \
       FuzzyInput.o FuzzyOutput.o FuzzyRules.o ConsoleMsg.o FuzzyControlAPI.o 
LIBS =  -L/usr/lib
INCS =  
BIN  = libfcl.a
CFLAGS = $(INCS) -ggdb

all:	$(BIN)
	ar -rcs $(BIN) $(OBJ)
	g++ RunFCL.cc -o RunFCL $(BIN)
	cp RunFCL ../

clean:
	rm -f $(OBJ) $(BIN) makefile.bak RunFCL.o RunFCL *.out

$(BIN): $(OBJ)

FuzzyControl.o: FuzzyControl.cc
	$(CC) -c FuzzyControl.cc $(CFLAGS)

ParseFCL.o: ParseFCL.cc
	$(CC) -c ParseFCL.cc $(CFLAGS)

FCL_IO_Func.o: FCL_IO_Func.cc
	$(CC) -c FCL_IO_Func.cc $(CFLAGS)

FCL_AccessoryFunc.o: FCL_AccessoryFunc.cc
	$(CC) -c FCL_AccessoryFunc.cc $(CFLAGS)

FuzzyInput.o: FuzzyInput.cc
	$(CC) -c FuzzyInput.cc $(CFLAGS)

FuzzyOutput.o: FuzzyOutput.cc
	$(CC) -c FuzzyOutput.cc $(CFLAGS)

FuzzyRules.o: FuzzyRules.cc
	$(CC) -c FuzzyRules.cc $(CFLAGS)

ConsoleMsg.o: ConsoleMsg.cc
	$(CC) -c ConsoleMsg.cc $(CFLAGS)

FuzzyControlAPI.o: FuzzyControlAPI.cc
	$(CC) -c FuzzyControlAPI.cc $(CFLAGS)

SRCS = `echo ${OBJ} | sed -e 's/.o /.cc /g'`
depend:
	@echo ${SRCS}
	makedepend -Y $(SRCS)

# DO NOT DELETE

FuzzyControl.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FuzzyControl.o: FuzzyRules.h FCLL_Version.h
ParseFCL.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
ParseFCL.o: FuzzyRules.h
FCL_AccessoryFunc.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FCL_AccessoryFunc.o: FuzzyRules.h
FCL_IO_Func.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FCL_IO_Func.o: FuzzyRules.h
FuzzyInput.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FuzzyInput.o: FuzzyRules.h
FuzzyOutput.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FuzzyOutput.o: FuzzyRules.h
FuzzyRules.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
FuzzyRules.o: FuzzyRules.h
ConsoleMsg.o: FuzzyControl.h FCL_Keyword.h FuzzyInput.h FuzzyOutput.h
ConsoleMsg.o: FuzzyRules.h
