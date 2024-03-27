-rw-rw-r-- 1 qlz qlz    2692 Mar 12 15:58 compiler.hpp
-rw-rw-r-- 1 qlz qlz    6292 Mar 12 20:38 compile_run.hpp
-rwxrwxr-x 1 qlz qlz 1237768 Mar 12 20:23 compile_server
-rw-rw-r-- 1 qlz qlz    1049 Mar 12 17:39 compile_server.cc
-rw-rw-r-- 1 qlz qlz     466 Mar  8 22:24 makefile
-rw-rw-r-- 1 qlz qlz       0 Mar 12 20:39 README.md
-rw-rw-r-- 1 qlz qlz    3928 Mar  7 20:16 runner.hpp
drwxrwxrwx 2 qlz qlz    4096 Mar 12 20:38 temp
drwxrwxr-x 2 qlz qlz    4096 Mar  8 22:46 wwwroot

以上是compile_server目录的信息，其中只有temp目录具有other的全部权限，因此，在启动compile_server服务时，我们使用非root，非qlz账号进行启动，这样，当第三方代码运行时，仅仅允许在temp目录进行文件的创建和删除。
当启动compile_server时，请以下列列表中的账户身份启动

用户列表，密码同
compiler1