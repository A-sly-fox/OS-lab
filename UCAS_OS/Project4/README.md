Project4实验说明
===========================

环境依赖
---
- Linux OS
- CPU support RISCV (optional)
- `QEMU`(necessary)

当前版本
---
完成Project4 A-core相关任务

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
* 也可以使用kill指令杀死进程
* 总体流程和P3是相同的，就是多了一个虚存管理

备注
---
<br>

作者
---
- 胡力杭，2019k8009926002, UCAS
