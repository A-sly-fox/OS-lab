Project6实验说明
===========================

环境依赖
---
- Linux OS
- CPU support RISCV (optional)
- `QEMU`(necessary)

当前版本
---
完成Project6 A-core相关任务

使用步骤
---
* 执⾏make命令编译<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ make  
```

* 执⾏ ``run_qemu.sh ``，显⽰出可以输⼊的命令⾏后，输⼊``loadboot``，开启单核系统。<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ ./run_qemu.sh   
```

* 此时应该有shell的页面，使用exec命令来打开不同的测试程序
* 可以使用一些简单的命令检验文件系统的完整性

备注
---
本次实验在qemu上用了一个更大的映射disk，但是因为比较大所以没有上传gitlab。有需要qemu时只需要使用make disk命令即可，想删去disk使用make remove命令

作者
---
- 胡力杭，2019k8009926002, UCAS
