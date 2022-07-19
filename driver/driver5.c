#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/usb.h>
//#include <err.h>
#define MEM_SIZE 1024

 

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))

#define BULK_EP_OUT 0x02
/*01 82*/
#define BULK_EP_IN 0x81

#define MAX_PKT_SIZE 512

uint8_t *kernel_buffer;
dev_t dev_num;
struct class *device_class;
struct cdev *char_device;
struct crypto_cipher *tfm;
char key[20] = "0123456789abcdef";
char type[100];
char data[MEM_SIZE];
size_t data_len = 0;

static struct usb_device *device;

static struct usb_class_driver class;

static unsigned char bulk_buf[MAX_PKT_SIZE];

 int hextostring(char *in, int len, char *out)
{
    int i;

    for (i = 0; i < len; i++)
    {
        sprintf(out, "%s%02hhx", out, in[i]);
    }
    return 0;
}

int stringtohex(char *in, int len, char *out)
{
    int i;
    int converter[105];
    converter['0'] = 0;
    converter['1'] = 1;
    converter['2'] = 2;
    converter['3'] = 3;
    converter['4'] = 4;
    converter['5'] = 5;
    converter['6'] = 6;
    converter['7'] = 7;
    converter['8'] = 8;
    converter['9'] = 9;
    converter['a'] = 10;
    converter['b'] = 11;
    converter['c'] = 12;
    converter['d'] = 13;
    converter['e'] = 14;
    converter['f'] = 15;

    for (i = 0; i < len; i = i + 2)
    {
        char byte = converter[(int)in[i]] << 4 | converter[(int)in[i + 1]];
        out[i / 2] = byte;
    }

    return 0;
}

static int pen_open(struct inode *i, struct file *f)

{

    return 0;

}

static int pen_close(struct inode *i, struct file *f)

{

    return 0;

}

static ssize_t pen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)

{

    int retval;

    int read_cnt;
    
    char cipher[1000];
    char hex_cipher[1000];
    int i, j;

    printk("data_len: %ld\n", data_len);

    memset(cipher, 0, sizeof(cipher));
    memset(hex_cipher, 0, sizeof(hex_cipher));

    for (i = 0; i < data_len / 16; i++)
    {
        char one_data[20], one_cipher[20];

        memset(one_data, 0, sizeof(one_data));
        memset(one_cipher, 0, sizeof(one_cipher));

        for (j = 0; j < 16; j++)
            one_data[j] = data[i * 16 + j];

        printk("one data: %s\n", one_data);

        if (strcmp(type, "encrypt") == 0)
            crypto_cipher_encrypt_one(tfm, one_cipher, one_data);
        if (strcmp(type, "decrypt") == 0)
            crypto_cipher_decrypt_one(tfm, one_cipher, one_data);
        for (j = 0; j < 16; j++)
            cipher[i * 16 + j] = one_cipher[j];

        printk("one cipher: %s\n", one_cipher);
    }

    hextostring(cipher, data_len, hex_cipher);
    printk("hex cipher: %s\n", hex_cipher);
    copy_to_user(buf, hex_cipher, data_len * 2);
 



    /*if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))

    {
        return -EFAULT;

    }
    return MIN(cnt, read_cnt);*/

 
    /* Read the data from the bulk endpoint 

   retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN),

            data, MAX_PKT_SIZE, &read_cnt, 5000);
    if (retval)

    {

        printk(KERN_ERR "Bulk message returned %d\n", retval);

        return retval;

    }

    */
    return 0;
    

}

static ssize_t pen_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)

{
    pr_info("USB pen: Received");
    int retval;

    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);

    char buffer[1000], hex_data[1000];
    int i, j;

    memset(buffer, 0, sizeof(buffer));
    memset(data, 0, sizeof(data));
    memset(type, 0, sizeof(type));
    memset(hex_data, 0, sizeof(hex_data));

    if (copy_from_user(buffer, buf, MIN(cnt, MAX_PKT_SIZE)))

    {

        return -EFAULT;

    }

    pr_info("USB device: Buffer content");
    for (i = 0; i < 16; i++) {
        pr_info("%x", buffer[i]);
    }
    i = 0;
    j = 0;
    while (buffer[i] != '\n' && j < cnt)
    {
        type[i] = buffer[j];
        i++;
        j++;
    }

    i = 0;
    j++;
    while (j < cnt)
    {
        hex_data[i] = buffer[j];
        i++;
        j++;
    }
    printk("type: %s\n", type);
    printk("hex_data: %s\n", hex_data);

    memset(buffer, 0, sizeof(buffer));
    stringtohex(hex_data, strlen(hex_data), data);
    printk("data: %s\n", data);
    if (strlen(hex_data) % 32 == 0)
        data_len = ((uint16_t)(strlen(hex_data) / 32)) * 16;
    else
        data_len = ((uint16_t)((strlen(hex_data) / 32) + 1)) * 16;
    
    /* Write the data into the bulk endpoint 

   retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT),
            data, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000); 
            
    if (retval)

    {

        printk(KERN_ERR "Bulk message returned %d\n", retval);

        return retval;

    }

 
    */
    return 0;

}

 

static struct file_operations fops =

{

    .open = pen_open,

    .release = pen_close,

    .read = pen_read,

    .write = pen_write,

};

 

static int pen_probe(struct usb_interface *interface, const struct usb_device_id *id)

{

    pr_info("USB Driver: Pen probing");
    int retval;

 

    device = interface_to_usbdev(interface);

 

    class.name = "usb/manh%d";

    class.fops = &fops;

    if ((retval = usb_register_dev(interface, &class)) < 0)

    {

        /* Something prevented us from registering this driver */

        //err("Not able to get a minor for this device.");
        printk(KERN_INFO "Not able to get a minor for this device.");

    }

    else

    {

        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);

}

 

    return retval;

}

 

static void pen_disconnect(struct usb_interface *interface)

{

    pr_info("Pen disconnected");
    usb_deregister_dev(interface, &class);

}

 

/* Table of devices that work with this driver */

static struct usb_device_id pen_table[] =

{

    {USB_DEVICE(0x04e8, 0x6860)},
    {} /* Terminating entry */

};

MODULE_DEVICE_TABLE (usb, pen_table);

 

static struct usb_driver pen_driver =

{

    .name = "pen_driver",

    .probe = pen_probe,

    .disconnect = pen_disconnect,

    .id_table = pen_table,

};

 

static int __init pen_init(void)

{

    int result;

    tfm = crypto_alloc_cipher("aes", 0, 0);
    crypto_cipher_setkey(tfm, key, 16);
    /* Register this driver with the USB subsystem */


    if ((result = usb_register(&pen_driver)))

    {

        //err("usb_register failed. Error number %d", result);
        printk(KERN_INFO "usb_register failed. Error number");

    }

    return result;

}

 

static void __exit pen_exit(void)

{


    /* Deregister this driver with the USB subsystem */


    usb_deregister(&pen_driver);

}

module_init(pen_init);

module_exit(pen_exit);

 

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Anil Kumar Pugalia");

MODULE_DESCRIPTION("USB Pen Device Driver");
