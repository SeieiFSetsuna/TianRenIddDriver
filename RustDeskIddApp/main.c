#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <tchar.h>

#include "./IddController.h"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "swdevice.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Newdev.lib")

#define MAX_MONITOR_MODES 10


int prompt_input()
{
    system("chcp 65001"); // 更改控制台为UTF-8编码
    printf("按键                  执行：\n");
    printf("       1. 'q'               1. 退出\n");
    printf("       2. 'c'               2. 创建具有生命周期\"SWDeviceLifetimeHandle\"的VD驱动\n");
    printf("       3. 'C'               3. 创建具有生命周期\"SWDeviceLifetimeParentPresent\"的VD驱动\n");
    printf("       4. 'd'               4. 销毁VD驱动\n");
    printf("       5. 'i'               5. 安装或更新驱动程序\n");
    printf("       6. 'u'               6. 卸载驱动程序\n");
    printf("       7. 'a'               7. 插入显示器\n");
    printf("       8. 'b'               8. 拔出显示器\n");
    printf("       9. 'm'               9. 更新显示器模式\n");

    return _getch();
}

int __cdecl main(int argc, char* argv[])
{
    HSWDEVICE hSwDevice = NULL;
    BOOL bExit = FALSE;
    SW_DEVICE_LIFETIME lifetime = SWDeviceLifetimeHandle;

    DWORD width = 1920;
    DWORD height = 1080;
    DWORD sync = 60;

    UINT index = 0;

    TCHAR exePath[1024] = { 0, };
    (void)GetModuleFileName(NULL, exePath, sizeof(exePath)/sizeof(exePath[0]) - 1);
    *_tcsrchr(exePath, _T('\\')) = _T('\0');
    PTCHAR infPath = _T("TianRenIddDriver\\TianRenIddDriver.inf");
    TCHAR infFullPath[1024] = { 0, };
    _sntprintf_s(infFullPath, sizeof(infFullPath) / sizeof(infFullPath[0]), _TRUNCATE, _T("%s\\%s"), exePath, infPath);

    do
    {
        int key = prompt_input();
        BOOL rebootRequired = FALSE;
        switch (key)
        {
        case 'i':
            printf("开始安装或更新驱动程序\n");
            if (FALSE == InstallUpdate(infFullPath, &rebootRequired))
            {
                printf(GetLastMsg());
            }
            else
            {
                printf("安装或更新驱动程序已完成，%s需要重新启动\n", (rebootRequired == TRUE ? "" : "不"));
            }
            break;
        case 'u':
            printf("卸载驱动程序开始\n");
            if (FALSE == Uninstall(infFullPath, &rebootRequired))
            //if (FALSE == InstallUpdate(_T("D:\\projects\\windows\\IndirectDisplay\\x64\\Debug\\TianRenIddDriver\\TianRenIddDriver.inf"), &rebootRequired))
            {
                printf(GetLastMsg());
            }
            else
            {
                printf("卸载驱动程序已完成，%s需要重新启动\n", (rebootRequired == TRUE ? "" : "不"));
            }
            break;
        case 'c':
        case 'C':
            lifetime = key == 'c' ? SWDeviceLifetimeHandle : SWDeviceLifetimeParentPresent;
            printf("创建驱动开始\n");
            if (hSwDevice != NULL)
            {
                printf("之前创建的驱动\n");
                break;
            }
            if (FALSE == DeviceCreateWithLifetime(&lifetime, &hSwDevice))
            {
                printf(GetLastMsg());
                DeviceClose(hSwDevice);
                hSwDevice = NULL;
            }
            else
            {
                printf("创建驱动完成\n");
            }
            break;
        case 'd':
            printf("关闭驱动开始\n");
            DeviceClose(hSwDevice);
            hSwDevice = NULL;
            printf("关闭驱动完成\n");
            break;
        case 'a':
            printf("插入显示器开始，当前索引 %u\n", index);
            if (FALSE == MonitorPlugIn(index, 0, 25))
            {
                printf(GetLastMsg());
            }
            else
            {
                printf("插入显示器完成\n");

                MonitorMode modes[2] = { { 1920, 1080,  60 }, { 1024,  768,  60 }, };
                if (FALSE == MonitorModesUpdate(index, sizeof(modes)/sizeof(modes[0]), modes))
                {
                    printf(GetLastMsg());
                }

                index += 1;
            }
            break;
        case 'b':
            if (index == 0) {
                printf("无虚拟显示器\n");
                break;
            }

            printf("拔出显示器开始，当前索引 %u\n", index - 1);
            if (FALSE == MonitorPlugOut(index - 1))
            {
                printf(GetLastMsg());
            }
            else
            {
                index -= 1;
                printf("拔出显示器完成\n");
            }
            break;
        case 'm':
            printf("更新显示器模式，当前最大索引 %u\n", index - 1);
            printf("请从0到%d中选择显示器\n", index - 1);
            UINT i = 0;
            scanf_s("%d%*c", &i);
            if (i >= index) {
                fprintf(stderr, "请选择指数小于等于 %d，您的输入为 %d\n", index - 1, i);
            }
            else {
                printf("\n您已选择 %d显示器 来更新模式。\n", i);
                printf("请至少添加一种显示器模式，格式：宽度、高度、刷新频率。\n最大模式数为 %d。\n按'e'结束输入。\n", MAX_MONITOR_MODES);

                int input_ok = 0;
                int k = 0;
                MonitorMode modes[MAX_MONITOR_MODES] = { {0, 0, 0}, };
                while (1) {
                    char input[100];
                    if (fgets(input, sizeof(input), stdin) != NULL) {
                        if (input[0] == 'e') {
                            input_ok = 1;
                            break;
                        }
                        else {
                            sscanf_s(input, "%d,%d,%d", &modes[k].width, &modes[k].height, &modes[k].sync);
                        }
                    }
                    else {
                        fprintf(stderr, "尝试读取输入时出错。请重试\n");
                        break;
                    }
                    ++k;
                    if (k == sizeof(modes) / sizeof(modes[0])) {
                        input_ok = 1;
                        break;
                    }
                }

                if (input_ok == 1) {
                    if (k == 0) {
                        fprintf(stderr, "未添加显示器模式，跳过。\n");
                    }
                    else {
                        if (FALSE == MonitorModesUpdate(i, k, modes))
                        {
                            printf(GetLastMsg());
                        }
                    }
                }
            }
            break;
        case 'q':
            bExit = TRUE;
            break;
        default:
            break;
        }

        printf("\n\n");
    }while (!bExit);

    if (hSwDevice)
    {
        SwDeviceClose(hSwDevice);
    }

    return 0;
}
