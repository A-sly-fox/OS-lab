Project3实验说明
===========================

环境依赖
---
- Linux OS
- CPU support RISCV (optional)
- `QEMU`(necessary)

当前版本
---
完成Project3 A-core相关任务 以及C-core的TASK 5

使用步骤
---
* 执⾏make命令编译<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ make  
```

* 执⾏ ``run_qemu.sh ``，显⽰出可以输⼊的命令⾏后，输⼊``loadboot``，开启单核系统。输入loadbootm，开启双核系统。<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ ./run_qemu.sh   
```

* 此时应该有shell的页面，使用exec命令来打开不同的测试程序
* 也可以使用kill指令杀死进程

备注
---
P2中有对优先级的设计，但是P3新加的进程并没有优先级的设计，所以主体框架实际上是P2 A-core的框架。bootblock也使用了单系统

作者
---
- 胡力杭，2019k8009926002, UCAS
