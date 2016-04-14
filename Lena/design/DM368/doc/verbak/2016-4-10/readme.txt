this version is for the second LENA board,for USA display
1.UBL is different for ground and air
2.uboot is only for second LENA, because of load fpga, gpio pin is changed
3.uImage is different for ground and air.
4.if you power up ground and air, and they get sync ,then you restart air, but do not power off the ground,
the ground can not get sync. you must restart air .