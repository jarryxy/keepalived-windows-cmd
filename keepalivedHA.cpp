#include <iostream>
#include <pthread.h>
#include <winsock2.h>  
#include <ws2tcpip.h>  
#include <csignal>
#include <Windows.h>
#include "vrrp.h"
#include "conf.h"
#include "vrrp_interface.h"
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "ws2_32.lib")
using namespace std;


pthread_t tids[5];
vrrp_conf conf;
bool running = true;

void* send_adv(void* args) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    vrrp_conf *conf = (vrrp_conf*)args;
    int fd = open_send_vrrp(*conf);
    conf->fd= fd;
    while (true)
    {
        if (!conf->preempt) {
            vrrp_send_adv(*conf, conf->instance.priority);
        }
        Sleep(conf->instance.advert_int*1000);
    }
    return 0;
}

void* recv_adv(void* args) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    vrrp_conf* conf = (vrrp_conf*)args;
    int fd = open_recv_vrrp(*conf);
    conf->fd = fd;
    vrrp_recv_adv(*conf);
    return 0;
}

//volatile sig_atomic_t gSignalStatus;
//
//void signal_handler(int signal)
//{
//    gSignalStatus = signal;
//}

BOOL console_handler(DWORD ctrl_type)
{
    cout << "exit(" << ctrl_type << ") keepalivedHA..." << endl;

    switch (ctrl_type)
    {
    case CTRL_CLOSE_EVENT:
    case CTRL_C_EVENT:
        cout << "exit(" << ctrl_type << ") keepalivedHA..." << endl;
        pthread_cancel(tids[0]);
        pthread_cancel(tids[1]);
        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
            remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
        }
        // 发送权重0的adv
        vrrp_send_adv(conf, 0);
        running = false;
        return TRUE;
    default:
        return FALSE;
    }

}
int main()
{
    // 注册控制事件处理函数
    SetConsoleCtrlHandler(console_handler, TRUE);
    // 设置信号处理函数
    //signal(SIGINT, signal_handler);
    conf = read_vrrp_conf("ha.conf");
    print_vrrp_conf(conf);
    conf.preempt = false;
    if (conf.instance.state == MASTER || conf.instance.state == "MASTER") {
        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
            //remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
            //Sleep(100);
            add_vip(conf.instance.virtual_ipaddress[i], "255.255.255.0", conf.instance.interface_name);
         }
        pthread_create(&tids[0], NULL, send_adv, (void*)&conf);
        Sleep(500);
        pthread_create(&tids[1], NULL, recv_adv, (void*)&conf);
    }
    else {
        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
            remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
        }
        pthread_create(&tids[0], NULL, send_adv, (void*)&conf);
        Sleep(500);
        pthread_create(&tids[1], NULL, recv_adv, (void*)&conf);
    }

    while (running) {
        Sleep(1000);
    }
    // 等待信号
    //while (true)
    //{
    //    if (gSignalStatus == SIGINT) {
    //        cout << "exit keepalivedHA..." << endl;
    //        pthread_cancel(tids[0]);
    //        pthread_cancel(tids[1]);
    //        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
    //            remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
    //        }
    //        // 发送权重0的adv
    //        vrrp_send_adv(conf, 0);
    //        Sleep(500);
    //        break;
    //    }
    //    Sleep(300);
    //}
    return 0;
}
