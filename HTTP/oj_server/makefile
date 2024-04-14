oj_server:oj_server.cc
	g++ -o $@ $^  -std=c++11 -lpthread -lctemplate -ljsoncpp \
	-L../lib/mysqlLib -lmysqlclient

.PHONY:clean
clean:
	rm -rf oj_server