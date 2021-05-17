
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h> 
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>

#define MYFS_MAGIC_NUMBER	0x13131313
#define SLABNAME			"my_cache"

struct kmem_cache *my_cache = NULL;
static void* *line = NULL;

struct myfs_inode
{
     int i_mode;
     unsigned long i_ino;
};

static int sco=0;
static int number=31;
static int size=sizeof(struct myfs_inode);
static int cache_pos=0;

void co(void* p) 
{ 
	*(int*)p = (int)p; 
	sco++; 
} 

static void myfs_put_super(struct super_block * sb)
{
	printk(KERN_INFO "MYFS super block destroyed!\n" );
}

static struct super_operations const myfs_super_ops = {
	.put_super = myfs_put_super,
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
};

void* get_slab_mem(void)
{
	if (cache_pos >= number)
		return NULL;
	else
		return line[cache_pos++];
}

static struct inode* myfs_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);
	struct myfs_inode *_inode = get_slab_mem();
	if (_inode)
	{
		_inode->i_mode = ret->i_mode;
		_inode->i_ino = ret->i_ino;
	}

	if (ret)
	{
		inode_init_owner(ret, NULL, mode);
		ret->i_size = PAGE_SIZE;
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
		ret->i_private = _inode;
	}

	printk(KERN_INFO "new inode created\n");
	return ret;
}


static int myfs_fill_sb(struct super_block* sb, void* data, int silent)
{
	struct inode* root = NULL;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = MYFS_MAGIC_NUMBER;
	sb->s_op = &myfs_super_ops;

	root =  myfs_make_inode(sb, S_IFDIR | 0755);
	if (!root)
	{
		printk (KERN_ERR "MYFS inode allocation failed !\n") ; 
		return -ENOMEM;
	}

	root->i_op  =  &simple_dir_inode_operations;
	root->i_fop = &simple_dir_operations;
	sb->s_root  = d_make_root(root) ;

	if (!sb->s_root)
	{
		printk(KERN_ERR "MYFS root creation failed !\n") ; 
		iput(root);
		return -ENOMEM;
	}

	return 0;
}

static struct dentry* myfs_mount (struct file_system_type *type, int flags, 
									char const *dev, void *data)
{
	struct dentry* const entry = mount_bdev(type,  flags,  dev,  data, myfs_fill_sb);

	if (IS_ERR(entry))
		printk(KERN_ERR  "MYFS mounting failed !\n") ;
	else
		printk(KERN_INFO "MYFS mounted!\n") ;
	return entry;
}

static struct file_system_type myfs_type  =  {
	.owner  =  THIS_MODULE,
	.name  =  "myfs",
	.mount  =  myfs_mount,
	.kill_sb  =  kill_block_super,
};

static int __init md_init(void) 
{
	int i, j;
	int ret;

	line = kmalloc(sizeof(void*) * number, GFP_KERNEL); 
	if (!line) 
	{ 
		printk(KERN_ERR "kmalloc error\n" ); 
		return -ENOMEM;
	}

	my_cache = kmem_cache_create(SLABNAME, size, 0, 0, co);
	if (!my_cache)
	{
		printk(KERN_ERR "Cache create failed\n");
		kfree(line);
		return -ENOMEM;
	}
	
	for(i=0; i < number; i++) 
	{
		line[i] = kmem_cache_alloc(my_cache, GFP_KERNEL);
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

	ret = register_filesystem(&myfs_type);
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
	kmem_cache_destroy(my_cache);
	kfree(line);

	ret = unregister_filesystem(&myfs_type);
	if (ret)
		printk(KERN_ERR "Filesystem unregister failed\n");
	
	printk(KERN_INFO "VFS_MD: module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
