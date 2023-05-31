Project2——A simple kernel实验说明
===========================

环境依赖
---
- Linux OS
- CPU support RISCV (optional)
- `QEMU`(necessary)

当前版本
---
完成Project2 前四个任务，完成了优先级队列

使用步骤
---
* 执⾏make命令编译<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ make  
```

* 执⾏ ``run_qemu.sh ``，显⽰出可以输⼊的命令⾏后，输⼊``loadboot``。等到“open which kernel? 0 or else”出现时通过键盘选择要开启的kernel。若输入为0，则开启第一个kernel；若输入是其他字符，则开启第二个kernel。本次实验只有一个系统，所以只能选择0.<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project2-SimpleKernel$ ./run_qemu.sh   
```

* 若出现六行指令和一个不断交替的小飞机，那么说明系统已经完成。其中第一行和第二行是sleep测试程序，第三行和第四行轮流使用lock，第五行和第六行（pcb[0]和pcb[1]）看起来后面的数字是一样的（实际上并不总是一样的，上板之后差距可能比QEMU更大）。

* 执⾏ make floppy ，写⼊磁盘

* 连接上板子后，输入``sudo sh minicom``，按板卡上的srst键重置板卡，应该就可以看到Nutshell的logo了。之后的步骤与QEMU相同。

备注
---
优先级队列需要修改init_pcb和do_scheduler函数部分代码，已经用/* 和 */标注

作者
---
- 胡力杭，2019k8009926002, UCAS
