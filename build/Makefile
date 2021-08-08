DEBUG =
CXX = g++
CFLAGS = -std=c++17 -O3 -w ${DEBUG} -g # -Wall -g -O2

TARGET = server
OBJS = ../code/common/*.cc ../code/netlib/*.cc ../code/httpserver/*.cc \
       ../code/httpserver/dispatcher/*.cc ../code/httpserver/handler/*.cc \
       ../code/httpserver/http/*.cc ../code/httpserver/utils/StringUtils.cc \
	   ../code/httpserver/utils/URLEncodeUtil.cc \
	   ../code/httpserver/interceptor/*.cc ../code/main.cc 

ALL: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)

.PHONY: clean ALL


