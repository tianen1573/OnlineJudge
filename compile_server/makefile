compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp -pthread

.PHONY:clean
clean:
	rm -rf compile_server
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

	