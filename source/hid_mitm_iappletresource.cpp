#include "hid_mitm_iappletresource.hpp"

#include "hid_custom.h"
#include "ini.h"
#include "udp_input.h"

#include <arpa/inet.h>
#include <map>
#include <math.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

//static SharedMemory fake_shmem = {0};
//static HidSharedMemory *fake_shmem_mem;
//static HidSharedMemory *real_shmem_mem;

// TODO: Currently assumes one sharedmem per pid.
static std::unordered_map<u64, std::pair<HidSharedMemory *, HidSharedMemory *>> sharedmems;

static HidSharedMemory tmp_shmem_mem;

static Thread shmem_patch_thread, network_thread;

// VALUE is the key that we want to get when we click KEY

static Mutex configMutex, pkgMutex;
static struct input_msg cur_fakegamepad_state = {0};

void add_shmem(u64 pid, SharedMemory *real_shmem, SharedMemory *fake_shmem)
{
    mutexLock(&shmem_mutex);
    HidSharedMemory *real_mapped = (HidSharedMemory *)shmemGetAddr(real_shmem);
    HidSharedMemory *fake_mapped = (HidSharedMemory *)shmemGetAddr(fake_shmem);
    sharedmems[pid] = std::pair<HidSharedMemory *, HidSharedMemory *>(real_mapped, fake_mapped);
    mutexUnlock(&shmem_mutex);
}

void del_shmem(u64 pid)
{
    mutexLock(&shmem_mutex);
    if (sharedmems.find(pid) != sharedmems.end())
    {
        sharedmems.erase(pid);
    }
    mutexUnlock(&shmem_mutex);
}

std::string key_names[] = {"KEY_A", "KEY_B", "KEY_X", "KEY_Y", "KEY_LSTICK", "KEY_RSTICK", "KEY_L", "KEY_R", "KEY_ZL", "KEY_ZR", "KEY_PLUS", "KEY_MINUS", "KEY_DLEFT", "KEY_DUP", "KEY_DRIGHT", "KEY_DDOWN"};

s64 get_key_ind(std::string str)
{
    for (u64 i = 0; i < sizeof(key_names) / sizeof(char *); i++)
    {
        if (str == key_names[i])
        {
            return BIT(i);
        }
    }
    return -1;
}

/* sample config:
; Gamepad-rebind config. Currently always rebinds for all players, no individual config.
; VALUE is the button that gets registered when KEY is held down
[player1]
deadzone=0
KEY_A=KEY_A
KEY_B=KEY_B
KEY_X=KEY_X
KEY_Y=KEY_Y
KEY_LSTICK=KEY_LSTICK
KEY_RSTICK=KEY_RSTICK
KEY_L=KEY_L
KEY_R=KEY_R
KEY_ZL=KEY_ZL
KEY_ZR=KEY_ZR
KEY_PLUS=KEY_PLUS
KEY_MINUS=KEY_MINUS
KEY_DLEFT=KEY_DLEFT
KEY_DUP=KEY_DUP
KEY_DRIGHT=KEY_DRIGHT
KEY_DDOWN=KEY_DDOWN
[network]
enabled=1
*/

bool networking_enabled = true;

s16 stick_deadzone = 0;

static int handler(void *dummy, const char *section, const char *name,
                   const char *value)
{
    /*if (!strcmp(name, "enabled"))
    {
        networking_enabled = atoi(value);
    }*/
    if (!strcmp(name, "deadzone"))
    {
        stick_deadzone = atoi(value);
    }

    s64 key = get_key_ind(name);
    s64 val = get_key_ind(value);

    if (key < 0 || val < 0)
    {
        return -1;
    }
    return 0;
}

void loadConfig()
{
    mutexLock(&configMutex);
    if (ini_parse("/config/hid_mitm/config.ini", handler, NULL) < 0)
    {
        //fatalSimple(MAKERESULT(321, 1)); // 2321-0001 bad config
    }
    mutexUnlock(&configMutex);
}

// Clears config until next load
void clearConfig()
{
    mutexLock(&configMutex);
    mutexUnlock(&configMutex);
}


struct hid_unk_section
{
    u64 type;
    u64 unk;
};

int FakeController::initialize()
{
    if (isInitialized == true) return 0;
    Result myResult;

    FILE *debugTXT = fopen("sdmc:/networkconlog.txt", "w+a");
    if (debugTXT != nullptr) {
        fprintf(debugTXT, "Controller initializing");
        fclose(debugTXT);
    }
    //printToFile("Controller initializing...");
        
    
    // Set the controller type to Pro-Controller, and set the npadInterfaceType.
    controllerDevice.deviceType = HidDeviceType_FullKey3; // FullKey3 for Pro Controller, JoyLeft4 for left joy con
    controllerDevice.npadInterfaceType = NpadInterfaceType_Bluetooth;
    // Set the controller colors. The grip colors are for Pro-Controller on [9.0.0+].
    controllerDevice.singleColorBody = RGBA8_MAXALPHA(255,153,204);
    controllerDevice.singleColorButtons = RGBA8_MAXALPHA(0,0,0);
    controllerDevice.colorLeftGrip = RGBA8_MAXALPHA(255,0,127);
    controllerDevice.colorRightGrip = RGBA8_MAXALPHA(255,0,127);

    // Setup example controller state.
    controllerState.batteryCharge = 4; // Set battery charge to full.
    controllerState.joysticks[JOYSTICK_LEFT].dx = 0x0;
    controllerState.joysticks[JOYSTICK_LEFT].dy = -0x0;
    controllerState.joysticks[JOYSTICK_RIGHT].dx = 0x0;
    controllerState.joysticks[JOYSTICK_RIGHT].dy = -0x0;
    
    myResult = hiddbgAttachHdlsVirtualDevice(&controllerHandle, &controllerDevice);
    if (R_FAILED(myResult)) {
        FILE *debugTXT = fopen("sdmc:/networkconlog.txt", "w+a");
        if (debugTXT != nullptr) {
            fprintf(debugTXT, "FAILED ATTACHING CONTROLLER... NANI?!");
            fclose(debugTXT);
        }
        return -1;
    }

    //printToFile("Controller initialized!");
    isInitialized = true;
    return 0;
}

int FakeController::deinitialize()
{
    if (isInitialized == false) return 0;
    Result myResult;
    //printToFile("Controller initializing...");
    
    myResult = hiddbgDetachHdlsVirtualDevice(controllerHandle);
    if (R_FAILED(myResult)) {
        //printToFile("Failed connecting controller... fuck");
        return -1;
    }

    //printToFile("Controller initialized!");
    isInitialized = false;
    return 0;
}


FakeController controllerList[8];
s32 controllersConnected = 0;
s32 realControllersConnectedAmount = 0;
int apply_fake_gamepad(struct input_msg msg) // This function is called every frame if networking is enabled
{
    // Experienced programmers! Enjoy this noob paradise with absolutely not optimized and terrible code I made! really, please tell me tips because I absolutely need them :p
    if(msg.magic != INPUT_MSG_MAGIC)
        return -1;

    for (s32 num = controllersConnected; num < msg.controllerCount; num++)
    {
        int oShit = controllerList[num].initialize();
        controllersConnected++;
    }
    /*for (s32 num = controllersConnected; num > msg.controllerCount; num--)
    {
        controllerList[num].deinitialize();
        controllersConnected--;
    }*/

    // Controller 1 inputs, it'll always be on
    controllerList[0].controllerState.buttons = msg.keys;
    controllerList[0].controllerState.joysticks[0].dx = msg.joy_l_x;
    controllerList[0].controllerState.joysticks[0].dy = msg.joy_l_y;
    controllerList[0].controllerState.joysticks[1].dx = msg.joy_r_x;
    controllerList[0].controllerState.joysticks[1].dy = msg.joy_r_y;
    hiddbgSetHdlsState(controllerList[0].controllerHandle, &controllerList[0].controllerState);

    // Controller 2 inputs
    if (msg.controllerCount >= 2)
    {
        controllerList[1].controllerState.buttons = msg.twoKeys;
        controllerList[1].controllerState.joysticks[0].dx = msg.twoJoy_l_x;
        controllerList[1].controllerState.joysticks[0].dy = msg.twoJoy_l_y;
        controllerList[1].controllerState.joysticks[1].dx = msg.twoJoy_r_x;
        controllerList[1].controllerState.joysticks[1].dy = msg.twoJoy_r_y;
        hiddbgSetHdlsState(controllerList[1].controllerHandle, &controllerList[1].controllerState);
    }

    // Controller 3 inputs
    if (msg.controllerCount >= 3)
    {
        controllerList[2].controllerState.buttons = msg.threeKeys;
        controllerList[2].controllerState.joysticks[0].dx = msg.threeJoy_l_x;
        controllerList[2].controllerState.joysticks[0].dy = msg.threeJoy_l_y;
        controllerList[2].controllerState.joysticks[1].dx = msg.threeJoy_r_x;
        controllerList[2].controllerState.joysticks[1].dy = msg.threeJoy_r_y;
        hiddbgSetHdlsState(controllerList[2].controllerHandle, &controllerList[2].controllerState);
    }

    // Controller 4 inputs
    if (msg.controllerCount >= 4)
    {
        controllerList[3].controllerState.buttons = msg.fourKeys;
        controllerList[3].controllerState.joysticks[0].dx = msg.fourJoy_l_x;
        controllerList[3].controllerState.joysticks[0].dy = msg.fourJoy_l_y;
        controllerList[3].controllerState.joysticks[1].dx = msg.fourJoy_r_x;
        controllerList[3].controllerState.joysticks[1].dy = msg.fourJoy_r_y;
        hiddbgSetHdlsState(controllerList[3].controllerHandle, &controllerList[3].controllerState);
    }

    return 0;
}

void shmem_copy(HidSharedMemory *source, HidSharedMemory *dest)
{

    // Apparently unused
    //memcpy(dest->controllerSerials, source->controllerSerials, sizeof(dest->controllerSerials));

    memcpy(&dest->touchscreen, &source->touchscreen, sizeof(source->touchscreen) - sizeof(source->touchscreen.padding));
    memcpy(&dest->mouse, &source->mouse, sizeof(source->mouse) - sizeof(source->mouse.padding));
    memcpy(&dest->keyboard, &source->keyboard, sizeof(source->keyboard) - sizeof(source->keyboard.padding));
    for (int i = 0; i < 10; i++)
    {
        // Only copy used gamepads
        if (dest->controllers[i].misc.deviceType != 0 || source->controllers[i].misc.deviceType != 0)
            memcpy(&dest->controllers[i], &source->controllers[i], sizeof(source->controllers[i]) - sizeof(source->controllers[i].unk_2)); // unk_2 is apparently unused and is huge
    }
}

void net_thread(void* _)
{
    struct input_msg tmp_pkg;
    while (true)
    {
        if (networking_enabled)
        {
            int poll_res = poll_udp_input(&tmp_pkg);

            mutexLock(&pkgMutex);
            if (poll_res == 0)
            {
                cur_fakegamepad_state = tmp_pkg;
            }
            else
            {
                cur_fakegamepad_state.magic = 0;
                svcSleepThread(1e+7l);
            }
            mutexUnlock(&pkgMutex);

            svcSleepThread(-1);
        } else {
            svcSleepThread(1e+9l);
        }
    }
}

#define WANT_TIME 96000
// Official software ticks 200 times/second

void copy_thread(void* _)
{
    ams::Result rc;

    /*rc = viInitialize(ViServiceType_System);
    if (R_FAILED(rc))
        fatalSimple(rc);

    ViDisplay disp;
    rc = viOpenDefaultDisplay(&disp);
    if (R_FAILED(rc))
        fatalSimple(rc);

    Event event;
    rc = viGetDisplayVsyncEvent(&disp, &event);
    if (R_FAILED(rc))
        fatalSimple(rc);*/

    loadConfig();

    while (true)
    {
        u64 curTime = svcGetSystemTick();

        mutexLock(&shmem_mutex);
        mutexLock(&pkgMutex);

        for (auto it = sharedmems.begin(); it != sharedmems.end(); it++)
        {
            shmem_copy(it->second.first, &tmp_shmem_mem);
            svcSleepThread(-1);

            if (networking_enabled)
            {
                apply_fake_gamepad(cur_fakegamepad_state);
            }

            shmem_copy(&tmp_shmem_mem, it->second.second);
        }
        mutexUnlock(&pkgMutex);

        if (sharedmems.empty())
        {
            mutexUnlock(&shmem_mutex);
            svcSleepThread(1e+9L / 60);
            continue;
        }
        mutexUnlock(&shmem_mutex);

        s64 time_rest = WANT_TIME - (svcGetSystemTick() - curTime);
        if (time_rest > 0)
        {
            svcSleepThread((time_rest * 1e+9L) / 19200000);
        }
    }

    /*viCloseDisplay(&disp);
    viExit();*/
}

void copyThreadInitialize()
{
    mutexInit(&configMutex);
    mutexInit(&pkgMutex);
    loadConfig();
    threadCreate(&shmem_patch_thread, copy_thread, NULL, NULL, 0x1000, 0x21, 3);
    threadStart(&shmem_patch_thread);

    threadCreate(&network_thread, net_thread, NULL, NULL, 0x1000, 0x30, 3);
    threadStart(&network_thread);
}

IAppletResourceMitmService::~IAppletResourceMitmService()
{
    del_shmem(this->pid.value);
    customHidExit(&this->iappletresource_handle, &this->real_sharedmem, &this->fake_sharedmem);
}

ams::Result IAppletResourceMitmService::GetSharedMemoryHandle(ams::sf::OutCopyHandle shmem_hand)
{
    shmem_hand.SetValue(this->fake_sharedmem.handle);

    return ams::ResultSuccess();
}

