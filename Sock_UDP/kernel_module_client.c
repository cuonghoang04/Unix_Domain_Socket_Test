
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/un.h>
#include <net/sock.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>

#include "socket_path.h"

#define DRIVER_AUTHOR ""
#define DRIVER_DESC ""

static char *sendData = "";
module_param(sendData, charp, 0000);
MODULE_PARM_DESC(sendData, "Data Send");

static int sendDataLen = 0;
module_param(sendDataLen, int, 0);
MODULE_PARM_DESC(sendDataLen, "Send data len");

static int opSend = 0;
module_param(opSend, int, 0);
MODULE_PARM_DESC(opSend, "Send data (1) or not send (0)");

static int opRecv = 0;
module_param(opRecv, int, 0);
MODULE_PARM_DESC(opRecv, "Wait receive data (1) or not receive (0)");

static int opSendFirst = 0;
module_param(opSendFirst, int, 0);
MODULE_PARM_DESC(opSendFirst, "Send data first (1) or receive first (0)");


#define MAX 100

struct socket *sock = NULL;
struct sockaddr_un *addr;

struct msghdr *msg;
struct iovec *iov;
char *data;
mm_segment_t oldfs;

int timeout;

static int create_client_socket(void)
{
    int result = -1;
    // create
    result = sock_create(AF_UNIX, SOCK_DGRAM, 0, &sock);
    if (result < 0)
    {
        printk(KERN_INFO "Error create: %d\n", result);
        return result;
    }

    addr = kmalloc(sizeof(struct sockaddr_un), GFP_KERNEL);
    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, CLIENT_SOCK_FILE);
    // bind
    result = sock->ops->bind(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_un));
    if (result < 0)
    {
        printk(KERN_INFO "Error bind: %d\n", result);
        kfree(addr);
        return result;
    }
    kfree(addr);
    return result;
}

static int connect_server_socket(void)
{
    int result = -1;
    addr = kmalloc(sizeof(struct sockaddr_un), GFP_KERNEL);
    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, SERVER_SOCK_FILE);
    while (result < 0)
    {
        result = sock->ops->connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_un), 0);
        mdelay(100);
        timeout++;
        if (timeout > 50)
        {
            printk(KERN_INFO "Connect server error: %d\n", result);
            kfree(addr);
            return result;
        }
    }
    kfree(addr);

    return result;
}

static int send_message_to_server(char *dataSend, int len)
{
    int result = -1;
    int timeout = 0;
    // Connect client socket
    if (connect_server_socket() < 0)
        return -1;

    // Create msg_send
    msg = kmalloc(sizeof(struct msghdr), GFP_KERNEL);
    iov = kmalloc(sizeof(struct iovec), GFP_KERNEL);

    memset(msg, 0, sizeof(struct msghdr));
    memset(iov, 0, sizeof(struct iovec));

    msg->msg_name = 0;
    msg->msg_namelen = 0;
    msg->msg_control = NULL;
    msg->msg_controllen = 0;
    msg->msg_flags = 0;
    // #if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0))
    //     iov->iov_base = dataSend;
    //     iov->iov_len = len;
    //     msg.msg_iov = &iov;
    //     msg.msg_iovlen = len;
    // #else
    //     iov->iov_base = dataSend;
    //     iov->iov_len = len;
    //     iov_iter_init(&msg->msg_iter, READ, iov, 1, len);
    // #endif

    iov->iov_base = dataSend;
    iov->iov_len = len;
    iov_iter_init(&msg->msg_iter, READ, iov, 1, len);

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    printk(KERN_INFO "Send message: %s, length %d .......\n", iov->iov_base, iov->iov_len);
    result = sock_sendmsg(sock, msg);
    set_fs(oldfs);
    if (result < 0)
    {
        printk(KERN_INFO "Send message error: %d\n", result);
    }

    kfree(msg);
    kfree(iov);
    kfree(dataSend);

    return result;
}

static int receive_message_from_server(char *dataReceive)
{
    int result;
    // Create msg_receive
    msg = kmalloc(sizeof(struct msghdr), GFP_KERNEL);
    iov = kmalloc(sizeof(struct iovec), GFP_KERNEL);

    memset(msg, 0, sizeof(struct msghdr));
    memset(iov, 0, sizeof(struct iovec));

    msg->msg_name = 0;
    msg->msg_namelen = 0;
    msg->msg_control = NULL;
    msg->msg_controllen = 0;
    msg->msg_flags = 0;
    // #if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0))
    //     iov->iov_base = dataReceive;
    //     iov->iov_len = MAX;
    //     msg.msg_iov = &iov;
    //     msg.msg_iovlen = MAX;
    // #else
    //     iov->iov_base = dataReceive;
    //     iov->iov_len = MAX;
    //     iov_iter_init(&msg->msg_iter, READ, iov, 1, MAX);
    // #endif

    iov->iov_base = dataReceive;
    iov->iov_len = MAX;
    iov_iter_init(&msg->msg_iter, READ, iov, 1, MAX);

    // Receive message
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    printk(KERN_INFO "Receive message .......\n");
    result = sock->ops->recvmsg(sock, (struct msghdr *)msg, sizeof(struct msghdr), 0);
    set_fs(oldfs);
    if (result < 0)
    {
        printk(KERN_INFO "Receive message error: %d\n", result);
        return result;
    }
    *iov = iov_iter_iovec(&msg->msg_iter);
    printk(KERN_INFO "Receive message: %s, length: %d\n", dataReceive, result);

    kfree(msg);
    kfree(iov);

    return result;
}

static int __init init_socket(void)
{
    // Malloc
    char *dataReceive;
    char *dataSend;

    printk(KERN_INFO "sendData: %s, length: %d\n", sendData, sendDataLen);
    printk(KERN_INFO "opSendFirst: %d\n", opSendFirst);
    printk(KERN_INFO "opSendr: %d\n", opSend);
    printk(KERN_INFO "opRecv: %d\n", opRecv);

    dataReceive = kmalloc(MAX, GFP_KERNEL);
    dataSend = kmalloc(sendDataLen, GFP_KERNEL);

    memcpy(dataSend, sendData, sendDataLen);

    if (create_client_socket() < 0)
        return 0;

    if (opSendFirst == 1)
    {
        if (opSend == 1)
        {
            send_message_to_server(dataSend, sendDataLen);
        }
        if (opRecv == 1)
        {
            receive_message_from_server(dataReceive);
        }
    }
    else
    {
        if (opRecv == 1)
        {
            receive_message_from_server(dataReceive);
        }
        if (opSend == 1)
        {
            send_message_to_server(dataSend, sendDataLen);
        }
    }

    kfree(dataReceive);
    kfree(sendData);

    return 0;
}

static void __exit exit_socket(void)
{
    sock_release(sock);
    printk("End\n");
}

module_init(init_socket);
module_exit(exit_socket);

MODULE_LICENSE("GPL");                 /* giay phep su dung cua module */
MODULE_AUTHOR(DRIVER_AUTHOR);          /* tac gia cua module */
MODULE_DESCRIPTION(DRIVER_DESC);       /* mo ta chuc nang cua module */
MODULE_SUPPORTED_DEVICE("testdevice"); /* kieu device ma module ho tro */
