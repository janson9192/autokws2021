
# headers:
ver    = #debug

HDRS    = infra.h \
	htkfile.h \
	dataset.h \
	dtw.h \
    distance.h \
    feature.h \
    util.h

# sources:
SRCS    = htkfile.cc \
    dataset.cc \
    dtw.cc \
    distance.cc \
	feature.cc \
    util.cc

CC      = g++

ifeq ($(ver), debug)
	CFLAGS = -g -std=c++0x -Iinfra2 -I.
	LFLAGS = -g -std=c++0x -pthread -Linfra2 -linfra 
else
	CFLAGS = -O3 -std=c++0x -Iinfra2 -I.
	LFLAGS = -O3 -std=c++0x -pthread -Linfra2 -linfra 
endif

OBJS = $(SRCS:.cc=.o)


#==========================================================#

all: dtw_std dtw_std_phrase i_dtw_std i_dtw_std_phrase template_avg template_avg_hierarchical template_avg_alitolong template_avg_forget

dtw_std: dtw_std.o $(OBJS) 
	$(CC) dtw_std.o $(OBJS) $(LFLAGS) -o dtw_std

dtw_std_phrase: dtw_std_phrase.o $(OBJS) 
	$(CC) dtw_std_phrase.o $(OBJS) $(LFLAGS) -o dtw_std_phrase

i_dtw_std: i_dtw_std.o $(OBJS) 
	$(CC) i_dtw_std.o $(OBJS) $(LFLAGS) -o i_dtw_std

i_dtw_std_phrase: i_dtw_std_phrase.o $(OBJS) 
	$(CC) i_dtw_std_phrase.o $(OBJS) $(LFLAGS) -o i_dtw_std_phrase

template_avg: template_avg.o $(OBJS)
	$(CC) template_avg.o $(OBJS) $(LFLAGS) -o template_avg

template_avg_hierarchical: template_avg_hierarchical.o $(OBJS)
	$(CC) template_avg_hierarchical.o $(OBJS) $(LFLAGS) -o template_avg_hierarchical

template_avg_alitolong: template_avg_alitolong.o $(OBJS)
	$(CC) template_avg_alitolong.o $(OBJS) $(LFLAGS) -o template_avg_alitolong

template_avg_forget: template_avg_forget.o $(OBJS)
	$(CC) template_avg_forget.o $(OBJS) $(LFLAGS) -o template_avg_forget

%.o: %.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o dtw_std template_avg template_avg_hierarchical template_avg_alitolong template_avg_forget

