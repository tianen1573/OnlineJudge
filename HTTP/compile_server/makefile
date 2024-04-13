compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp -pthread

.PHONY:clean
clean:
	rm -rf compile_server

# .PHONY:static
# static:
# 	g++ -o compile_server -std=c++11 compile_server.cc -static -ljsoncpp -pthread


# .PHONY:remove
# remove:
# 	rm -rf ./temp/*.exe
# 	rm -rf ./temp/*.compiler_error
# 	rm -rf ./temp/*.stderr
# 	rm -rf ./temp/*.stdout
# 	rm -rf ./temp/*.stdin
# 	rm -rf ./temp/*.cpp

# 	# rm -rf ./temp/code.exe
# 	# rm -rf ./temp/code.compiler_error
# 	# rm -rf ./temp/code.stderr
# 	# rm -rf ./temp/code.stdout
# 	# rm -rf ./temp/code.stdin

	