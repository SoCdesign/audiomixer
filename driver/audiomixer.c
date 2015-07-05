#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm/uaccess.h>	/* Needed for copy_from_user */
#include <asm/io.h>		/* Needed for IO Read/Write Functions */
#include <linux/proc_fs.h>	/* Needed for Proc File System Functions */
#include <linux/seq_file.h>	/* Needed for Sequence File Operations */
#include <linux/platform_device.h>	/* Needed for Platform Driver Functions */
#include <linux/spinlock.h>
#include <asm/spinlock.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/time.h>

/* Define Driver Name */
#define DRIVER_NAME "audiomixer"
#define NAMELEN 128
#define NAMEOFFSET 9
#define DEVNUM 5

#define CTLREG 27
#define INPUTREG 28 
#define FRAMEREG 30

//#define NSINTERVAL 20834
#define NSINTERVAL 41667

//#define FRAMESPERSEC 48000
#define FRAMESPERSEC 24000
#define FRAMEBUFLEN FRAMESPERSEC * 2
#define FRAMEBUFSIZE FRAMESPERSEC * sizeof(u32) * 2

//#define FRAMEBUFLEN 8000 
//#define FRAMEBUFSIZE 8000 * sizeof(u32) 

#define INPUTBUFSIZE FRAMEBUFLEN * 16 

char *channel_in_str;

unsigned int buf_sw, buf_free, buf_count;
spinlock_t buf_free_lock;

ktime_t kt;
struct hrtimer hrt;
int tt_count = 0;

struct timespec handler_start, handler_stop, handler_delta;

char * channel_in = "channel_in ";

typedef struct frame_buffer {
	u32 *buf;	
	struct frame_buffer * next;
} frame_buffer;

typedef struct {
	char name[NAMELEN];
	struct platform_device *pdev;
	struct proc_dir_entry *pde;
	struct proc_dir_entry *pde_channel_in;
	unsigned long *base_addr;	/* Vitual Base Address */
	unsigned long remap_size;	/* Device Memory Size */
	struct resource *res;		/* Device Resource Structure */
	u32 *framebuf[2];
	u32 fb_count;
	u32 frames_read;
	u32 frames_count;
	unsigned long frames_missed;
} audiomixer_device;

//typedef struct {
//	char name[NAMELEN];
//	struct platform_device *pdev;
//	struct proc_dir_entry *pde;
//	struct proc_dir_entry *pde_channel_in;
//	unsigned long *base_addr;	/* Vitual Base Address */
//	unsigned long remap_size;	/* Device Memory Size */
//	struct resource *res;		/* Device Resource Structure */
//	frame_buffer *fb;	
//	frame_buffer *curr;	
//	frame_buffer *last;
//	u32 fb_count;
//	u32 frames_read;
//	u32 frames_count;
//	unsigned long frames_missed;
//} audiomixer_device;


audiomixer_device a_devs[DEVNUM];


//frame_buffer *create_fbuffer(frame_buffer *next)
//{
//	frame_buffer *fb = kmalloc(sizeof(frame_buffer), GFP_ATOMIC);
//	
//	fb->buf = kmalloc(FRAMEBUFSIZE, GFP_ATOMIC);
//	memset(fb->buf, 0, FRAMEBUFSIZE);
//	fb->next = next;
//
//	return fb;
//}
//
//int destroy_fbuffer(frame_buffer *fb)
//{
//	frame_buffer *nfb, *nnfb;
//
//	nnfb = (frame_buffer *)1;
//	nfb = fb;
//
//	while(nnfb != NULL && nnfb != fb)
//	{	
//		kfree(nfb->buf);
//		nnfb = nfb->next;	
//		kfree(nfb);
//		nfb = nnfb;
//	}
//
//	return 0;	
//}
//
//void print_fbuffer(struct seq_file *p, frame_buffer *fb)
//{
//	frame_buffer *nfb;
//	int i;
//        nfb = fb;
//
//	do
//        {
//		for(i = 0; i < FRAMEBUFLEN; i += 2)
//			seq_printf(p, "%08x%08x\n", nfb->buf[i], nfb->buf[i + 1]);		
//                nfb = nfb->next;
//        }
//	while(nfb != NULL && nfb != fb);
//}

enum hrtimer_restart handler(struct hrtimer *timer)
{
	int i;
	//int j;
	unsigned int frames_missed;

	//u32 ch_l, ch_r, filter_done, ctlreg;
	//u32 filter_done, ctlreg;

	getnstimeofday(&handler_start);

	if (buf_count >= FRAMEBUFLEN)
	{
	       	buf_sw = buf_free;
	
	        //spin_lock(&buf_free_lock);
	        buf_free = (buf_free + 1) & 0x1;
	        //spin_unlock(&buf_free_lock);
					
		buf_count = 0;
	}

	for(i = 0; i < DEVNUM - 1; i++)
	{

		iowrite32(a_devs[i].framebuf[buf_sw][buf_count], a_devs[i].base_addr + FRAMEREG);
		iowrite32(a_devs[i].framebuf[buf_sw][buf_count + 1], a_devs[i].base_addr + FRAMEREG + 1);

				

		//ctlreg = ioread32(a_devs[i].base_addr + CTLREG);

		//do
		//{
			//filter_done = ioread32(a_devs[i].base_addr + 26);
		//}
		//while(! (filter_done & 0x4));
	

		
		
		//set sample_trig
		//iowrite32(ctlreg | 0x20, a_devs[i].base_addr + CTLREG);
		//
		////wait
		//for(j = 0; j < 32; j++);

		////unset sample trig
		//iowrite32(ctlreg & ~0x20, a_devs[i].base_addr + CTLREG);
			
		//set input amplitude to zero until (again) overwritten by actual frames 
		//a_devs[i].framebuf[buf_sw][buf_count] = 0; 
		//a_devs[i].framebuf[buf_sw][buf_count + 1] = 0;
		
		
		//write back debug
		//ch_l = ioread32(a_devs[i].base_addr + INPUTREG);		
		//ch_r = ioread32(a_devs[i].base_addr + INPUTREG + 1);		
		//wmb();
		//
		//rmb();
		//iowrite32(ch_l, a_devs[i].base_addr + FRAMEREG);
		//iowrite32(ch_r, a_devs[i].base_addr + FRAMEREG + 1);

	}


	frames_missed = hrtimer_forward_now(timer, kt);

	for(i = 0; i < DEVNUM - 1; i++)
	{
		if (buf_count >= FRAMEBUFLEN)
		a_devs[i].frames_count++;
		a_devs[i].frames_missed += frames_missed;
	}

	getnstimeofday(&handler_stop);
	handler_delta = timespec_sub(handler_stop, handler_start);

	//if((buf_count % 8000) == 0)	
	//	printk(KERN_INFO DRIVER_NAME " HANDLER device %d wrote frame %08x%08x in %lu seconds %lu nanoseconds\n", 0, a_devs[0].framebuf[buf_sw][buf_count], a_devs[0].framebuf[buf_sw][buf_count + 1], handler_delta.tv_sec, handler_delta.tv_nsec);


	buf_count += 2;
	

        return HRTIMER_RESTART;
}


void trigger_timer(void)
{
	//if (tt_count == 4)
	if (tt_count == 5)
	{
		buf_sw = 0;
        	buf_free = 1;
        	buf_count = 0;
        	spin_lock_init(&buf_free_lock);

		kt = ktime_set(0, NSINTERVAL);
        	hrtimer_init(&hrt, CLOCK_REALTIME, HRTIMER_MODE_REL_PINNED);
        	hrt.function = handler;
        	hrtimer_start(&hrt, kt, HRTIMER_MODE_REL_PINNED);
	}

	tt_count++;
}



int get_audiomixer_device_pdev(struct platform_device *pdev) {
	int i;
	for(i = 0; (a_devs[i].pdev != pdev) && i < DEVNUM; ++i);

	if (i == DEVNUM) i = -1;

	return i;
}

int get_audiomixer_device_file(struct file *file) {
	int i;
	struct proc_dir_entry *pde = PDE(file->f_path.dentry->d_inode);

	for(i = 0; (a_devs[i].pde != pde) && i < DEVNUM; ++i);
	
	if(i == DEVNUM)
		for(i = 0; (a_devs[i].pde_channel_in != pde) && i < DEVNUM; ++i);
	
	if (i == DEVNUM) i = -1;

	return i;
}



/* Write operation for /proc/audiomixer
 * -----------------------------------
 *  When user cat a string to /proc/audiomixer file, the string will be stored in
 *  const char __user *buf. This function will copy the string from user
 *  space into kernel space, and change it to an unsigned long value.
 *  It will then write the value to the register of audiomixer controller,
 *  and turn on the corresponding LEDs eventually.
 */

int filter_count = 0;

static ssize_t proc_audiomixer_write(struct file *file, const char __user * buf,
				size_t count, loff_t * ppos)
{
	char audiomixer_str_reg[5];
	char audiomixer_str_value[16];
	
	int i = get_audiomixer_device_file(file);	

	u32 audiomixer_reg;
	u32 audiomixer_value;
	//u32 ctlreg = 0;

	if (count < 14) {
		if (copy_from_user(audiomixer_str_value, buf, count))
			return -EFAULT;

		audiomixer_str_value[count] = '\0';
	}

	audiomixer_str_reg[0] = '0';
	audiomixer_str_reg[1] = 'x';
	audiomixer_str_reg[2] = audiomixer_str_value[0]; 
	audiomixer_str_reg[3] = audiomixer_str_value[1]; 
	audiomixer_str_reg[4] = '\0';
	
	audiomixer_reg = simple_strtoul(audiomixer_str_reg, NULL, 0);

	if(audiomixer_reg == 0x20)
	{
		trigger_timer();
		return count;
	}

	audiomixer_str_value[0] = '0';
	audiomixer_str_value[1] = 'x';

	//bad approach
	////if filter coefficients are updated
	//If(audiomixer_reg < 0x10)
	//{
	//	filter_count++;
	//	if(filter_count == 1)
	//	{
	//		//set filter to reset
	//		ctlreg = ioread32(a_devs[i].base_addr + CTLREG);

	//		iowrite32(ctlreg | 0x10, a_devs[i].base_addr + CTLREG);
	//		
	//	}	
	//}
		
	audiomixer_value = simple_strtoul(audiomixer_str_value, NULL, 0);

	//if filter coefficients were updated
	//if(audiomixer_reg < 0x10)
	//{
	//	if(filter_count == 5)
	//	{
	//		//release filter reset
	//		ctlreg = ioread32(a_devs[i].base_addr + CTLREG);
	//		iowrite32(ctlreg & ~0x10, a_devs[i].base_addr + CTLREG);
	//		filter_count = 0;
	//	}
	//}

	//audiomixer_value = simple_strtoul(audiomixer_phrase, NULL, 0);

	wmb();
	iowrite32(audiomixer_value, a_devs[i].base_addr + audiomixer_reg);
	printk(KERN_INFO DRIVER_NAME " audiomixer let's write to %08x\n", (int)(a_devs[i].base_addr + audiomixer_reg));
	return count;
}



/* Callback function when opening file /proc/audiomixer
 * ------------------------------------------------------
 *  Read the register value of audiomixer controller, print the value to
 *  the sequence file struct seq_file *p. In file open operation for /proc/audiomixer
 *  this callback function will be called first to fill up the seq_file,
 *  and seq_read function will print whatever in seq_file to the terminal.
 */
static int proc_audiomixer_show(struct seq_file *p, void *v)
{
	u32 audiomixer_value;
	int i;
	audiomixer_device * a_dev = (audiomixer_device *)p->private;

	printk(KERN_INFO DRIVER_NAME " Show Address: %08x\n", (unsigned int) p->private);

	seq_printf(p, "component %s\n", a_dev->name);
	
	for(i = 0; i < 32; ++i)
	{
		audiomixer_value = ioread32(a_dev->base_addr + i);
		seq_printf(p, "R 0x%08x V 0x%08x\n", (u32) (a_dev->base_addr + i), (u32) audiomixer_value);
	}
	
	return 0;
}

/* Open function for /proc/audiomixer
 * ------------------------------------
 *  When user want to read /proc/audiomixer (i.e. cat /proc/audiomixer ), the open function will
 *  be called first. In the open function, a seq_file will be prepared and the status
 *  of audiomixer will be filled into the seq_file by proc_audiomixer_show function.
 */
static int proc_audiomixer_open(struct inode *inode, struct file *file)
{
	unsigned int size = 16;
	char *buf;
	struct seq_file *m;
	int res;

	int i = get_audiomixer_device_file(file);
	printk(KERN_INFO DRIVER_NAME " Open %d\n", i);

	buf = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	printk(KERN_INFO DRIVER_NAME " Open Address: %08x\n", (unsigned int)&a_devs[i]);
	res = single_open(file, proc_audiomixer_show, (void *)&a_devs[i]);

	if (!res) {
		m = file->private_data;
		m->buf = buf;
		m->size = size;
	} else {
		kfree(buf);
	}

	return res;
}

static const struct file_operations proc_audiomixer_operations = {
	.open = proc_audiomixer_open,
	.read = seq_read,
	.write = proc_audiomixer_write,
	.llseek = seq_lseek,
	.release = single_release
};


/* Write operation for /proc/channel_in
 * -----------------------------------
 *  When user cat a string to /proc/channel_in file, the string will be stored in
 *  const char __user *buf. This function will copy the string from user
 *  space into kernel space, and change it to an unsigned long value.
 *  It will then write the value to the register of channel_in controller,
 *  and turn on the corresponding LEDs eventually.
 */

#define FRAMEREG 30


static ssize_t proc_channel_in_write(struct file *file, const char __user * buf,
				size_t count, loff_t * ppos)
{
	char str[11];
	//frame_buffer *fb;

	char inputbuffer[17];


	int j, k[5], m;
	int i = get_audiomixer_device_file(file);
	audiomixer_device * a_dev = &a_devs[i];

	printk(KERN_INFO DRIVER_NAME " \n");
	printk(KERN_INFO DRIVER_NAME " ###NEW CHANNEL_IN CALL### for device %d\n", i);


	if (count < INPUTBUFSIZE) {
		if (copy_from_user(channel_in_str, buf, count))
			return -EFAULT;

		channel_in_str[count] = '\0';

		printk(KERN_INFO DRIVER_NAME " CHANNEL_IN input data is %s\n", channel_in_str);
		printk(KERN_INFO DRIVER_NAME " CHANNEL_IN count is %d\n", count);

	}

	str[0] = '0';
	str[1] = 'x';
	
        //spin_lock(&buf_free_lock);

	//drop part of frame fragment in front of next whole frame 
	for(j = 0; channel_in_str[j] != '#' && j < 17; j++);


	for(; j < count ; j += 17)		
	{
		if(channel_in_str[j] != '#')
		{
			for(m = 0; channel_in_str[j] != '#' && m < 17; m++, j++);	

			if(m == 17)
			{
				//input broken
				goto FINISH;	
			}
		}	

		//not enough bytes left
		if((count - 17 - j) < 0)
		{
			//drop part of frame
			goto FINISH;	
		}
		else
		{
			memcpy(inputbuffer, channel_in_str + j + 1, 16);
			inputbuffer[16] = '\0';
			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN k[%d] %d before conversion input %s\n", i, k[i], inputbuffer);

			k[i] = a_dev->frames_read;
			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN intermediate k[%d] %d\n", i, k[i]);
			k[i] %= FRAMEBUFLEN;
			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN new k[%d] %d\n", i, k[i]);

			memcpy(str + 2, channel_in_str + j + 1, 8);
			str[10] = '\0';
			
			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN k[%d] %d before conversion value left %s\n", i, k[i], str);

			a_dev->framebuf[buf_free][k[i]] = simple_strtoul(str, NULL, 0);


			memcpy(str + 2, channel_in_str + j + 9, 8);
			str[10] = '\0';

			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN k[%d] %d before conversion value right %s\n", i, k[i], str);

			a_devs->framebuf[buf_free][k[i] + 1] = simple_strtoul(str, NULL, 0);
			//wmb();
			

			a_devs->frames_read += 2;


			//iowrite32(channel_in_l, a_devs[i].base_addr + FRAMEREG);
			//iowrite32(channel_in_r, a_devs[i].base_addr + FRAMEREG + 1);
			//printk(KERN_INFO DRIVER_NAME " CHANNEL_IN channel %d frame_read %d value %08x%08x\n", i, a_devs[i].frames_read, a_devs[i].framebuf[buf_free][k[1]], a_devs[i].framebuf[buf_free][k[1] + 1]);
		}

	}

FINISH:

	//spin_unlock(&buf_free_lock);

	printk(KERN_INFO DRIVER_NAME " CHANNEL_IN count %d k[%d] %d a_devs[0].frames_read %d\n", count, i, k[i], a_dev->frames_read);

	return count;
}

/* Callback function when opening file /proc/channel_in
 * ------------------------------------------------------
 *  Read the register value of channel_in controller, print the value to
 *  the sequence file struct seq_file *p. In file open operation for /proc/channel_in
 *  this callback function will be called first to fill up the seq_file,
 *  and seq_read function will print whatever in seq_file to the terminal.
 */
static int proc_channel_in_show(struct seq_file *p, void *v)
{
	//u32 channel_in_value;
	int i;
	audiomixer_device * a_dev = (audiomixer_device *)p->private;

	for(i = 0; i < 5; i++)
		seq_printf(p, "device struct %d : %08x\n", i, (unsigned int) &a_devs[i]);
	seq_printf(p, "this device: %08x\n", (unsigned int) p->private);

	seq_printf(p, "component %s\n", a_dev->name);

	//printk(KERN_INFO DRIVER_NAME " print_fbuffer: p %08x a_dev->fb %08x\n", (unsigned int) p, (unsigned int) a_dev->fb);
	//print_fbuffer(p, a_dev->fb);

		
	for(i = 0; i < FRAMEBUFLEN; i += 2)
	{
		seq_printf(p, "%08x%08x\n", a_dev->framebuf[buf_free][i], a_dev->framebuf[buf_free][i + 1]); 
	}

	for(i = 0; i < 5; ++i)
	{
		seq_printf(p, "R%02d pdev 0x%08x\n pde 0x%08x \n pde_channel_in 0x%08x \n base_addr 0x%08x\n remap_size %08x \n res %08x\n", i, (u32) a_devs[i].pdev, (u32) a_devs[i].pde, (u32) a_devs[i].pde_channel_in, (u32) a_devs[i].base_addr, (u32) a_devs[i].remap_size, (u32) a_devs[i].res);
	}
	
	return 0;
}

/* Open function for /proc/channel_in
 * ------------------------------------
 *  When user want to read /proc/channel_in (i.e. cat /proc/channel_in ), the open function will
 *  be called first. In the open function, a seq_file will be prepared and the status
 *  of channel_in will be filled into the seq_file by proc_channel_in_show function.
 */
static int proc_channel_in_open(struct inode *inode, struct file *file)
{
	unsigned int size = 16;
	char *buf;
	struct seq_file *m;
	int res;

	int i = get_audiomixer_device_file(file);
	printk(KERN_INFO DRIVER_NAME " Open %d\n", i);

	buf = (char *)kmalloc(size * sizeof(char), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	printk(KERN_INFO DRIVER_NAME " Open Address: %08x\n", (unsigned int)&a_devs[i]);
	res = single_open(file, proc_channel_in_show, (void *)&a_devs[i]);

	if (!res) {
		m = file->private_data;
		m->buf = buf;
		m->size = size;
	} else {
		kfree(buf);
	}

	return res;
}

static const struct file_operations proc_channel_operations = {
	.open = proc_channel_in_open,
	.read = seq_read,
	.write = proc_channel_in_write,
	.llseek = seq_lseek,
	.release = single_release
};

/* Shutdown function for audiomixer
 * -----------------------------------
 *  Before audiomixer shutdown, turn-off all the leds
 */
static void audiomixer_shutdown(struct platform_device *pdev)
{
	unsigned int j = get_audiomixer_device_pdev(pdev);
	unsigned int i;
	hrtimer_cancel(&hrt);
	for(i = 0; i < 32; i++)
		iowrite32(0, a_devs[j].base_addr + i);

	for(i = 0; i < 5; i++)
	{
		kfree(a_devs[i].framebuf[0]);
		kfree(a_devs[i].framebuf[1]);
	}
	

	kfree(channel_in_str);
}

/* Remove function for audiomixer
 * ----------------------------------
 *  When audiomixer module is removed, turn off all the leds first,
 *  release virtual address and the memory region requested.
 */
static int audiomixer_remove(struct platform_device *pdev)
{
	unsigned int j = get_audiomixer_device_pdev(pdev);
	audiomixer_shutdown(pdev);

	/* Remove /proc/audiomixer entry */
	remove_proc_entry(DRIVER_NAME, NULL);

	/* Release mapped virtual address */
	iounmap(a_devs[j].base_addr);

	/* Release the region */
	release_mem_region(a_devs[j].res->start, a_devs[j].remap_size);

	return 0;
}

/* Device Probe function for audiomixer
 * ------------------------------------
 *  Get the resource structure from the information in device tree.
 *  request the memory regioon needed for the controller, and map it into
 *  kernel virtual memory space. Create an entry under /proc file system
 *  and register file operations for that entry.
 */
static int __devinit audiomixer_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i, j;
	char c;

	char *str = (char*)pdev->name + NAMEOFFSET;
	
	
	printk(KERN_INFO DRIVER_NAME " PROBE device is %s\n", str);

	if (strncmp(str, "channel", 7) == 0) {
		printk(KERN_INFO DRIVER_NAME " channel device is %s\n", str);
		c = *(str + 7);
		i = ((int) c) - 48;
		i = ((0 <= i) && (i < DEVNUM)) ? i : DEVNUM - 1;
	} else {
		printk(KERN_INFO DRIVER_NAME " audiomixer device is %s\n", str);
		i = DEVNUM - 1;
		c = (char)(i + 48);
	}
	
	strncpy(a_devs[i].name, pdev->name + NAMEOFFSET, NAMELEN);
	a_devs[i].pdev = pdev;

	j = strnlen(channel_in, NAMELEN);
	if(j < NAMELEN)
	{	
		channel_in[j-1] = c;
	} else {
		channel_in[j-1] = 'x';
	}
	

	printk(KERN_INFO DRIVER_NAME " name %s with id %d and number %c (int) %d - 48 = %d a_devs[%d].name %s\n", pdev->name, pdev->id, c, (int) c, i, i, a_devs[i].name);

	printk(KERN_INFO DRIVER_NAME " a_devs[%d] %08x\n", i, (unsigned int) &a_devs[i]);

	//remapping IO

	a_devs[i].res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!a_devs[i].res) {
		dev_err(&pdev->dev, "No memory resource\n");
		return -ENODEV;
	}


	printk(KERN_INFO DRIVER_NAME " %d platform_get_resource %08x\n", i, (unsigned int) a_devs[i].res);

	a_devs[i].remap_size = a_devs[i].res->end - a_devs[i].res->start + 1;
	if (!request_mem_region(a_devs[i].res->start, a_devs[i].remap_size, pdev->name)) {
		dev_err(&pdev->dev, "Cannot request IO\n");
		return -ENXIO;
	}

	printk(KERN_INFO DRIVER_NAME " request_mem_region\n");

	a_devs[i].base_addr = ioremap(a_devs[i].res->start, a_devs[i].remap_size);
	if (a_devs[i].base_addr == NULL) {
		dev_err(&pdev->dev, "Couldn't ioremap memory at 0x%08lx\n",
			(unsigned long)a_devs[i].res->start);
		ret = -ENOMEM;
		goto err_release_region;
	}

	printk(KERN_INFO DRIVER_NAME " %d ioremap %08x\n", i, (unsigned int) a_devs[i].res);

	//creating proc entries	
	a_devs[i].pde = proc_create(a_devs[i].name, 0, NULL, &proc_audiomixer_operations);
	if (a_devs[i].pde == NULL)
	{
		dev_err(&pdev->dev, "Couldn't create proc entry\n");
		ret = -ENOMEM;
		goto err_create_proc_entry;
	}

	//if device is channel, create channel_in as frame input device
	if(i < DEVNUM - 1)
	{
		a_devs[i].pde_channel_in = proc_create(channel_in, 0, NULL, &proc_channel_operations);
		if (a_devs[i].pde_channel_in == NULL)
		{
			dev_err(&pdev->dev, "Couldn't create proc entry\n");
			ret = -ENOMEM;
			goto err_create_proc_entry;
		}
	} else {
		a_devs[i].pde_channel_in = NULL;
	}

	if(! (i == DEVNUM -1))
	{
		a_devs[i].framebuf[0] = kmalloc(FRAMEBUFSIZE, GFP_ATOMIC);
	        a_devs[i].framebuf[1] = kmalloc(FRAMEBUFSIZE, GFP_ATOMIC);
		memset(a_devs[i].framebuf[0], 0, FRAMEBUFSIZE);
		memset(a_devs[i].framebuf[1], 0, FRAMEBUFSIZE);
	
		//a_devs[i].fb = create_fbuffer(NULL);
		//a_devs[i].fb->next = a_devs[i].fb;
		//a_devs[i].curr = a_devs[i].fb;
		//a_devs[i].last = a_devs[i].fb;
		
	        a_devs[i].fb_count = 0;
	        a_devs[i].frames_read = 0;
	        a_devs[i].frames_count = 0;
	        a_devs[i].frames_missed = 0;
	}

	buf_count = 0;

	channel_in_str = kmalloc(INPUTBUFSIZE, GFP_ATOMIC);

	trigger_timer();

	printk(KERN_INFO DRIVER_NAME " probed at va 0x%08lx\n",
	       (unsigned long)a_devs[i].base_addr);

	return 0;

 err_create_proc_entry:
	iounmap(a_devs[i].base_addr);
 err_release_region:
	release_mem_region(a_devs[i].res->start, a_devs[i].remap_size);

	return ret;
}

/* device match table to match with device node in device tree */
static const struct of_device_id audiomixer_of_match[] __devinitconst = {
	{.compatible = "dglnt,channel-1.00.a"},
	{.compatible = "dglnt,globalmixer-1.00.a"},
	{},
};

MODULE_DEVICE_TABLE(of, audiomixer_of_match);

/* platform driver structure for audiomixer driver */
static struct platform_driver audiomixer_driver = {
	.driver = {
		   .name = DRIVER_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = audiomixer_of_match},
	.probe = audiomixer_probe,
	.remove = __devexit_p(audiomixer_remove),
	.shutdown = __devexit_p(audiomixer_shutdown)
};

/* Register audiomixer platform driver */
module_platform_driver(audiomixer_driver);

/* Module Infomations */
MODULE_AUTHOR("Digilent, Inc.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_NAME ": AUDIOMIXER driver (Simple Version)");
MODULE_ALIAS(DRIVER_NAME);
