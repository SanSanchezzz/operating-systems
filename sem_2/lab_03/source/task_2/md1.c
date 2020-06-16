#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Kupry");

char* md1_str_data = "MD1: Hello, world!";
int md1_int_data = 42;

extern char* md1_get_str(int n)
{
	printk( "+ md1: md1_get_str() called!\n" );
	switch (n)
	{
	case 1:
		return "Message 1!\n";
		break;
	case 2:
		return "Message 2!\n";
		break;
	default:
		return "Other message!\n";
		break;
	}
}

extern int md1_factorial(int n)
{
	int i, res;
	res = 1;

	printk( "+ md1: md1_factorial() called!\n" );
    if (n <= 0)
    {
        return 0;
    }
	for (i = 2; i <= n; i++)
    {
        res *= i;
    }

	return res;
}

// экспортируем данные
EXPORT_SYMBOL(md1_str_data);
EXPORT_SYMBOL(md1_int_data);
// экспортируем функции
EXPORT_SYMBOL(md1_get_str);
EXPORT_SYMBOL(md1_factorial);


static int __init md_init( void )
{
   printk( "+ md1: module md1 start!\n" );
   return 0;
}

static void __exit md_exit( void )
{
   printk( "+ md1: module md1 unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );
