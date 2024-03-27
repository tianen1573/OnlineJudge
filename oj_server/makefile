oj_server:oj_server.cc
	g++ -o $@ $^ -I./mysqlInclude -L./mysqlLib -std=c++11 -lpthread -lctemplate -ljsoncpp -lmysqlclient

.PHONY:clean
clean:
	rm -rf oj_server