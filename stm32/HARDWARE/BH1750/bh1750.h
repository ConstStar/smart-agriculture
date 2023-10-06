#ifndef __BH1750_H
#define __BH1750_H

//开启光照传感器
void bh1750_power_on(void);
	
// 读取光照强度数据
double bh1750_read_data(void);
	
// 初始化光照传感器
void bh1750_init(void);

#endif
