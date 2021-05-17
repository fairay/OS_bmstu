
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h> 
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>

// Магическое число
// По нему драйвер ФС проверяет, что на диске именно та самая ФС, а не что-то другое
// Можно сказать, что это некий секретный ключ
#define MYFS_MAGIC_NUMBER	0x13131313

// Определения и переменные для кэша
#define SLABNAME			"my_cache"	// имя
struct kmem_cache *my_cache = NULL;		// сам slab-cache
static int sco=0;						// счётчик вызова конструктора

// Собственный inode
struct myfs_inode
{
     int i_mode;
     unsigned long i_ino;
} myfs_inode;

// Переменные для кэшируемых объектов
static void* *line = NULL;				// массив 
static int number = 31;					// количество
static int size = sizeof(struct myfs_inode); // размер в байтах
static int cache_pos=0;					// позиция незанятого элемента кэша

// Конструктор элемента кэша
void co(void* p) 
{ 
	*(int*)p = (int)p; 	// хз зачем, если убрать ничего не изменится
	sco++; 				// +1 счётчик вызова конструктора
} 


// Функция, вызываемая в конце umount
static void myfs_put_super(struct super_block * sb)
{
	printk(KERN_INFO "MYFS super block destroyed!\n" );
}

static struct super_operations const myfs_super_ops = {
	.put_super = myfs_put_super, // вызывается на последней стадии работы umount, освобождает всё содержимое
	.statfs = simple_statfs,	// реализация системного вызова fstatfs/statfs
								// simple_statfs - заглушка из стандартной библиотеки 
	.drop_inode = generic_delete_inode, //
};

// Получение незанятого элемента кэша
void* get_slab_mem(void)
{
	if (cache_pos >= number) 
		return NULL; // Если всё занято - вернуть NULL
	else
		return line[cache_pos++]; // Вернуть свободный, сдвинуть позицию незанятого
}

// Инициализация нового inode 
static struct inode* myfs_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);		// Размещение (создание) новго inode для заданного суперблока
	struct myfs_inode *_inode = get_slab_mem();	// Получение свободного элемента кэша для собственной структуры inode

	if (ret)
	{
		inode_init_owner(ret, NULL, mode);	// Инициирование значений uid,gid,mode 
		if (_inode)
		{
			_inode->i_mode = ret->i_mode;	// Дублирование информации
			_inode->i_ino = ret->i_ino;
		}

		ret->i_size = PAGE_SIZE;			// размер файла равен 4Кб
		// время последнего изменения, доступа, изменения индекса = текущее время
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);	
		ret->i_private = _inode;	// Поле приватной информации
	}

	printk(KERN_INFO "new inode created\n");
	return ret;
}


// Инициализация суперблока
static int myfs_fill_sb(struct super_block* sb, void* data, int silent)
{	
	struct inode* root = NULL;			// inode корневого каталога
	sb->s_blocksize = PAGE_SIZE;		// Размер блока 
	sb->s_blocksize_bits = PAGE_SHIFT;  // Количество бит, необходимое для хранения размера блока
	sb->s_magic = MYFS_MAGIC_NUMBER;	// Магическое число (см выше)
	sb->s_op = &myfs_super_ops;			// Набор операций над суперблоком

	root =  myfs_make_inode(sb, S_IFDIR | 0755);	// Инициализация inode, S_IFDIR <=> каталог
	if (!root)
	{
		printk (KERN_ERR "MYFS inode allocation failed !\n") ; 
		return -ENOMEM;
	}

	root->i_op  =  &simple_dir_inode_operations;	// Операции с inode
	root->i_fop = &simple_dir_operations;			// Операции с файлом
	sb->s_root  = d_make_root(root);				// Cоздание dentry по корневому inode
	

	if (!sb->s_root)
	{
		printk(KERN_ERR "MYFS root creation failed !\n") ; 
		iput(root);	// Сброс inode
		return -ENOMEM;
	}

	return 0;
}

// Функция монтирования ФС, вызывается по команде mount
static struct dentry* myfs_mount (struct file_system_type *type, int flags, 
									char const *dev, void *data)
{
	// см readme
	struct dentry* const entry = mount_bdev(type,  flags,  dev,  data, myfs_fill_sb);

	if (IS_ERR(entry))
		printk(KERN_ERR  "MYFS mounting failed !\n") ;
	else
		printk(KERN_INFO "MYFS mounted!\n") ;
	return entry;
}

static struct file_system_type myfs_type  =  {
	.owner  =  THIS_MODULE,		// указатель на модуль реализации ФС
	.name  =  "myfs",			// имя ФС
	.mount  =  myfs_mount,		// функция монтирования ФС
	.kill_sb  =  kill_block_super, // функция демонтирования ФС
};

static int __init md_init(void) 
{
	int i, j;
	int ret;

	line = kmalloc(sizeof(void*) * number, GFP_KERNEL); // выделение памяти под массив элементов кэша
	if (!line) 
	{ 
		printk(KERN_ERR "kmalloc error\n" ); 
		return -ENOMEM;
	}

	my_cache = kmem_cache_create(SLABNAME, size, 0, 0, co);	// создание slab-кэша 
	if (!my_cache)
	{
		printk(KERN_ERR "Cache create failed\n");
		kfree(line);
		return -ENOMEM;
	}
	
	for(i=0; i < number; i++) 
	{
		line[i] = kmem_cache_alloc(my_cache, GFP_KERNEL);	// создание элемента кэша
		if(!line[i])
		{ 
			printk(KERN_ERR "kmem_cache_alloc error\n" );
			for (j=0; j < i; j++)
				kmem_cache_free(my_cache, line[j]);
			kmem_cache_destroy(my_cache);
			kfree(line);
			return -ENOMEM;
		}
	}

	ret = register_filesystem(&myfs_type);		// регистрация ФС
	if (ret < 0)
	{
		printk(KERN_ERR "Filesystem register failed\n");
		for (j=0; j < number; j++)
			kmem_cache_free(my_cache, line[j]);
		kmem_cache_destroy(my_cache);
		kfree(line);
		return ret;
	}

	printk(KERN_INFO "VFS_MD: allocate %d objects into slab: %s\n", number, SLABNAME); 
	printk(KERN_INFO "VFS_MD: object size %d bytes, full size %ld bytes\n", size, (long)size * number); 
	printk(KERN_INFO "VFS_MD: constructor called %d times\n", sco); 
	printk(KERN_INFO "VFS_MD: module is loaded\n");
 	return 0;
}


static void __exit md_exit(void) 
{
	int j, ret;
	for (j=0; j < number; j++)
		kmem_cache_free(my_cache, line[j]);
	if (kmem_cache_destroy(my_cache) < 0)
		printk(KERN_ERR "Cache destroy failed\n");
	kfree(line);

	ret = unregister_filesystem(&myfs_type);		// Дерегистрация 
	if (ret)
		printk(KERN_ERR "Filesystem unregister failed\n");
	
	printk(KERN_INFO "VFS_MD: module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
