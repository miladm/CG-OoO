CC=g++ -ggdb -O3 -Wl,--hash-style=sysv -shared
CXX = $(CC)
DEPS = instruction.h basicblock.h phraseblock.h global.h list.h utility.h basicblock.h phraseblock.h dot.h make_instruction.h make_basicblock.h make_superblock.h make_phraseblock.h loop.h dependencyTable.h regfile.h phrase.h logGen.h annotateTrace.h config.h stat.h listSchedule.h quickSort.h registerAllocate.h registerAllocate_sb.h interfNode.h dominator.h ssa.h variable.h dependencySetup.h
OBJ = main.o instruction.o basicblock.o phraseblock.o basicblock.o phraseblock.o dot.o make_instruction.o make_basicblock.o make_superblock.o make_phraseblock.o loop.o utility.o dependencyTable.o regFile.o phrase.o logGen.o annotateTrace.o config.o stat.o listSchedule.o quickSort.o registerAllocate.o registerAllocate_sb.o interfNode.o dominator.o ssa.o variable.o dependencySetup.o
LDFLAGS = -lstdc++ -lm
#CFLAGS= -O3 -ldconfig++ -I./ -ggdb
CFLAGS= -O3 -I./ -ggdb
%.o: %.c $(DEPS)
	$(CXX) $(APP_CPPFLAGS) -c -o $@ $< $(CFLAGS)

PhraseFormer: $(OBJ)
	gcc $(LDFLAGS) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o *.out *.csv *~ core *~ PhraseFormer
