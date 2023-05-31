Project1——Bootloader实验说明
===========================

环境依赖
---
- Linux OS
- CPU support RISCV (optional)
- `QEMU`(necessary)

当前版本
---
可实现C-core双系统的引导

使用步骤
---
* 执⾏make命令编译<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project1-Bootloader$ make  
```

* 执⾏ ``run_qemu.sh ``，显⽰出可以输⼊的命令⾏后，输⼊``loadboot``。等到“open which kernel? 0 or else”出现时通过键盘选择要开启的kernel。若输入为0，则开启第一个kernel；若输入是其他字符，则开启第二个kernel。<br>
```Java
stu@stu:~/oslab/UCAS_OS/Project1-Bootloader$ ./run_qemu.sh   
```

* 若出现“Hello OS！This is kernel1”或者“Hello OS！This is kernel2”并且可以输入字符，说明kernel成功打开

* 执⾏ make floppy ，写⼊磁盘

* 连接上板子后，输入``sudo sh minicom``，按板卡上的srst键重置板卡，应该就可以看到Nutshell的logo了。之后的步骤与QEMU相同。


作者
---
- 胡力杭，2019k8009926002
