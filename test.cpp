#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main1()
{
    ULONG buflen = 0;
    DWORD ret = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;

    // 获取缓冲区大小
    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL,
        pAddresses, &buflen);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        // 分配缓冲区
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(buflen);
        if (pAddresses == NULL) {
            printf("malloc failed\n");
            return -1;
        }
        // 获取适配器信息
        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL,
            pAddresses, &buflen);
        if (ret != NO_ERROR) {
            printf("GetAdaptersAddresses failed with error %d\n", ret);
            free(pAddresses);
            return -1;
        }
        // 遍历适配器列表并打印信息
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            printf("Adapter name: %S\n", pCurrAddresses->FriendlyName);
            printf("Adapter description: %S\n", pCurrAddresses->Description);

            // 输出IPv4地址
            PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
            while (pUnicast != NULL) {
                if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                    struct sockaddr_in* pAddr = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
                    char buffer[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &pAddr->sin_addr, buffer, INET_ADDRSTRLEN);
                    printf("Adapter IPv4 address: %s\n", buffer);
                    break;
                }
                pUnicast = pUnicast->Next;
            }

            // 输出子网掩码
     /*       if (pCurrAddresses->FirstPrefix != NULL) {
                DWORD mask = pCurrAddresses->FirstPrefix->PrefixLength;
                DWORD maskBytes[4] = { 0 };
                int i;
                for (i = 0; i < mask / 8; i++) {
                    maskBytes[i] = 0xFF;
                }
                if (mask % 8 != 0) {
                    maskBytes[i] = (0xFF << (8 - (mask % 8)));
                }
                printf("Adapter subnet mask: %d.%d.%d.%d\n", maskBytes[0], maskBytes[1], maskBytes[2], maskBytes[3]);
            }*/

            pCurrAddresses = pCurrAddresses->Next;
        }
        free(pAddresses);
    }
    else {
        printf("GetAdaptersAddresses failed with error %d\n", ret);
        return -1;
    }

    return 0;
}
