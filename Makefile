MINISATDIR = minisat
#CPLEXDIR = ../CPLEX_Studio126

ifndef CPLEXDIR
$(error CPLEXDIR is not set)
endif

ABS_CPPFLAGS += -DGITHASH=$(GIT_HASH) -DGITDATE=$(GIT_DATE)

OBJFILES = obj/AbHS.o obj/main.o obj/WCNFParser.o obj/AbdInstance.o \
					 obj/Util.o
					 
ABS_CPPFLAGS += -O3 -std=c++11 -pthread -fPIC -m64 \
	-D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

ABD_DEBUG ?= 0
ifeq ($(ABD_DEBUG),0)
	ABS_CPPFLAGS += -DNDEBUG 
else
	ABS_CPPFLAGS += -ggdb -Og -DDEBUG
endif

WARNS = -pedantic -Wall -Wextra -W -Wpointer-arith -Wcast-align -Wwrite-strings \
 -Wdisabled-optimization -Wctor-dtor-privacy \
 -Wreorder -Woverloaded-virtual -Wsign-promo -Wsynth #\
 #-Wno-strict-aliasing -Wno-unused-parameter -Wnon-virtual-dtor \
 #-Wno-unknown-pragmas -Wno-cast-qual -Wno-shadow -Wno-redundant-decls

SATDIR    = $(MINISATDIR)
SAT_LNFLAGS	=	-lz
ABS_CPPFLAGS += -DSAT_MINISAT
SATDEP 	= minisat
SATOBJS = $(MINISATDIR)/build/release/minisat/core/Solver.o \
			$(MINISATDIR)/build/release/minisat/utils/System.o \
			$(MINISATDIR)/build/release/minisat/utils/Options.o
OBJFILES += obj/MinisatInterface.o				

CONCERTINCDIR = $(CPLEXDIR)/concert/include
CPLEXINCDIR   = $(CPLEXDIR)/cplex/include
CCOPT         = -fno-strict-aliasing -fexceptions -DIL_STD
CPLEX_LNDIR = $(shell dirname `find $(CPLEXDIR)/cplex -type f -name 'libcplex.a'`)
CONCERT_LNDIR = $(shell dirname `find $(CPLEXDIR)/concert -type f -name 'libconcert.a'`)
MIP_LNDIRS		=	-L$(CPLEX_LNDIR) -L$(CONCERT_LNDIR)
MIP_LNFLAGS		=	-lconcert -lilocplex -lcplex -lm -lpthread
ABS_CPPFLAGS += $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)
OBJFILES += obj/CPLEXInterface.o

CPP			 =	g++
EXE = bin/AbHS

.PHONY: all
all: $(EXE)

obj:	
	@-mkdir -p obj

.PHONY: minisat
minisat:
	@echo "Compiling minisat:"
	@make -s -C $(MINISATDIR) config prefix=.
	@make -s -C $(MINISATDIR) build/release/minisat/core/Solver.o
	@make -s -C $(MINISATDIR) build/release/minisat/utils/System.o
	@make -s -C $(MINISATDIR) build/release/minisat/utils/Options.o
	@echo "Minisat compiled."

-include	$(MAINDEP)

$(EXE):	$(SATDEP) $(MIPDEP) obj $(OBJFILES)	
	@mkdir -p bin
	@echo "-> linking $@"
	@$(CPP) $(ABS_CPPFLAGS) $(OBJFILES) $(SATOBJS) \
			$(SAT_LNDIRS) $(MIP_LNDIRS) $(SAT_LNFLAGS) $(MIP_LNFLAGS) $(MIP_FLAGS) -o $@

obj/%.o: src/%.cpp
	@echo "-> compiling $@"
	@$(CPP) $(ABS_CPPFLAGS) -I$(SATDIR) $(MIP_FLAGS) $(WARNS) -c $< -o $@
