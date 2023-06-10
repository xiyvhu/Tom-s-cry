PROJ   = ../../bin/tracker
OBJS   = $(patsubst %.cpp, %.o, $(wildcard ../common/*.cpp *.cpp))
CC     = g++
LINK   = g++
RM     = rm -fr
CFLAGS = -c -Wall \
		 -I/usr/include/acl-lib/acl_dpp \
		 `mysql_config --cflags` \
		 -I../common
LIBS   = -pthread -lacl_all `mysql_config --libs`

all: $(PROJ)

$(PROJ): $(OBJS)
	$(LINK) $^ $(LIBS) -o $@
	
.cpp.o:
	$(CC) $(CFLAGS) $^ -o $@
	
clean:
	$(RM) $(PROJ) $(OBJS)