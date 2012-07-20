BOOST_LIB_PATH		?= /usr/local/lib
BOOST_INCLUDE_PATH ?= /usr/local/include

CFLAGS = -O2 -I$(BOOST_INCLUDE_PATH)
LDFLAGS = -L$(BOOST_LIB_PATH)

CXX		?= c++
SHARED  ?= "1"

ifeq ($(SHARED), 1)
	LDFLAGS := $(LDFLAGS)  -lwebsocketpp -lboost_system -lboost_date_time -lboost_regex -lboost_unit_test_framework
else
	LDFLAGS := $(LDFLAGS)  ../../libwebsocketpp.a $(BOOST_LIB_PATH)/libboost_system.a $(BOOST_LIB_PATH)/libboost_date_time.a $(BOOST_LIB_PATH)/libboost_regex.a $(BOOST_LIB_PATH)/libboost_unit_test_framework.a
endif

all: hybi log parse perf

hybi: hybi_util.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

log: logging.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

parse: parsing.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

perf: uri_perf.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $^

test: all
	./hybi
	./log
	./parse
	./perf

# cleanup by removing generated files
#
.PHONY:		clean
clean:
		rm -f *.o tests hybi log parse perf
